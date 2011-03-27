#include "socket.h"


Socket::Socket() : QSslSocket()
{
	blockSize=0;
	QObject::connect(this, SIGNAL(readyRead()), this, SLOT(inputStream()));
}


bool Socket::setDescriptor(int socketDescriptor)
{
	if(this->state()==QAbstractSocket::ConnectedState) return false;
        if(!this->setSocketDescriptor(socketDescriptor)) {
            qWarning("Couldn't set socket descriptor");
            return false;
        }
	bool result = waitForConnected(3000);
	return result;
}


bool Socket::disconnectClient()
{
        if(this->state()==QAbstractSocket::UnconnectedState) return true;
	disconnectFromHost();
	bool result = waitForDisconnected(3000);
	return result;
}


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

        qDebug() << *message;   //Test: affiche tous les messages qui arrivent

	emit receiveMessage(message);
}



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
	return waitForBytesWritten(3000);
}


