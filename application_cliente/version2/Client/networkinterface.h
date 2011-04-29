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

	//Les requetes de connexion et déconnexion
	void requestConnectToServer();
	void requestDisconnectFromServer();

	//Envoi de messages de divers types
	Response *sendMediaCreated(QString realPath,bool isDirectory, int revision);
	Response *sendMediaUpdated(QString realPath,QByteArray content, int revision);
	Response *sendMediaRemoved(QString realPath, int revision);
	Response *sendDepotRevision(QString realPath,int revision);
	bool sendEndOldDetections();

	//Pour l'exécution du thread
	void run();

	//Gérer la liste des requêtes récues
	void putReceiveRequestList(Request *r);
	Request *getReceiveRequestList();

	//Pour renseigner l'objet de reveil du hddInteface à la réception d'une requete
	void setWaitReceiveRequestList(QWaitCondition *waitReceiveRequestList);

	//Pour savoir si on est connecté
	bool checkIsConnected();

	//Bloquer le thread appelant tant qu'on ne se connecte pas
	bool blockWhileDisconnected();

signals:
	void connected();
	void identified();
	void disconnected();
	void receiveErrorMessage(QString);

	//Les signaux de requetes de connexion et déconnexion
	void connectToServerRequested();
	void disconnectFromServerRequested();

private slots:
	//Slots pour recevoir les évènement de la socket
	void stateChangedAction(QAbstractSocket::SocketState);
	void connectedToServer();
	void disconnectedFromServer();
	void connexionEncrypted();
	void erreursSsl(const QList<QSslError>&);
	void receiveMessageAction(QByteArray *message);

	//Les slots de connexion et déconnexion
	void connectToServer();
	void disconnectFromServer();

private:
	//Le constructeur
	NetworkInterface(ConfigurationNetwork *configurationNetwork,ConfigurationIdentification *configurationIdentification, QStandardItemModel *model);

	//La socket qui servira à se connecter au serveur
	Socket *socket;
	bool isConnected;

	//Les configurations nécéssaires
	ConfigurationNetwork *configurationNetwork;
	ConfigurationIdentification *configurationIdentification;

	//La condition pour les requetes recus
	QWaitCondition *waitReceiveRequestList;

	//La liste des requetes recus
	QList<Request*> *receiveRequestList;

	//Le mutex pour les accès concurents à la liste précédante
	QMutex receiveRequestListMutex;

	//La condition pour synchroniser les messages
	QWaitCondition waitMessages;

	//Le mutex pour bloquer tant qu'on est pas connecté
	QMutex blockDisconnectedMutex;

	//Pour les réponses aux requetes envoyées par le client
	Response *response;

	//Juste pour l'affichage
	QStandardItemModel *model;

	//Pour envoyer un message d'identification
	//Ne peut se faire que depuis l'intérieur de la classe
	bool sendIdentification();

};


#endif // NETWORKINTERFACE_H
