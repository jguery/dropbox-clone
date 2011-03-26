#include "clientmanager.h"
#include "widget.h"


//Le constructeur
ClientManager::ClientManager(QVector<ClientManager*> *clients,QStandardItemModel *model): QObject()
{
	socket=new Socket();
	this->model=model;
	this->clients=clients;
	QObject::connect(socket, SIGNAL(receiveMessage(QByteArray*)), this, SLOT(receiveMessageAction(QByteArray*)));
}

//Pour allouer l'objet ClientManager
ClientManager *ClientManager::createClientManager(int clientSocket,QVector<ClientManager*> *clients,QStandardItemModel *model)
{
	ClientManager *cl=new ClientManager(clients,model);
	if(cl->socket->setDescriptor(clientSocket))
	{
		Widget::addRowToTable("Le client "+cl->socket->peerAddress().toString()+" vient de se connecter",model);
		return cl;
	}
	delete cl;
	return NULL;
}

//Cette fonction est automatiquement appelé lorsqu'un client envoi un message.
//Pour l'instant on ne fait que renvoyer simplement le message à tous les autres clients
void ClientManager::receiveMessageAction(QByteArray *message)
{
	Widget::addRowToTable("Le client "+this->socket->peerAddress().toString()+" a détecté un changement.",model);
	for(int i=0;i<clients->size();i++)
	{
		if(clients->at(i)!=this) clients->at(i)->socket->sendMessage(message);
	}
}
