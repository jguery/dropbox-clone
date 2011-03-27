#ifndef SERVER_H
#define SERVER_H

#include "clientmanager.h"
#include <QtNetwork>
#include <QtGui>

/*
 Cette classe implémente le serveur qui traite les connexions des clients
*/

class Server: public QTcpServer
{

public:
	//Le constructeur
	Server(QStandardItemModel *model);
	//Pour écouter sur un port
	bool beginListenning(int port);
	//Arreter l'écoute sur un port
	bool stopListenning();
	//Lorsqu'un client se connecte
	void incomingConnection(int socketDescriptor);

private:
	//La liste des clients
	QVector<ClientManager*> *clients;
	QStandardItemModel *model;
};

#endif // SERVER_H
