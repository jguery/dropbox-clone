#ifndef CLIENTMANAGER_H
#define CLIENTMANAGER_H

#include <QStandardItemModel>
#include <QtCore>
#include "socket.h"
#include "messages.h"
#include "databasemanager.h"
#include "filemanager.h"


typedef enum ClientState {

	CONNECTED,
	CLIENT_DETECTIONS,
	SERVER_DETECTIONS,
	SYNCHRONIZED

} ClientState;



/*
 Un objet de cette classe est alloué à chaque client qui se connecte.
 Il est chargé de communiquer avec le client.
*/

class ClientManager: public QThread
{
	Q_OBJECT

public:
	//Une méthode statique pour allocation
	static ClientManager *createClientManager(int clientSocket,QVector<ClientManager*> *clients,DatabaseManager *databaseManager,FileManager *fileManager,QStandardItemModel *model);

	//Pour l'exécution du thread
	void run();

	//Le destructeur
	~ClientManager();

	void sendDetectionRequest(QByteArray *request);

private slots:
	//Le slot levé lorsqu'un message est recu
	void receiveMessageAction(QByteArray *message);

	//Lorsqu'une requete est récue
	void receivedRequest(Request *r);

        //Lorsqu'un client est déconnecté
        void clientDisconnected();

        //On recoit des erreurs lors de la connexion SSL
        void erreursSsl(const QList<QSslError>&);

	//La connexion est cryptée
        void connexionEncrypted();

	void sendDetection(QByteArray *request);

signals:
	//Pour avertir le serveur de la déconnexion du client
	void disconnectedClient(ClientManager *clientManager);

	void sendDetectionRequested(QByteArray *request);

private:
	//Constructeur
	ClientManager(int clientSocket,QVector<ClientManager*> *clients,DatabaseManager *databaseManager,FileManager *fileManager,QStandardItemModel *model);

        //La socket communiquant avec le client
	Socket *socket;
	int clientSocket;

	//La liste des autres clients
	QVector<ClientManager*> *clients;

	//La bdd
	DatabaseManager *databaseManager;
	FileManager *fileManager;

	//L'état
	ClientState state;
	SqlUser *user;
	QString clientDescription;

	//Le model
	QStandardItemModel *model;

	//Pour mettre à jour les clients qui ne sont pas à la bonne révision
	QList<Request*> upgrading;

};

#endif // CLIENTMANAGER_H
