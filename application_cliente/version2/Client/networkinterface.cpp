#include "networkinterface.h"
#include "widget.h"





//La méthode statique d'allocation
NetworkInterface *NetworkInterface::createNetworkInterface(ConfigurationNetwork *configurationNetwork,ConfigurationIdentification *configurationIdentification, QStandardItemModel *model)
{
	//On teste la validité de la configuration
	if(configurationNetwork==NULL || configurationIdentification==NULL || model==NULL)
	{
		if(model!=NULL) Widget::addRowToTable("Echec lors de l'allocation du module d'interface réseau.",model,MSG_1);
		return NULL;
	}

	//On retourne l'objet créé
	return new NetworkInterface(configurationNetwork,configurationIdentification,model);
}




//Le constructeur
NetworkInterface::NetworkInterface(ConfigurationNetwork *configurationNetwork, ConfigurationIdentification *configurationIdentification, QStandardItemModel *model): QThread()
{
	this->moveToThread(this);

	this->configurationNetwork=configurationNetwork;
	this->configurationIdentification=configurationIdentification;

	//On fait les initialisations
	this->isConnected=false;
	this->waitReceiveRequestList=NULL;
	this->receiveRequestList=new QList<Request*>();
	this->response=NULL;

	this->model = model;

	this->start();
	Widget::addRowToTable("Le module d'interface réseau a bien été alloué.",model,MSG_1);
}






//La méthode run pour lancer le thread
void NetworkInterface::run()
{
	//On crèe la socket
	this->socket=new Socket(this);

	//On établit les connexion des évenement de socket à la classe.
	QObject::connect(socket,SIGNAL(stateChanged(QAbstractSocket::SocketState)),this,SLOT(stateChangedAction(QAbstractSocket::SocketState)));
	QObject::connect(socket,SIGNAL(connected()),this,SLOT(connectedToServer()));
	QObject::connect(socket,SIGNAL(disconnected()),this,SLOT(disconnectedFromServer()));

	QObject::connect(socket,SIGNAL(encrypted()),this,SLOT(connexionEncrypted()));
	QObject::connect(socket,SIGNAL(sslErrors(QList<QSslError>)),this,SLOT(erreursSsl(QList<QSslError>)));
	QObject::connect(socket,SIGNAL(receiveMessage(QByteArray*)),this,SLOT(receiveMessageAction(QByteArray*)));

	QObject::connect(this,SIGNAL(connectToServerRequested()),this,SLOT(connectToServer()));
	QObject::connect(this,SIGNAL(disconnectFromServerRequested()),this,SLOT(disconnectFromServer()));

	exec();
}




//Une requete de connexion
void NetworkInterface::requestConnectToServer()
{
	emit this->connectToServerRequested();
}



//Une requete de déconnexion
void NetworkInterface::requestDisconnectFromServer()
{
	emit this->disconnectFromServerRequested();
}



//Pour se connecter au serveur
void NetworkInterface::connectToServer()
{
	if(isConnected)
	{
		qDebug("Warning 1 N.I.");
		return;
	}
	Widget::addRowToTable("Tentative de connexion au serveur",model,MSG_1);
	//On tente une connexion
	bool a=socket->connectToServer(configurationNetwork->getAddress(),configurationNetwork->getPort());
	if(!a) {Widget::addRowToTable("Echec: Connexion échouée",model,MSG_3);return;}

	//On s'identifie
	if(!this->sendIdentification())
		emit this->disconnectFromServerRequested();
}



//Pour se déconnecter du serveur
//Cette fonction est bloquante pendant quelques secondes
//elle doit être appelée par un thread externe (si gui revoir socket.disconnect)
void NetworkInterface::disconnectFromServer()
{
	if(!isConnected)
	{
		qDebug("Warning 2 N.I.");
		return;
	}
	socket->disconnectFromServer();
}




//Slot appelé lorsque l'état de la socket a changé
//Le nouvel état est dans state. Selon sa valeur, on rédige une description à afficher
void NetworkInterface::stateChangedAction(QAbstractSocket::SocketState state)
{
	QString description;
	if(state==QAbstractSocket::UnconnectedState) description="L'application déconnectée du serveur";
	else if(state==QAbstractSocket::HostLookupState) description="L'application recherche le serveur à l'adresse: "+configurationNetwork->getFullAddress();
	else if(state==QAbstractSocket::ConnectingState) description="L'application tente de se connecter au serveur";
	else if(state==QAbstractSocket::ConnectedState) description="L'application est connectée au serveur en mode non crypté";
	else if(state==QAbstractSocket::ClosingState) description="L'application coupe sa connexion au serveur";
	else description="La connexion réseau est à un état inconnu";
	Widget::addRowToTable("socket::stateChanged: "+description,model,MSG_2);
}




//Slot appelé lorsque la socket est connectée
void NetworkInterface::connectedToServer()
{
	Widget::addRowToTable("Connexion établie",model,MSG_2);
}


//Recu quand la connexion a été correctement cryptée
void NetworkInterface::connexionEncrypted()
{
	Widget::addRowToTable("Connexion avec le serveur correctement établie et cryptée",model,MSG_2);
	emit connected();
}


//Erreurs SSL recues pendant la phase de handshake
void NetworkInterface::erreursSsl(const QList<QSslError> &errors)
{
	foreach(const QSslError &error, errors)
	{
		Widget::addRowToTable("Erreur SSL ignorée: "+ error.errorString(), model,MSG_3);
	}
}


//Slot appelé lorsque la socket est déconnectée
void NetworkInterface::disconnectedFromServer()
{
	this->isConnected=false;
	response=new Response();
	response->setType(NOT_CONNECT);
	waitMessages.wakeAll();
	Widget::addRowToTable("Connexion perdue",model,MSG_3);
	emit disconnected();
}




//Savoir si on est connecté au serveur
bool NetworkInterface::checkIsConnected()
{
	return isConnected;
}





//Ce slot est apellé lorsqu'un message est recu par la socket
void NetworkInterface::receiveMessageAction(QByteArray *message)
{
	//On récupère le message
	Message *m=Messages::parseMessage(message);
	//Si le message est inconnu on emet un signal d'erreur
	if(!m)
	{
		qDebug("Warning 3 N.I.");
		emit receiveErrorMessage("NON_XML_MESSAGE");
		return;
	}

	//On n'est pas identifié, le message reçu est donc un message relatif à l'établissement de la connexion
	if(!isConnected)
	{
		//Le message doit normalement être une réponse!
		if(m->isRequest())
		{
			qDebug("Warning 4 N.I.");
			return;
		}
		//Comme on n'est pas identifié, cette réponse n'est pas envoyé à hddInterface,
		//pas besoin donc de la mettre dans l'attribut response de this
		Response *response=(Response*)m;
		ResponseEnum r=response->getType();	//Type de la réponse

		if(r==ACCEPT_IDENTIFICATION) Widget::addRowToTable("Identification acceptée",model,MSG_2);
		else if(r==REJECT_IDENTIFICATION_FOR_PSEUDO) Widget::addRowToTable("Identification refusée pour pseudo, vous allez être déconnecté du serveur",model,MSG_2);
		else if(r==REJECT_IDENTIFICATION_FOR_PASSWORD) Widget::addRowToTable("Identification refusée pour password, vous allez être déconnecté du serveur",model,MSG_2);
		else if(r==REJECT_IDENTIFICATION_FOR_BLOCK) Widget::addRowToTable("Identification refusée pour block, vous allez être déconnecté du serveur",model,MSG_2);
		else  qDebug("Warning 5 N.I.");

		//Si echec d'identification, on se déconnecte
		if(r!=ACCEPT_IDENTIFICATION)
			emit this->disconnectFromServerRequested();
		else
		{
			isConnected=true;
			emit identified();
		}
		return;
	}

	//Si c'est une requete
	if(m->isRequest())
	{
		Request *r=(Request*)m;
		putReceiveRequestList(r);
		return;
	}
	//sinon une réponse
	else
	{
		Response *r=(Response*)m;
		response=r;
		waitMessages.wakeAll();
		return;
	}
}




//Pour envoyer un message d'identification
//cette fonction n'est pas bloquante. elle renvoi un booleen pour dire si le
//message a été envoyé.
bool NetworkInterface::sendIdentification()
{
	//On récupère le pseudo et le mot de passe de la configuration d'identification
	QString pseudo=configurationIdentification->getPseudo();
	QString password=configurationIdentification->getPassword();

	Request request;
	request.setType(IDENTIFICATION);
	request.getParameters()->insert("pseudo",pseudo.toAscii());
	request.getParameters()->insert("password",password.toAscii());

	//On crèe le message d'identification
	QByteArray *message=request.toXml();
	bool r=socket->sendMessage(message);
	return r;
}



//Pour envoyer un message de fin d'anciennes détections
bool NetworkInterface::sendEndOldDetections()
{
	Request request;
	request.setType(END_OLD_DETECTIONS);

	//On crèe le message
	QByteArray *message=request.toXml();
	bool r=socket->sendMessage(message);
	return r;
}





//Pour envoyer le numéro de dépot
Response *NetworkInterface::sendDepotRevision(QString realPath,int revision)
{
	//On vérifie que la socket est bien connectée et qu'on est identifié
	if(!isConnected)
	{
		qDebug("Warning 226 N.I.");
		response=new Response();
		response->setType(NOT_CONNECT);
		return response;
	}

	//On vérifie que le realPath n'est pas vide
	if(realPath.isEmpty() || revision<0)
	{
		qDebug("Warning 117 N.I.");
		response=new Response();
		response->setType(NOT_PARAMETERS);
		return response;
	}

	Request request;
	request.setType(REVISION_FILE_INFO);
	request.getParameters()->insert("realPath",realPath.toAscii());
	request.getParameters()->insert("revision",QByteArray::number(revision));

	//On crèe le message et on l'envoi
	QByteArray *message=request.toXml();
	bool r=socket->sendMessage(message);
	if(!r)
	{
		qDebug("Warning 123 N.I.");
		response=new Response();
		response->setType(NOT_SEND);
		return response;
	}

	QMutex mutex;
	if(!waitMessages.wait(&mutex,20000))
	{
		qDebug("Warning 127 N.I.");
		response=new Response();
		response->setType(NOT_TIMEOUT);
	}

	qDebug() << response;
	return response;
}




//Pour envoyer un message de fichier modifié
Response *NetworkInterface::sendMediaUpdated(QString realPath,QByteArray content, int revision)
{
	//On vérifie que la socket est bien connectée et qu'on est identifié
	if(!isConnected)
	{
		qDebug("Warning 6 N.I.");
		response=new Response();
		response->setType(NOT_CONNECT);
		return response;
	}

	//On vérifie que le realPath n'est pas vide
	if(realPath.isEmpty())
	{
		qDebug("Warning 7 N.I.");
		response=new Response();
		response->setType(NOT_PARAMETERS);
		return response;
	}

	Request request;
	request.setType(UPDATE_FILE_INFO);
	request.getParameters()->insert("realPath",realPath.toAscii());
	request.getParameters()->insert("revision",QString::number(revision).toAscii());
	request.getParameters()->insert("content",content);

	//On rédige le message et on l'envoi
	QByteArray *message=request.toXml();
	bool r=socket->sendMessage(message);
	if(!r)
	{
		qDebug("Warning 13 N.I.");
		response=new Response();
		response->setType(NOT_SEND);
		return response;
	}

	QMutex mutex;
	if(!waitMessages.wait(&mutex,20000))
	{
		qDebug("Warning 17 N.I.");
		response=new Response();
		response->setType(NOT_TIMEOUT);
	}

	return response;
}



//Pour envoyer un message de média créé
Response *NetworkInterface::sendMediaCreated(QString realPath, bool isDirectory, int revision)
{
	//On vérifie que la socket est bien connectée et qu'on est identifié
	if(!isConnected)
	{
		qDebug("Warning 8 N.I.");
		response=new Response();
		response->setType(NOT_CONNECT);
		return response;
	}

	//On vérifie que le realPath n'est pas vide
	if(realPath.isEmpty())
	{
		qDebug("Warning 9 N.I.");
		response=new Response();
		response->setType(NOT_PARAMETERS);
		return response;
	}
	Request request;
	request.setType(CREATE_FILE_INFO);
	request.getParameters()->insert("realPath",realPath.toAscii());
	request.getParameters()->insert("revision",QString::number(revision).toAscii());
	request.getParameters()->insert("isDirectory",isDirectory?"true":"false");

	//On rédige le message et on l'envoi
	QByteArray *message=request.toXml();
	bool r=socket->sendMessage(message);
	if(!r)
	{
		qDebug("Warning 14 N.I.");
		response=new Response();
		response->setType(NOT_SEND);
		return response;
	}

	QMutex mutex;
	if(!waitMessages.wait(&mutex,20000))
	{
		qDebug("Warning 18 N.I.");
		response=new Response();
		response->setType(NOT_TIMEOUT);
	}

	return response;
}






//Envoyer un message de média supprimé
Response *NetworkInterface::sendMediaRemoved(QString realPath, int revision)
{
	//On vérifie que la socket est bien connectée et qu'on est identifié
	if(!isConnected)
	{
		qDebug("Warning 10 N.I.");
		response=new Response();
		response->setType(NOT_CONNECT);
		return response;
	}

	//On vérifie que le realPath n'est pas vide
	if(realPath.isEmpty())
	{
		qDebug("Warning 11 N.I.");
		response=new Response();
		response->setType(NOT_PARAMETERS);
		return response;
	}

	Request request;
	request.setType(REMOVE_FILE_INFO);
	request.getParameters()->insert("realPath",realPath.toAscii());
	request.getParameters()->insert("revision",QString::number(revision).toAscii());

	//On rédige le message et on l'envoi
	QByteArray *message=request.toXml();
	bool r=socket->sendMessage(message);
	if(!r)
	{
		qDebug("Warning 15 N.I.");
		response=new Response();
		response->setType(NOT_SEND);
		return response;
	}

	QMutex mutex;
	if(!waitMessages.wait(&mutex,20000))
	{
		qDebug("Warning 19 N.I.");
		response=new Response();
		response->setType(NOT_TIMEOUT);
	}

	return response;
}







//Pour récupérer la prochaine requete à traiter
Request *NetworkInterface::getReceiveRequestList()
{
	//On vérouille l'accès
	receiveRequestListMutex.lock();
	Request *r=NULL;

	//On récupère le premier element s'il existe
	if(receiveRequestList->size()>0)
	{
		r=receiveRequestList->first();
		//On le supprime après l'avoir récupérer
		receiveRequestList->removeFirst();
	}

	//On libère l'accès
	receiveRequestListMutex.unlock();
	return r;
}




//pour ajouter une requete à traiter prochainement
void NetworkInterface::putReceiveRequestList(Request *r)
{
	//On vérouille l'accès
	receiveRequestListMutex.lock();

	//On ajoute l'element
	receiveRequestList->append(r);
	//On reveille le thread de HddInterface
	if(waitReceiveRequestList!=NULL)
		waitReceiveRequestList->wakeAll();

	//On libère l'objet
	receiveRequestListMutex.unlock();
}





//Indiquer l'objet de reveil du controleur
void NetworkInterface::setWaitReceiveRequestList(QWaitCondition *waitReceiveRequestList)
{
	this->waitReceiveRequestList=waitReceiveRequestList;
}




NetworkInterface::~NetworkInterface()
{
	this->terminate();
	delete socket;
}
