#include "clientmanager.h"


ClientManager::ClientManager(): QObject()
{
	socket=new Socket();
	QObject::connect(socket, SIGNAL(receiveMessage(QByteArray*)), this, SLOT(receiveMessageAction(QByteArray*)));
}


ClientManager *ClientManager::createClientManager(int clientSocket)
{
	ClientManager *cl=new ClientManager();
	if(cl->socket->setDescriptor(clientSocket)) return cl;
	return NULL;
}


void ClientManager::receiveMessageAction(QByteArray *message)
{
	//Cette fonction est automatiquement appelé lorsqu'un client envoi un message.

}
