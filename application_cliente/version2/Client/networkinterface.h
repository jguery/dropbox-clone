#ifndef NETWORKINTERFACE_H
#define NETWORKINTERFACE_H

#include "configurationdata.h"
#include "socket.h"
#include "messages.h"

/*
 Cette classe implémente l'interface de communication avec le serveur
 Toutes les requêtes passent par ici
*/

class NetworkInterface: public QObject
{
	Q_OBJECT

public:
	//Une méthode statique pour créer l'objet
	static NetworkInterface *createNetworkInterface(ConfigurationData *configurationData);

	//Les fonction de connexion et déconnexion
	bool connect();
	bool disconnect();

	//Envoi de messages de divers types
	bool sendIdentification();
	bool sendFileModified(QString realPath,QByteArray content);
	bool sendMediaCreated(QString realPath);
	bool sendMediaRemoved(QString realPath);

signals:
	//Signaux pour connaitre l'état de la connexion
	void connexionStateChanged(QAbstractSocket::SocketState state,QString stateDescription);
	void connectedToServer();
	void disconnectedFromServer();

	//Signaux de réceptions de messages de divers types
	void receiveErrorMessage(QString);
	void receiveModifiedFileMessage(File*,QByteArray);
	void receiveCreatedMediaMessage(Dir *parent,QString realName);
	void receiveRemovedMediaMessage(Media*);
	void receiveValidationMessage();
	void receiveAnnulationMessage();

private slots:
	//Slots pour recevoir les évènement de la socket
	void stateChangedAction(QAbstractSocket::SocketState);
	void receiveMessageAction(QByteArray *message);

private:
	//Le constructeur
	NetworkInterface(ConfigurationData *configurationData);

	//La socket qui servira à se connecter au serveur
	Socket *socket;

	//La configuration totale
	ConfigurationData *configurationData;
};


#endif // NETWORKINTERFACE_H
