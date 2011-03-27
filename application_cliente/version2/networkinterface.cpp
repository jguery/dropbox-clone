#include "networkinterface.h"


//La méthode statique d'allocation
NetworkInterface *NetworkInterface::createNetworkInterface(ConfigurationData *configurationData)
{
	if(configurationData==NULL) return NULL;
	return new NetworkInterface(configurationData);
}


//Le constructeur
NetworkInterface::NetworkInterface(ConfigurationData *configurationData): QObject()
{
	socket=new Socket();
	QObject::connect(socket,SIGNAL(stateChanged(QAbstractSocket::SocketState)),this,SLOT(stateChangedAction(QAbstractSocket::SocketState)));
	QObject::connect(socket,SIGNAL(connected()),this,SIGNAL(connectedToServer()));
	QObject::connect(socket,SIGNAL(disconnected()),this,SIGNAL(disconnectedFromServer()));
	QObject::connect(socket,SIGNAL(receiveMessage(QByteArray*)),this,SLOT(receiveMessageAction(QByteArray*)));
	this->configurationData=configurationData;
}


//Pour se connecter au serveur
bool NetworkInterface::connect()
{
	return socket->connectToServer(configurationData->getConfigurationNetwork()->getAddress(),configurationData->getConfigurationNetwork()->getPort());
}

//Pour se déconnecter du servuer
bool NetworkInterface::disconnect()
{
	return socket->disconnectFromServer();
}

#include<QtGui>
//Ce slot est apellé lorsqu'un message est recu par la socket
void NetworkInterface::receiveMessageAction(QByteArray *message)
{
	QMessageBox::information(NULL,"",*message);
	//On parse pour récupérer le message
	QHash<QString,QByteArray> *arg=Messages::parseMessage(message);
	if(!arg){emit receiveErrorMessage("4");return;}
	//On récupere le realPath
	QString realPath=arg->value("realPath","");
	if(realPath.isEmpty()){emit receiveErrorMessage("5");return;}

	//En fonction de l'action demandé, on emet le signal correspondant
	if(arg->value("action")=="MODIFIED")
	{
		Media *m=configurationData->getConfigurationFile()->findMediaByRealPath(realPath);
		if(m==NULL){emit receiveErrorMessage("2");return;}
		if(m->isDirectory()){emit receiveErrorMessage("3");return;}
		File *f=(File*)m;
		QByteArray content=arg->value("content","");
		emit receiveModifiedFileMessage(f,content);
		return;
	}
	else if(arg->value("action")=="CREATED")
	{
		QString realPath2=realPath;
		if(realPath2.endsWith("/")) realPath2=realPath2.left(realPath2.length()-1);
		Media *parentMedia=configurationData->getConfigurationFile()->findMediaByRealPath(realPath2.left(realPath2.lastIndexOf("/")));
		if(parentMedia==NULL){emit receiveErrorMessage("7");return;}
		if(!parentMedia->isDirectory()){emit receiveErrorMessage("10");return;}
		Dir *parent=(Dir*)parentMedia;

		QString realName=realPath2.right(realPath2.length()-realPath2.lastIndexOf("/")-1);
		if(realPath.endsWith("/")) realName=realName+"/";

		emit receiveCreatedMediaMessage(parent,realName);
		return;
	}
	else if(arg->value("action")=="REMOVED")
	{
		Media *m=configurationData->getConfigurationFile()->findMediaByRealPath(realPath);
		if(m==NULL){emit receiveErrorMessage("6");return;}
		emit receiveRemovedMediaMessage(m);
		return;
	}
	else if(arg->value("action")=="VALIDATION")
	{
		emit receiveValidationMessage();
		return;
	}
	else if(arg->value("action")=="ANNULATION")
	{
		emit receiveAnnulationMessage();
		return;
	}
	emit receiveErrorMessage("8");
	return;
}




//Pour envoyer un message d'identification
bool NetworkInterface::sendIdentification()
{
	if(!socket->isWritable()) return false;
	QString pseudo=configurationData->getConfigurationIdentification()->getPseudo();
	QString password=configurationData->getConfigurationIdentification()->getPassword();
	QByteArray *message=Messages::createIdentificationMessage(pseudo,password);
	return socket->sendMessage(message);
}



//Pour envoyer un message de fichier modifié
bool NetworkInterface::sendFileModified(QString realPath,QByteArray content)
{
	if(!socket->isWritable()) return false;
	if(realPath.isEmpty()) return false;
	QByteArray *message=Messages::createFileContentMessage(realPath,content);
	return socket->sendMessage(message);
}


//Pour envoyer un message de média créé
bool NetworkInterface::sendMediaCreated(QString realPath)
{
	if(!socket->isWritable()) return false;
	if(realPath.isEmpty()) return false;
	QByteArray *message=Messages::createMediaCreatedMessage(realPath);
	return socket->sendMessage(message);
}

//supprimé
bool NetworkInterface::sendMediaRemoved(QString realPath)
{
	if(!socket->isWritable()) return false;
	if(realPath.isEmpty()) return false;
	QByteArray *message=Messages::createMediaRemovedMessage(realPath);
	return socket->sendMessage(message);
}



void NetworkInterface::stateChangedAction(QAbstractSocket::SocketState state)
{
	QString description;
	if(state==QAbstractSocket::UnconnectedState) description="L'application est en mode déconnecté du serveur";
	else if(state==QAbstractSocket::HostLookupState) description="L'application recherche le serveur à l'adresse: "+configurationData->getConfigurationNetwork()->getAddress();
	else if(state==QAbstractSocket::ConnectingState) description="L'application tente de se connecter au serveur";
	else if(state==QAbstractSocket::ConnectedState) description="L'application est en mode connecté au serveur";
	else if(state==QAbstractSocket::ClosingState) description="L'application est en cours de fermeture de la connexion";
	else description="La connexion réseau est à un état inconnu";
	emit connexionStateChanged(state,description);
}

