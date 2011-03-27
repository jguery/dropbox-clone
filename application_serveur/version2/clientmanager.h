#ifndef CLIENTMANAGER_H
#define CLIENTMANAGER_H

#include "socket.h"
#include <QtGui>


/*
 Un objet de cette classe est alloué à chaque client qui se connecte.
 Il est chargé de communiquer avec le client.
*/

class ClientManager: private QObject
{
	Q_OBJECT

public:
	//Une méthode statique pour allocation
	static ClientManager *createClientManager(int clientSocket,QVector<ClientManager*> *clients,QStandardItemModel *model);

public slots:
	//Le slot levé lorsqu'un message est recu
	void receiveMessageAction(QByteArray *message);

private:
	ClientManager(QVector<ClientManager*> *clients,QStandardItemModel *model);
	//Le socket communiquant avec le client
	Socket *socket;
	//La liste des autres clients
	QVector<ClientManager*> *clients;
	QStandardItemModel *model;
};

#endif // CLIENTMANAGER_H
