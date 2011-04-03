#include "clientmanager.h"
#include "widget.h"


//Le constructeur
ClientManager::ClientManager(QVector<ClientManager*> *clients,QStandardItemModel *model): QObject()
{
	socket=new Socket();
	this->model=model;
	this->clients=clients;

	QObject::connect(socket, SIGNAL(receiveMessage(QByteArray*)), this, SLOT(receiveMessageAction(QByteArray*)));
        QObject::connect(socket, SIGNAL(encrypted()), this, SLOT(connexionEncrypted()));
        QObject::connect(socket, SIGNAL(disconnected()), this, SLOT(clientDisconnected()));
        QObject::connect(socket, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(erreursSsl(QList<QSslError>)));
}

//Pour allouer l'objet ClientManager
ClientManager *ClientManager::createClientManager(int clientSocket,QVector<ClientManager*> *clients,QStandardItemModel *model)
{
	ClientManager *cl=new ClientManager(clients,model);
	if(cl->socket->setDescriptor(clientSocket))
	{
                Widget::addRowToTable("Le client "+cl->socket->peerAddress().toString()+" vient de se connecter en mode non crypté",model);
		return cl;
	}
	delete cl;
	return NULL;
}


//Recu quand la connexion avec le client est crypté par SSL
void ClientManager::connexionEncrypted()
{
     Widget::addRowToTable("Le client "+socket->peerAddress().toString()+" est maintenant connecté en mode crypté",model);
}


//Cette fonction est automatiquement appelé lorsqu'un client envoi un message.
//Pour l'instant on ne fait que renvoyer simplement le message à tous les autres clients
void ClientManager::receiveMessageAction(QByteArray *message)
{
	Widget::addRowToTable("Le client "+this->socket->peerAddress().toString()+" a détecté un changement.",model);
	Widget::addRowToTable(QString(*message),model,false);
	for(int i=0;i<clients->size();i++)
	{
		if(clients->at(i)!=this) clients->at(i)->socket->sendMessage(message);
	}
	Widget::addRowToTable("Le changement a été transmit à "+QString::number(clients->size()-1)+" clients.",model,false);
}


//Slot appelé quand un client se déconnecte
void ClientManager::clientDisconnected()
{
    QSslSocket *client = qobject_cast<QSslSocket*>(sender());
    Widget::addRowToTable("Le client "+ client->peerAddress().toString()+" s'est déconnecté.",model,false);

    emit clientManagerStop(this);
}

void ClientManager::erreursSsl(const QList<QSslError> &errors)
{
    foreach(const QSslError &error, errors)
    {
        Widget::addRowToTable("Erreur SSL ignorée: "+ error.errorString(), model,false);
    }
}
