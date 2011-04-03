#ifndef SERVER_H
#define SERVER_H

#include <QTcpServer>
#include <QStandardItemModel>
#include "clientmanager.h"

/*
 Cette classe implémente le serveur qui traite les connexions des clients
*/

class Server: public QTcpServer
{

Q_OBJECT

public:
	//Le constructeur
	Server(QStandardItemModel *model);

	//Pour écouter sur un port
	bool beginListenning(int port);

	//Arreter l'écoute sur un port
	bool stopListenning();

	//Lorsqu'un client se connecte
	void incomingConnection(int socketDescriptor);

public slots:
	void disconnectClient(ClientManager*);	//Lorsqu'un client se déconnecte, appelé pour l'enlever du QVector

private:
	//La liste des clients
	QVector<ClientManager*> *clients;
	QStandardItemModel *model;
};

#endif // SERVER_H
