#include "networkinterface.h"
#include "widget.h"


//La méthode statique d'allocation
NetworkInterface *NetworkInterface::createNetworkInterface(ConfigurationNetwork *configurationNetwork,ConfigurationIdentification *configurationIdentification, QStandardItemModel *model)
{
	//On teste la validité de la configuration
	if(configurationNetwork==NULL || configurationIdentification==NULL)
		return NULL;

	if(model==NULL)
		return NULL;

	//On retourne l'objet créé
	return new NetworkInterface(configurationNetwork,configurationIdentification,model);
}




//Le constructeur
NetworkInterface::NetworkInterface(ConfigurationNetwork *configurationNetwork, ConfigurationIdentification *configurationIdentification, QStandardItemModel *model): QThread()
{
	this->moveToThread(this);

	//On crèe la socket
	this->socket=new Socket();
	this->configurationNetwork=configurationNetwork;
	this->configurationIdentification=configurationIdentification;

	//On fait les initialisations
	this->isConnected=false;
	this->isIdentified=false;
	this->waitReceiveRequestList=NULL;
	this->receiveRequestList=new QList<Request*>();

	//On établit les connexion des évenement de socket à la classe.
	QObject::connect(socket,SIGNAL(stateChanged(QAbstractSocket::SocketState)),this,SLOT(stateChangedAction(QAbstractSocket::SocketState)));
	QObject::connect(socket,SIGNAL(connected()),this,SLOT(connectedToServer()));
	QObject::connect(socket,SIGNAL(disconnected()),this,SLOT(disconnectedFromServer()));

	QObject::connect(socket,SIGNAL(encrypted()),this,SLOT(connexionEncrypted()));
	QObject::connect(socket,SIGNAL(sslErrors(QList<QSslError>)),this,SLOT(erreursSsl(QList<QSslError>)));
	QObject::connect(socket,SIGNAL(receiveMessage(QByteArray*)),this,SLOT(receiveMessageAction(QByteArray*)));

	this->model = model;
}



//Slot appelé lorsque l'état de la socket a changé
void NetworkInterface::stateChangedAction(QAbstractSocket::SocketState state)
{
	//Le nouvel état est dans state. Selon sa valeur, on rédige une description à afficher
	QString description;
	if(state==QAbstractSocket::UnconnectedState) description="L'application déconnectée du serveur";
	else if(state==QAbstractSocket::HostLookupState) description="L'application recherche le serveur à l'adresse: "+configurationNetwork->getFullAddress();
	else if(state==QAbstractSocket::ConnectingState) description="L'application tente de se connecter au serveur";
	else if(state==QAbstractSocket::ConnectedState) description="L'application est connectée au serveur en mode non crypté";
	else if(state==QAbstractSocket::ClosingState) description="L'application coupe sa connexion au serveur";
	else description="La connexion réseau est à un état inconnu";

	Widget::addRowToTable("socket::stateChanged: "+description,model,MSG_NETWORK);
}



//Slot appelé lorsque la socket est connectée
void NetworkInterface::connectedToServer()
{
	Widget::addRowToTable("Connexion établie",model,MSG_NETWORK);
	emit connected();
}



//Slot appelé lorsque la socket est déconnectée
void NetworkInterface::disconnectedFromServer()
{
	this->isConnected=false;
	this->isIdentified=false;
	Widget::addRowToTable("Connexion perdue",model,MSG_NETWORK);
	emit disconnected();
}



//Recu quand la connexion a été correctement cryptée
void NetworkInterface::connexionEncrypted()
{
	this->isConnected=true;
	Widget::addRowToTable("Connexion avec le serveur correctement établie et cryptée",model,MSG_NETWORK);
}




//Erreurs SSL recues pendant la phase de handshake
void NetworkInterface::erreursSsl(const QList<QSslError> &errors)
{
	foreach(const QSslError &error, errors)
	{
		Widget::addRowToTable("Erreur SSL ignorée: "+ error.errorString(), model,MSG_NETWORK);
	}
}



//La méthode exec pour lancer le thread
void NetworkInterface::run()
{
	exec();
}




//Pour se connecter au serveur
//Cette fonction est bloquante pendant quelques secondes
//elle doit être appelée par un thread externe (si gui revoir socket.connect)
bool NetworkInterface::connectToServer()
{
	return socket->connectToServer(configurationNetwork->getAddress(),configurationNetwork->getPort());
}



//Pour se déconnecter du servuer
//Cette fonction est bloquante pendant quelques secondes
//elle doit être appelée par un thread externe (si gui revoir socket.disconnect)
bool NetworkInterface::disconnectFromServer()
{
	return socket->disconnectFromServer();
}




//Ce slot est apellé lorsqu'un message est recu par la socket
void NetworkInterface::receiveMessageAction(QByteArray *message)
{
	//On récupère le message
	Message *m=Messages::parseMessage(message);

	//Si le message est inconnu on emet un signal d'erreur
	if(!m){emit receiveErrorMessage("NON_XML_MESSAGE");return;}

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
		response=r->getType();
		waitMessages.wakeAll();
		return;
	}

	//Si on ne trouve pas le type de message, on emet un signal d'érreur.
	emit receiveErrorMessage("UNKNOWN_MESSAGE");
	return;
}




//Pour envoyer un message d'identification
//cette fonction est bloquante tant qu'elle ne recoit pas de réponse
//il l'appeller dans un thread externe
bool NetworkInterface::sendIdentification()
{
	//On vérifie que la socket est bien connectée
	if(!isConnected) return false;
	this->isIdentified=false;

	//On récupère le pseudo et le mot de passe de la configuration d'identification
	QString pseudo=configurationIdentification->getPseudo();
	QString password=configurationIdentification->getPassword();

	Request request;
	request.setType(IDENTIFICATION);
	request.getParameters().insert("pseudo",pseudo.toAscii());
	request.getParameters().insert("password",password.toAscii());

	//On crèe le message d'identification
	QByteArray *message=request.toXml();
	bool r=socket->sendMessage(message);
	if(!r) return false;

	//On attends la réponse
	QMutex mutex;
	waitMessages.wait(&mutex);

	if(response==VALID_IDENTIFICATION)
	{
		this->isIdentified=true;
		return true;
	}
	return false;
}



//Pour envoyer un message de fichier modifié
bool NetworkInterface::sendMediaUpdated(QString realPath,QByteArray content)
{
	//On vérifie que la socket est bien connectée et qu'on est identifié
	if(!isConnected) return false;
	if(!isIdentified) return false;

	//On vérifie que le realPath n'est pas vide
	if(realPath.isEmpty()) return false;

	Request request;
	request.setType(UPDATE_FILE_INFO);
	request.getParameters().insert("realPath",realPath.toAscii());
	request.getParameters().insert("content",content);

	//On rédige le message et on l'envoi
	QByteArray *message=request.toXml();
	bool r=socket->sendMessage(message);
	if(!r) return false;

	QMutex mutex;
	waitMessages.wait(&mutex);

	if(response==ACK_FILE_INFO)
	{
		return true;
	}
	return false;
}



//Pour envoyer un message de média créé
bool NetworkInterface::sendMediaCreated(QString realPath, bool isDirectory)
{
	//On vérifie que la socket est bien connectée et qu'on est identifié
	if(!isConnected) return false;
	if(!isIdentified) return false;

	//On vérifie que le realPath n'est pas vide
	if(realPath.isEmpty()) return false;

	Request request;
	request.setType(CREATE_FILE_INFO);
	request.getParameters().insert("realPath",realPath.toAscii());
	request.getParameters().insert("isDirectory",isDirectory?"true":"false");

	//On rédige le message et on l'envoi
	QByteArray *message=request.toXml();
	bool r=socket->sendMessage(message);
	if(!r) return false;

	QMutex mutex;
	waitMessages.wait(&mutex);

	if(response==ACK_FILE_INFO)
	{
		return true;
	}
	return false;
}






//Envoyer un message de média supprimé
bool NetworkInterface::sendMediaRemoved(QString realPath)
{
	//On vérifie que la socket est bien connectée et qu'on est identifié
	if(!isConnected) return false;
	if(!isIdentified) return false;

	//On vérifie que le realPath n'est pas vide
	if(realPath.isEmpty()) return false;

	Request request;
	request.setType(REMOVE_FILE_INFO);
	request.getParameters().insert("realPath",realPath.toAscii());

	//On rédige le message et on l'envoi
	QByteArray *message=request.toXml();
	bool r=socket->sendMessage(message);
	if(!r) return false;

	QMutex mutex;
	waitMessages.wait(&mutex);

	if(response==ACK_FILE_INFO)
	{
		return true;
	}
	return false;
}





//Pour récupérer la prochaine requete à traiter
Request *NetworkInterface::getReceiveRequestList()
{
	receiveRequestListMutex.lock();
	Request *r;
	if(receiveRequestList->size()>0)
	{
		r=receiveRequestList->first();
		receiveRequestList->removeFirst();
	}
	else r=NULL;
	receiveRequestListMutex.unlock();
	return r;
}




//pour ajouter une requete à traiter prochainement
void NetworkInterface::putReceiveRequestList(Request *r)
{
	receiveRequestListMutex.lock();
	receiveRequestList->append(r);
	if(waitReceiveRequestList!=NULL) waitReceiveRequestList->wakeAll();
	receiveRequestListMutex.unlock();
}



//Indiquer l'objet de reveil du controleur
void NetworkInterface::setWaitReceiveRequestList(QWaitCondition *waitReceiveRequestList)
{
	this->waitReceiveRequestList=waitReceiveRequestList;
}



