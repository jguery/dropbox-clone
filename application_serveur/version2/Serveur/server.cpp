#include "server.h"




Server *Server::createServer(DatabaseManager *databaseManager,FileManager *fileManager, QStandardItemModel *model)
{
	if(!databaseManager || !fileManager || !model) return NULL;
	return new Server(databaseManager,fileManager,model);
}



//Le constructeur
Server::Server(DatabaseManager *databaseManager,FileManager *fileManager,QStandardItemModel *model): QTcpServer()
{
	this->databaseManager=databaseManager;
	this->fileManager=fileManager;
	this->model=model;
	clients=new QVector<ClientManager*>();
}




//Permet de démarrer le serveur et le mettre à l'écoute sur un port donné
bool Server::beginListenning(int port)
{
	if(this->isListening()) return false;
	if(clients!=NULL) {
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
	if(clients!=NULL) {
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
	ClientManager *cm=ClientManager::createClientManager(socketDescriptor,clients,databaseManager,fileManager,model);
	if(cm==NULL) return;
	clients->append(cm);
	QObject::connect(cm,SIGNAL(disconnectedClient(ClientManager*)),this,SLOT(disconnectedClient(ClientManager*)));
}






//Slot lorsqu'un client se déconnecte
//On l'envèle de la liste des clients connectés
void Server::disconnectedClient(ClientManager *clientManager)
{
	for(int i=0;i<clients->size();i++)
	{
		if(clients->at(i)==clientManager)
		{
			ClientManager *c=clients->at(i);
			clients->remove(i);
			delete c;
		}
	}
}
