#include "clientmanager.h"
#include "widget.h"



//Pour allouer l'objet ClientManager
ClientManager *ClientManager::createClientManager(int clientSocket,QVector<ClientManager*> *clients,DatabaseManager *databaseManager,FileManager *fileManager,QStandardItemModel *model)
{
	if(!databaseManager || !fileManager) return NULL;
	ClientManager *cl=new ClientManager(clients,databaseManager,fileManager,model);
	if(cl->socket->setDescriptor(clientSocket))
	{
		Widget::addRowToTable("Le client "+cl->socket->peerAddress().toString()+" vient de se connecter en mode non crypté",model);
		return cl;
	}
	delete cl;
	return NULL;
}




//Le constructeur
ClientManager::ClientManager(QVector<ClientManager*> *clients,DatabaseManager *databaseManager,FileManager *fileManager,QStandardItemModel *model): QThread()
{
	this->moveToThread(this);

	socket=new Socket();
	this->databaseManager=databaseManager;
	this->fileManager=fileManager;
	this->model=model;
	this->clients=clients;
	this->state=CONNECTED;
	this->user=NULL;

	start();

	QObject::connect(socket, SIGNAL(receiveMessage(QByteArray*)), this, SLOT(receiveMessageAction(QByteArray*)));
	QObject::connect(socket, SIGNAL(encrypted()), this, SLOT(connexionEncrypted()),Qt::DirectConnection);
        QObject::connect(socket, SIGNAL(disconnected()), this, SLOT(clientDisconnected()));
	QObject::connect(socket, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(erreursSsl(QList<QSslError>)),Qt::DirectConnection);
}




//Recu quand la connexion avec le client est crypté par SSL
void ClientManager::connexionEncrypted()
{
	Widget::addRowToTable("Le client "+socket->peerAddress().toString()+" est maintenant connecté en mode crypté",model);
}




//Slot appelé quand un client se déconnecte
void ClientManager::clientDisconnected()
{
	QSslSocket *client = qobject_cast<QSslSocket*>(sender());
	Widget::addRowToTable("Le client "+ client->peerAddress().toString()+" s'est déconnecté.",model,false);
	emit disconnectedClient(this);
}



//Slot pour les erreurs ssl
void ClientManager::erreursSsl(const QList<QSslError> &errors)
{
	foreach(const QSslError &error, errors)
	{
		Widget::addRowToTable("Erreur SSL ignorée: "+ error.errorString(), model,false);
	}
}




//Cette fonction est automatiquement appelé lorsqu'un client envoi un message.
//Pour l'instant on ne fait que renvoyer simplement le message à tous les autres clients
void ClientManager::receiveMessageAction(QByteArray *message)
{
	//On récupère le message
	Message *m=Messages::parseMessage(message);

	//Si le message est inconnu
	if(!m) return;

	//Si c'est une requete
	if(m->isRequest())
	{
		Request *r=(Request*)m;
		receivedRequest(r);
		return;
	}
}





void ClientManager::receivedRequest(Request *r)
{
	if(r->getType()==IDENTIFICATION)
	{
		Widget::addRowToTable("Le client "+this->socket->peerAddress().toString()+" a envoyé un message d'identification.",model);
		Response response;
		QString pseudo=r->getParameters()->value("pseudo");
		QString password=r->getParameters()->value("password");
		bool rep=databaseManager->authentificateUser(pseudo,password);
		if(rep)
		{
			response.setType(ACCEPT_IDENTIFICATION);
			Widget::addRowToTable("Identification acceptée",model);
			this->state=IDENTIFIED;
			user=databaseManager->getUser(pseudo);
		}
		else
		{
			if(databaseManager->isUserExists(pseudo)) response.setType(REJECT_IDENTIFICATION_FOR_PASSWORD);
			else response.setType(REJECT_IDENTIFICATION_FOR_PASSWORD);
			Widget::addRowToTable("Identification refusée",model);
		}
		this->socket->sendMessage(response.toXml());
	}
	else if(r->getType()==CREATE_FILE_INFO || r->getType()==UPDATE_FILE_INFO || r->getType()==REMOVE_FILE_INFO)
	{
		Widget::addRowToTable("Le client "+this->socket->peerAddress().toString()+" a détecté un changement.",model);
		if(state<IDENTIFIED)
		{
			Widget::addRowToTable("Le changement a été ignoré, une identification est requise.",model);
			Response response;response.setType(REJECT_FILE_INFO_FOR_IDENTIFICATION);
			this->socket->sendMessage(response.toXml());
			return;
		}
		QString realPath=r->getParameters()->value("realPath","");
		if(realPath.isEmpty())
		{
			Widget::addRowToTable("La requête contient des paramètres eronnés",model);
			Response response;response.setType(REJECT_FILE_INFO_FOR_PARAMETERS);
			this->socket->sendMessage(response.toXml());
			return;
		}
		SqlUtilisation *u=NULL;
		for(int i=0;i<user->utilisations->length();i++)
		{
			if(realPath.startsWith(user->utilisations->at(i)->depotname))
				u=user->utilisations->at(i);
		}
		if(u==NULL)
		{
			Widget::addRowToTable("Le changement est ignoré pour manque de droits",model);
			Response response;response.setType(REJECT_FILE_INFO_FOR_RIGHT);
			this->socket->sendMessage(response.toXml());
			return;
		}
		Depot *depot=this->fileManager->getDepot(u->depotname);
		if(!depot)
		{
			this->fileManager->addDepot(u->depotname);
			depot=this->fileManager->getDepot(u->depotname);
		}
		bool result=false;
		if(r->getType()==CREATE_FILE_INFO)
		{
			if(r->getParameters()->value("isDirectory","")=="true")
				result=depot->createDir(realPath,user->login,user->password);
			else
				result=depot->createDir(realPath,user->login,user->password);
		}
		for(int i=0;i<clients->size();i++)
		{
			if(clients->at(i)!=this) clients->at(i)->socket->sendMessage(r->toXml());
		}
		//Widget::addRowToTable("Le changement a été transmit à "+QString::number(clients->size()-1)+" clients.",model,false);
		Response response;
		response.setType(ACCEPT_FILE_INFO);
		if(this->socket->sendMessage(response.toXml()))
			Widget::addRowToTable("L'accusé de reception a été envoyé",model,false);
	}
}



void ClientManager::run()
{
	exec();
}



ClientManager::~ClientManager()
{
	socket->disconnectClient();
	delete this->socket;
}


