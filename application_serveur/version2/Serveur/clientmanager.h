#ifndef CLIENTMANAGER_H
#define CLIENTMANAGER_H

#include <QStandardItemModel>
#include <QtCore>
#include "socket.h"
#include "messages.h"
#include "databasemanager.h"
#include "filemanager.h"


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

	void run();

	~ClientManager();

private slots:
	//Le slot levé lorsqu'un message est recu
	void receiveMessageAction(QByteArray *message);

	//Lorsqu'une requete est récue
	void receivedRequest(Request *r);

        //Lorsqu'un client est déconnecté
        void clientDisconnected();

        //On recoit des erreurs lors de la connexion SSL
        void erreursSsl(const QList<QSslError>&);

        void connexionEncrypted();
signals:
	void disconnectedClient(ClientManager *clientManager);

private:
	ClientManager(QVector<ClientManager*> *clients,DatabaseManager *databaseManager,FileManager *fileManager,QStandardItemModel *model);

        //La socket communiquant avec le client
	Socket *socket;

	//La liste des autres clients
	QVector<ClientManager*> *clients;

	//La bdd
	DatabaseManager *databaseManager;
	FileManager *fileManager;

	QStandardItemModel *model;
};

#endif // CLIENTMANAGER_H
