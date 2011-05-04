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

	this->start();
}




//La méthode run pour l'exécution du thread
void ClientManager::run()
{
	socket=new Socket();
	QObject::connect(socket, SIGNAL(disconnected()), this, SLOT(clientDisconnected()));
	QObject::connect(socket, SIGNAL(receiveMessage(QByteArray*)), this, SLOT(receiveMessageAction(QByteArray*)));
	QObject::connect(socket, SIGNAL(encrypted()), this, SLOT(connexionEncrypted()));
	QObject::connect(socket, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(erreursSsl(QList<QSslError>)));
	this->clientDescription=socket->peerAddress().toString();

	if(socket->setDescriptor(clientSocket))
	{
		Widget::addRowToTable("Connexion initialisée avec descripteur de socket du client "+clientDescription+".",model,MSG_2);
	}
	else
	{
		Widget::addRowToTable("Impossible d'initialiser une connexion avec le descripteur de socket du client "+clientDescription,model,MSG_2);
		socket->disconnectClient();
		return ;
	}
	QObject::connect(this,SIGNAL(sendDetectionRequested(QByteArray*)),this,SLOT(sendDetection(QByteArray*)));
	exec();
}




void ClientManager::sendDetectionRequest(QByteArray *request)
{
	emit this->sendDetectionRequested(request);
}



void ClientManager::sendDetection(QByteArray *request)
{
	this->socket->sendMessage(request);
}



//Recu quand la connexion avec le client est crypté par SSL
void ClientManager::connexionEncrypted()
{
	Widget::addRowToTable("Le client "+clientDescription+" est maintenant connecté en mode crypté",model,MSG_2);
}




//Slot appelé quand le client se déconnecte
void ClientManager::clientDisconnected()
{
	Widget::addRowToTable("Le client "+clientDescription+" s'est déconnecté.",model,MSG_2);
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
void ClientManager::receiveMessageAction(QByteArray *message)
{
	//On récupère le message
	Message *m=Messages::parseMessage(message);

	//Si le message est inconnu
	if(!m)
	{
		qDebug("Warning -1 C.M.");
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









//Lorsqu'une requete est récue du client. C'est la fonction principale
// attention elle est longue :D
void ClientManager::receivedRequest(Request *r)
{
	//Si la requete est inconnue, on envoi un message d'érreur
	if(r==NULL)
	{
		Response response;
		response.setType(REJECT_FILE_INFO_FOR_PARAMETERS);
		this->socket->sendMessage(response.toXml());
		return ;
	}
	//Si on se trouve à l'état connecté
	if(state==CONNECTED)
	{
		if(r->getType()==IDENTIFICATION)
		{
			Widget::addRowToTable("Le client "+clientDescription+" a envoyé un message d'identification.",model,MSG_2);
			Response response;
			QString pseudo=r->getParameters()->value("pseudo");
			QString password=r->getParameters()->value("password");
			bool rep=databaseManager->authentificateUser(pseudo,password);
			if(rep)
			{
				response.setType(ACCEPT_IDENTIFICATION);
				Widget::addRowToTable("Identification acceptée",model,MSG_3);
				clientDescription=pseudo+":"+socket->peerAddress().toString();
				this->state=CLIENT_DETECTIONS;
				this->user=databaseManager->getUser(pseudo);
				this->socket->sendMessage(response.toXml());
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
	if(state==CLIENT_DETECTIONS)
	{
		if(r->getType()==REVISION_FILE_INFO)
		{
			Widget::addRowToTable("Le client "+clientDescription+" a envoyé un message de révision de dépot.",model,MSG_2);
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
				this->socket->sendMessage(response.toXml());
				return;
			}
			Depot *depot=this->fileManager->getDepot(u->depotname);
			QList<Request*> list=depot->getUpgradingRequest(revision);
			upgrading.append(list);
			Response response;response.setType(ACCEPT_FILE_INFO);
			this->socket->sendMessage(response.toXml());
			return;
		}
		else if(r->getType()==END_OLD_DETECTIONS)
		{
			this->state=SERVER_DETECTIONS;
			for(int i=0;i<upgrading.size();i++)
				this->socket->sendMessage(upgrading.at(i)->toXml());
			Request request;
			request.setType(END_OLD_DETECTIONS);
			this->socket->sendMessage(request.toXml());
			this->state=SYNCHRONIZED;
			return;
		}
	}
	if(state==SERVER_DETECTIONS)
	{
		Response response;response.setType(REJECT_FILE_INFO_FOR_RIGHT);
		this->socket->sendMessage(response.toXml());
		return;
	}
	if(r->getType()==CREATE_FILE_INFO || r->getType()==UPDATE_FILE_INFO || r->getType()==REMOVE_FILE_INFO)
	{
		QString realPath=r->getParameters()->value("realPath","");
		QString description=(r->getType()==CREATE_FILE_INFO)?"créé":((r->getType()==UPDATE_FILE_INFO)?"modifié":"supprimé");
		Widget::addRowToTable("Le client "+clientDescription+" a détecté que le média "+realPath+" a été "+description,model,MSG_2);
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
			Widget::addRowToTable("Le dépot SVN du média n'est pas chargé.",model,MSG_3);
			Response response;response.setType(REJECT_FILE_INFO_FOR_SVNERROR);
			this->socket->sendMessage(response.toXml());
			return;
		}
		int svnRevision=depot->getRevision();

		if(clientRevision>svnRevision)
		{
			Widget::addRowToTable("La révision du dépot SVN du client est supérieure à celle du serveur.",model,MSG_3);
			Response response;response.setType(REJECT_FILE_INFO_FOR_SVNERROR);
			response.getParameters()->insert("revision",QByteArray::number(svnRevision));
			this->socket->sendMessage(response.toXml());
			return;
		}

		Response response;
		if(r->getType()==CREATE_FILE_INFO)
		{
			if(depot->isMediaExists(realPath))
			{
				if(svnRevision==clientRevision)	qDebug("Warning 20 C.M.");
				Widget::addRowToTable("Un conflit SVN a été détecté: Le media à créér existe déjà.",model,MSG_3);
				response.setType(REJECT_FILE_INFO_FOR_CONFLICT);
				response.getParameters()->insert("revision",QByteArray::number(svnRevision));
			}
			else if(!(depot->isMediaExists(Depot::extractParentPath(realPath)+"/")))
			{
				if(svnRevision==clientRevision)	qDebug("Warning 21 C.M.");
				Widget::addRowToTable("Un conflit SVN a été détecté: Le parent du média à créer n'existe pas.",model,MSG_3);
				response.setType(REJECT_FILE_INFO_FOR_CONFLICT);
				response.getParameters()->insert("revision",QByteArray::number(svnRevision));
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
					Widget::addRowToTable("La requete a été acceptée et committée sur le SVN.",model,MSG_3);
					response.setType(ACCEPT_FILE_INFO);
					response.getParameters()->insert("revision",QByteArray::number(depot->getRevision()));
				}
				else
				{
					qDebug("Warning 23 C.M.");
					Widget::addRowToTable("Une erreur s'est produite lors du commit. La requete a été rejettée.",model,MSG_3);
					response.setType(REJECT_FILE_INFO_FOR_SVNERROR);
					response.getParameters()->insert("revision",QByteArray::number(svnRevision));
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
					Widget::addRowToTable("La requete a été acceptée et committée sur le SVN.",model,MSG_3);
					response.setType(ACCEPT_FILE_INFO);
					response.getParameters()->insert("revision",QByteArray::number(depot->getRevision()));
				}
				else
				{
					qDebug("Warning 25 C.M.");
					Widget::addRowToTable("Une erreur s'est produite lors du commit. La requete a été rejettée.",model,MSG_3);
					response.setType(REJECT_FILE_INFO_FOR_SVNERROR);
					response.getParameters()->insert("revision",QByteArray::number(svnRevision));
				}
			}
			else
			{
				if(svnRevision==clientRevision)	qDebug("Warning 26 C.M.");
				Widget::addRowToTable("Un conflit SVN a été détecté: Le media à modifier n'existe pas.",model,MSG_3);
				response.setType(REJECT_FILE_INFO_FOR_CONFLICT);
				response.getParameters()->insert("revision",QByteArray::number(svnRevision));
			}
		}
		else if(r->getType()==REMOVE_FILE_INFO)
		{
			if(!depot->isMediaExists(realPath))
			{
				if(svnRevision==clientRevision)	qDebug("Warning 27 C.M.");
				Widget::addRowToTable("Un conflit SVN a été détecté: Le media à supprimer n'existe pas.",model,MSG_3);
				response.setType(REJECT_FILE_INFO_FOR_CONFLICT);
				response.getParameters()->insert("revision",QByteArray::number(svnRevision));
			}
			else
			{
				bool result=depot->deleteMedia(realPath,user->login,user->password);
				if(result)
				{
					Widget::addRowToTable("La requete a été acceptée et committée sur le SVN.",model,MSG_3);
					response.setType(ACCEPT_FILE_INFO);
					response.getParameters()->insert("revision",QByteArray::number(depot->getRevision()));
				}
				else
				{
					qDebug("Warning 23 C.M.");
					Widget::addRowToTable("Une erreur s'est produite lors du commit. La requete a été rejettée.",model,MSG_3);
					response.setType(REJECT_FILE_INFO_FOR_SVNERROR);
					response.getParameters()->insert("revision",QByteArray::number(svnRevision));
				}
			}
		}
		if(response.getType()==ACCEPT_FILE_INFO)
		{
			r->getParameters()->remove("revision");
			r->getParameters()->insert("revision",QByteArray::number(depot->getRevision()));
			for(int i=0;i<clients->size();i++)
			{
				if(clients->at(i)!=this && clients->at(i)->getUser()!=NULL)
				{
					QString login=clients->at(i)->getUserLogin();
					QString depotname=depot->getDepotName();
					if(depotname.endsWith("/")) depotname=depotname.left(depotname.length()-1);
					if(databaseManager->isUserLinkDepot(login,depotname))
						clients->at(i)->sendDetectionRequest(new QByteArray(*(r->toXml())));
				}
			}
		}
		this->socket->sendMessage(response.toXml());
	}
}





QStandardItemModel *ClientManager::getModelClient()
{
	return model;
}


SqlUser *ClientManager::getUser()
{
	return user;
}


QString ClientManager::getUserLogin()
{
	if(!user) return "";
	else return user->login;
}


ClientManager::~ClientManager()
{
	this->terminate();
	delete this->socket;
}


