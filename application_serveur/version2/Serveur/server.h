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
	//Pour allouer l'objet
	static Server *createServer(DatabaseManager *databaseManager,FileManager *fileManager,QTabWidget *onglets,QStandardItemModel *model);

	//Pour écouter sur un port
	bool beginListenning(int port);

	//Arreter l'écoute sur un port
	bool stopListenning();

	//Lorsqu'un client se connecte
	void incomingConnection(int socketDescriptor);

public slots:
	void disconnectedClient(ClientManager *clientManager);

private:
	//Le constructeur
	Server(DatabaseManager *databaseManager,FileManager *fileManager,QTabWidget *onglets,QStandardItemModel *model);

	//La bdd
	DatabaseManager *databaseManager;
	FileManager *fileManager;

	//La liste des clients
	QVector<ClientManager*> *clients;
	QTabWidget *onglets;
	QStandardItemModel *model;
};

#endif // SERVER_H
