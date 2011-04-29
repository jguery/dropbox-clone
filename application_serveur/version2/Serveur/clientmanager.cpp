#include "clientmanager.h"
#include "widget.h"



//Pour allouer l'objet ClientManager
ClientManager *ClientManager::createClientManager(int clientSocket,QVector<ClientManager*> *clients,DatabaseManager *databaseManager,FileManager *fileManager,QStandardItemModel *model)
{
	if(!databaseManager || !fileManager)
		return NULL;
	ClientManager *cl=new ClientManager(clientSocket,clients,databaseManager,fileManager,model);
	return cl;
}




//Le constructeur
ClientManager::ClientManager(int clientSocket,QVector<ClientManager*> *clients,DatabaseManager *databaseManager,FileManager *fileManager,QStandardItemModel *model): QThread()
{
	this->moveToThread(this);
	this->clientSocket=clientSocket;

	this->databaseManager=databaseManager;
	this->fileManager=fileManager;
	this->model=model;
	this->clients=clients;
	this->state=CONNECTED;
	this->user=NULL;
	this->timerOldDetection.setSingleShot(true);
	this->timerOldDetection.setInterval(5000);
	QObject::connect(&timerOldDetection,SIGNAL(timeout()),this,SLOT(sendNewDetection()));

	this->start();
}



void ClientManager::run()
{
	socket=new Socket();
	QObject::connect(socket, SIGNAL(disconnected()), this, SLOT(clientDisconnected()),Qt::DirectConnection);
	QObject::connect(socket, SIGNAL(receiveMessage(QByteArray*)), this, SLOT(receiveMessageAction(QByteArray*)));
	QObject::connect(socket, SIGNAL(encrypted()), this, SLOT(connexionEncrypted()),Qt::DirectConnection);
	QObject::connect(socket, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(erreursSsl(QList<QSslError>)),Qt::DirectConnection);
	if(socket->setDescriptor(clientSocket))
	{
		Widget::addRowToTable("Initialisation d'une connexion avec le client "+socket->peerAddress().toString()+".",model,MSG_2);
	}
	else
	{
		Widget::addRowToTable("Impossible d'initialiser une connexion avec le client "+socket->peerAddress().toString(),model,MSG_2);
		socket->disconnectClient();
		return ;
	}
	exec();
}






//Recu quand la connexion avec le client est crypté par SSL
void ClientManager::connexionEncrypted()
{
	Widget::addRowToTable("Le client "+socket->peerAddress().toString()+" est maintenant connecté en mode crypté",model,MSG_2);
}




//Slot appelé quand un client se déconnecte
void ClientManager::clientDisconnected()
{
	QSslSocket *client = qobject_cast<QSslSocket*>(sender());
	Widget::addRowToTable("Le client "+ client->peerAddress().toString()+" s'est déconnecté.",model,MSG_2);
	emit disconnectedClient(this);
}



//Slot pour les erreurs ssl
void ClientManager::erreursSsl(const QList<QSslError> &errors)
{
	foreach(const QSslError &error, errors)
	{
		Widget::addRowToTable("Erreur SSL ignorée: "+ error.errorString(), model,MSG_2);
	}
}




//Cette fonction est automatiquement appelé lorsqu'un client envoi un message.
//Pour l'instant on ne fait que renvoyer simplement le message à tous les autres clients
void ClientManager::receiveMessageAction(QByteArray *message)
{
	//On récupère le message
	Message *m=Messages::parseMessage(message);

	//Si le message est inconnu
	if(!m)
	{
		qDebug("Warning -1 C.M.");
		socket->disconnectClient();
		return;
	}

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
	if(r==NULL)
	{
		Response response;
		response.setType(REJECT_FILE_INFO_FOR_SVNERROR);
		this->socket->sendMessage(response.toXml());
		return ;
	}
	if(state==CONNECTED)
	{
		if(r->getType()==IDENTIFICATION)
		{
			Widget::addRowToTable("Le client "+this->socket->peerAddress().toString()+" a envoyé un message d'identification.",model,MSG_3);
			Response response;
			QString pseudo=r->getParameters()->value("pseudo");
			QString password=r->getParameters()->value("password");
			bool rep=databaseManager->authentificateUser(pseudo,password);
			if(rep)
			{
				response.setType(ACCEPT_IDENTIFICATION);
				Widget::addRowToTable("Identification acceptée",model,MSG_3);
				this->state=IDENTIFIED;
				this->user=databaseManager->getUser(pseudo);
				this->socket->sendMessage(response.toXml());
				this->timerOldDetection.start();
			}
			else
			{
				if(databaseManager->isUserExists(pseudo)) response.setType(REJECT_IDENTIFICATION_FOR_PASSWORD);
				else response.setType(REJECT_IDENTIFICATION_FOR_PSEUDO);
				Widget::addRowToTable("Identification refusée",model,MSG_3);
				this->socket->sendMessage(response.toXml());
			}
		}
		else
		{
			Widget::addRowToTable("La requête a été ignorée, une identification est requise.",model,MSG_3);
			Response response;response.setType(REJECT_FILE_INFO_FOR_RIGHT);
			this->socket->sendMessage(response.toXml());
		}
		return;
	}
	if(state==IDENTIFIED)
	{
		this->timerOldDetection.start();

		if(r->getType()==REVISION_FILE_INFO)
		{
			Widget::addRowToTable("Le client "+this->socket->peerAddress().toString()+" a envoyé un message de révision de dépot.",model,MSG_3);
			QString realPath=r->getParameters()->value("realPath","");
			QString revisionString=r->getParameters()->value("revision","");
			int revision=revisionString.toInt();

			SqlUtilisation *u=NULL;
			for(int i=0;i<user->utilisations->length();i++)
			{
				if(realPath.startsWith(user->utilisations->at(i)->depotname))
					u=user->utilisations->at(i);
			}
			if(u==NULL)
			{
				Widget::addRowToTable("Dépot introuvable",model,MSG_3);
				Response response;response.setType(REJECT_FILE_INFO_FOR_RIGHT);
				//this->socket->sendMessage(response.toXml());
				return;
			}
			Depot *depot=this->fileManager->getDepot(u->depotname);
			QList<Request*> list=depot->getUpgradingRequest(revision);
			upgrading.append(list);
		}
	}
	if(state==NEWDETECTIONS)
	{
		qDebug("Warning 59 C.M.");
	}
	if(r->getType()==CREATE_FILE_INFO || r->getType()==UPDATE_FILE_INFO || r->getType()==REMOVE_FILE_INFO)
	{
		Widget::addRowToTable("Le client "+this->socket->peerAddress().toString()+" a détecté un changement.",model,MSG_3);
		QString realPath=r->getParameters()->value("realPath","");
		QString clientRevisionString=r->getParameters()->value("revision","");
		int clientRevision=0;clientRevision=clientRevisionString.toInt();

		if(realPath.isEmpty())
		{
			Widget::addRowToTable("La requête contient des paramètres eronnés",model,MSG_3);
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
			Widget::addRowToTable("Le changement est ignoré pour manque de droits",model,MSG_3);
			Response response;response.setType(REJECT_FILE_INFO_FOR_RIGHT);
			this->socket->sendMessage(response.toXml());
			return;
		}
		Depot *depot=this->fileManager->getDepot(u->depotname);
		if(!depot)
		{
			qDebug("Warning 1 C.M.");
			Response response;response.setType(REJECT_FILE_INFO_FOR_SVNERROR);
			this->socket->sendMessage(response.toXml());
			return;
		}
		int svnRevision=depot->getRevision();

		if(clientRevision>svnRevision)
		{
			qDebug("Warning 2 C.M.");
			Response response;response.setType(REJECT_FILE_INFO_FOR_SVNERROR);
			if(state==SYNCHRONIZED) response.getParameters()->insert("revision",QByteArray::number(svnRevision));
			this->socket->sendMessage(response.toXml());
			return;
		}

		Response response;
		if(r->getType()==CREATE_FILE_INFO)
		{
			if(depot->isMediaExists(realPath))
			{
				if(svnRevision==clientRevision)	qDebug("Warning 20 C.M.");
				response.setType(REJECT_FILE_INFO_FOR_CONFLICT);
				if(state==SYNCHRONIZED) response.getParameters()->insert("revision",QByteArray::number(svnRevision));
			}
			else if(!(depot->isMediaExists(Depot::extractParentPath(realPath)+"/")))
			{
				if(svnRevision==clientRevision)	qDebug("Warning 21 C.M.");
				response.setType(REJECT_FILE_INFO_FOR_CONFLICT);
				if(state==SYNCHRONIZED) response.getParameters()->insert("revision",QByteArray::number(svnRevision));
			}
			else
			{
				bool result=false;
				if(r->getParameters()->value("isDirectory","")=="true")
					result=depot->createDir(realPath,user->login,user->password);
				else
					result=depot->createFile(realPath,user->login,user->password);
				if(result)
				{
					response.setType(ACCEPT_FILE_INFO);
					if(state==SYNCHRONIZED) response.getParameters()->insert("revision",QByteArray::number(svnRevision));
				}
				else
				{
					qDebug("Warning 23 C.M.");
					response.setType(REJECT_FILE_INFO_FOR_SVNERROR);
					if(state==SYNCHRONIZED) response.getParameters()->insert("revision",QByteArray::number(svnRevision));
				}
			}
		}
		else if(r->getType()==UPDATE_FILE_INFO)
		{
			QByteArray content=r->getParameters()->value("content","");
			if(depot->isMediaExists(realPath))
			{
				bool result=depot->updateFileContent(realPath,content,user->login,user->password);
				if(result)
				{
					response.setType(ACCEPT_FILE_INFO);
					if(state==SYNCHRONIZED) response.getParameters()->insert("revision",QByteArray::number(svnRevision));
				}
				else
				{
					qDebug("Warning 25 C.M.");
					response.setType(REJECT_FILE_INFO_FOR_SVNERROR);
					if(state==SYNCHRONIZED) response.getParameters()->insert("revision",QByteArray::number(svnRevision));
				}
			}
			else
			{
				if(svnRevision==clientRevision)	qDebug("Warning 26 C.M.");
				response.setType(REJECT_FILE_INFO_FOR_CONFLICT);
				if(state==SYNCHRONIZED) response.getParameters()->insert("revision",QByteArray::number(svnRevision));
			}
		}
		else if(r->getType()==REMOVE_FILE_INFO)
		{
			if(!depot->isMediaExists(realPath))
			{
				if(svnRevision==clientRevision)	qDebug("Warning 27 C.M.");
				response.setType(REJECT_FILE_INFO_FOR_CONFLICT);
				if(state==SYNCHRONIZED) response.getParameters()->insert("revision",QByteArray::number(svnRevision));
			}
			else
			{
				bool result=depot->deleteMedia(realPath,user->login,user->password);
				if(result)
				{
					response.setType(ACCEPT_FILE_INFO);
					if(state==SYNCHRONIZED) response.getParameters()->insert("revision",QByteArray::number(svnRevision));
				}
				else
				{
					qDebug("Warning 23 C.M.");
					response.setType(REJECT_FILE_INFO_FOR_SVNERROR);
					if(state==SYNCHRONIZED) response.getParameters()->insert("revision",QByteArray::number(svnRevision));
				}
			}
		}
		/*for(int i=0;i<clients->size();i++)
		{
			if(clients->at(i)!=this) clients->at(i)->socket->sendMessage(r->toXml());
		}*/
		this->socket->sendMessage(response.toXml());
	}
}



void ClientManager::sendNewDetection()
{
	this->state=NEWDETECTIONS;
	for(int i=0;i<upgrading.size();i++)
		this->socket->sendMessage(upgrading.at(i)->toXml());
	this->state=SYNCHRONIZED;
}



ClientManager::~ClientManager()
{
	this->terminate();
	delete this->socket;
}


