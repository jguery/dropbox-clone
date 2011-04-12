#ifndef NETWORKINTERFACE_H
#define NETWORKINTERFACE_H

#include <QStandardItemModel>
#include "configurationdata.h"
#include "socket.h"
#include "messages.h"

/*
 Cette classe implémente l'interface de communication avec le serveur
 Toutes les requêtes passent par ici
*/

class NetworkInterface: public QThread
{

	Q_OBJECT

public:
	//Une méthode statique pour créer l'objet
	static NetworkInterface *createNetworkInterface(ConfigurationNetwork *configurationNetwork,ConfigurationIdentification *configurationIdentification, QStandardItemModel *model);

	//Les fonction de connexion et déconnexion
	bool connectToServer();
	bool disconnectFromServer();

	//Envoi de messages de divers types
	ResponseEnum sendMediaCreated(QString realPath,bool isDirectory);
	ResponseEnum sendMediaUpdated(QString realPath,QByteArray content);
	ResponseEnum sendMediaRemoved(QString realPath);
	ResponseEnum sendIdentification();

	void run();

	//Gérer la liste des requetes recues
	void putReceiveRequestList(Request *r);
	Request *getReceiveRequestList();
	void setWaitReceiveRequestList(QWaitCondition *waitReceiveRequestList);

	//Pour savoir si on est connecté
	bool checkIsConnected();

	//Bloquer le thread appelant tant qu'on est pas connecté
	void blockWhileDisconnected();
signals:
	void connected();
	void disconnected();
	void receiveErrorMessage(QString);

private slots:
	//Slots pour recevoir les évènement de la socket
	void stateChangedAction(QAbstractSocket::SocketState);
	void connectedToServer();
	void disconnectedFromServer();
	void connexionEncrypted();
	void erreursSsl(const QList<QSslError>&);
	void receiveMessageAction(QByteArray *message);

private:
	//Le constructeur
	NetworkInterface(ConfigurationNetwork *configurationNetwork,ConfigurationIdentification *configurationIdentification, QStandardItemModel *model);

	//La socket qui servira à se connecter au serveur
	Socket *socket;
	bool isConnected;
	bool isIdentified;

	//Les configurations nécéssaires
	ConfigurationNetwork *configurationNetwork;
	ConfigurationIdentification *configurationIdentification;

	//La condition pour les requetes recus
	QWaitCondition *waitReceiveRequestList;

	//La liste des requetes recus
	QList<Request*> *receiveRequestList;
	QMutex receiveRequestListMutex;

	//La condition pour synchroniser les messages
	QWaitCondition waitMessages;

	//Le mutex pour bloquer tant qu'on est pas connecté
	QMutex blockDisconnectedMutex;

	//Pour les réponses aux requetes envoyées par le client
	ResponseEnum response;

	//Juste pour l'affichage
	QStandardItemModel *model;
};


#endif // NETWORKINTERFACE_H
