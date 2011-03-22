#ifndef CLIENTMANAGER_H
#define CLIENTMANAGER_H

#include "socket.h"


class ClientManager: private QObject
{
	Q_OBJECT

public:
	static ClientManager *createClientManager(int clientSocket);

public slots:
	void receiveMessageAction(QByteArray *message);

private:
	ClientManager();
	Socket *socket;
};

#endif // CLIENTMANAGER_H
