#ifndef SERVER_H
#define SERVER_H

#include "clientmanager.h"
#include <QtGui>
#include <QtNetwork>

class Server: public QTcpServer
{

    Q_OBJECT

public:
	Server();
	bool beginListenning(int port);
	bool stopListenning();
	void incomingConnection(int socketDescriptor);

private:
        QVector<ClientManager*> *clients;


};

#endif // SERVER_H
