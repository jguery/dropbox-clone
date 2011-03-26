#include "socket.h"


//Le constructeur
Socket::Socket() : QTcpSocket()
{
	blockSize=0;
	QObject::connect(this, SIGNAL(readyRead()), this, SLOT(inputStream()));
}

//Pour se connecter au serveur
bool Socket::connectToServer(QString address,int port)
{
	if(this->state()==ConnectedState) return true;
	connectToHost(address,port);
	bool result = waitForConnected(3000);
	return result;
}


//Se déconnecter du serveur
bool Socket::disconnectFromServer()
{
	if(this->state()!=ConnectedState) return true;
	disconnectFromHost();
	bool result = waitForDisconnected(3000);
	return result;
}



//Ce slot privé est appellé automatiquement à chaque fois qu'un bout de paquet arrive
void Socket::inputStream()
{
	QDataStream in(this);
	in.setVersion(QDataStream::Qt_4_0);
	if(blockSize==0)
	{
		if (bytesAvailable() < (int)sizeof(quint64))
			return;
		in >> blockSize;
	}

	if (bytesAvailable() < blockSize)
		return;
	blockSize=0;
	QByteArray *message=new QByteArray();
	in >> (*message);
	emit receiveMessage(message);
}


//Pour envoyer un message complet
bool Socket::sendMessage(QByteArray *message)
{
	if(message==NULL) return false;
	QByteArray block;
	QDataStream out(&block, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_0);
	out << (quint64)0;
	out << (*message);
	out.device()->seek(0);
	out << (quint64)(block.size() - sizeof(quint64));
	write(block);
	bool result = waitForBytesWritten(3000);
	delete message;
	return result;
}


