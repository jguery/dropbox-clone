#include "server.h"


//Le constructeur
Server::Server(QStandardItemModel *model): QTcpServer()
{
	this->model=model;
	clients=new QVector<ClientManager*>();
}


//Permet de démarrer le serveur et le mettre à l'écoute sur un port donné
bool Server::beginListenning(int port)
{
	if(this->isListening()) return false;
	if(clients!=NULL){
		while(!clients->isEmpty()){
			delete clients->at(0);
			clients->remove(0);
		}
		delete clients;clients=NULL;
	}
	clients=new QVector<ClientManager*>();
	bool result = listen(QHostAddress::Any,port);
	return result;
}


//Permet de stopper l'écoute du serveur
bool Server::stopListenning()
{
	if(clients!=NULL){
		while(!clients->isEmpty()){
			delete clients->at(0);
			clients->remove(0);
		}
		delete clients;clients=NULL;
	}
	close();
	return true;
}


//Cette fonction est automatiquement appelée lorsqu'un client se connecte
void Server::incomingConnection(int socketDescriptor)
{
	ClientManager *cm=ClientManager::createClientManager(socketDescriptor,clients,model);
	if(cm==NULL) return;
	clients->append(cm);
}

