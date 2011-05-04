#include "server.h"
#include "widget.h"



Server *Server::createServer(DatabaseManager *databaseManager,FileManager *fileManager, QTabWidget *onglets, QStandardItemModel *model)
{
	if(!databaseManager || !fileManager || !onglets || !model) return NULL;
	return new Server(databaseManager,fileManager,onglets,model);
}



//Le constructeur
Server::Server(DatabaseManager *databaseManager,FileManager *fileManager,QTabWidget *onglets,QStandardItemModel *model): QTcpServer()
{
	this->databaseManager=databaseManager;
	this->fileManager=fileManager;
	this->model=model;
	this->onglets=onglets;
	this->clients=new QVector<ClientManager*>();
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
	static int indiceClient=1;
	QTableView *tableView=new QTableView();
	QStandardItemModel *modelClient=new QStandardItemModel(0,2,tableView);
	ClientManager *cm=ClientManager::createClientManager(socketDescriptor,clients,databaseManager,fileManager,modelClient);
	if(cm==NULL) {delete tableView;return;}
	QObject::connect(cm,SIGNAL(disconnectedClient(ClientManager*)),this,SLOT(disconnectedClient(ClientManager*)));
	clients->append(cm);

	QStringList list;
	list<<trUtf8("Evenement")<<trUtf8("Heure");
	modelClient->setHorizontalHeaderLabels(list);
	tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	tableView->setModel(modelClient);
	tableView->horizontalHeader()->setStretchLastSection(true);
	tableView->setColumnWidth(0,650);
	onglets->addTab(tableView,"Client "+::QString::number(indiceClient++));
	onglets->setCurrentWidget(tableView);
	Widget::addRowToTable("Un client vient de se connecter. Nombre actuel de clients: "+QString::number(clients->size()),model,MSG_2);
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
			if(!c) continue;
			Widget::addRowToTable("Un client vient de se déconnecter. Nombre actuel de clients: "+QString::number(clients->size()),model,MSG_2);
			for(int i=1;i<onglets->count();i++)
			{
				QWidget *o=(QWidget*)(c->getModelClient()->parent());
				onglets->removeTab(onglets->indexOf(o));
				delete o;
			}
			delete c;
		}
	}
}
