#include "socket.h"

#define CACERTIFICATES_FILE "../../certs/ca/ca.pem"
#define LOCALCERTIFICATE_FILE "../../certs/client-crt.pem"
#define PRIVATEKEY_FILE "../../certs/client-key.pem"

Socket::Socket() : QSslSocket()
{
	blockSize=0;
	QObject::connect(this, SIGNAL(readyRead()), this, SLOT(inputStream()));

        //Malheureusement pour le moment, tout marche grâce à cette ligne...
        //connect(this,SIGNAL(sslErrors(QList<QSslError>)),this,SLOT(ignoreSslErrors()));
}


bool Socket::connectToServer(QString address,int port)
{
	if(this->state()==ConnectedState) return true;


        QFile file(PRIVATEKEY_FILE);

        file.open(QIODevice::ReadOnly);
        if (!file.isOpen()) {
           qWarning("could'n open %s", PRIVATEKEY_FILE);
           disconnectFromHost();
           return false;
        }

        QSslKey key(&file, QSsl::Rsa, QSsl::Pem, QSsl::PrivateKey, "serveurS2");

        if (key.isNull()) {
           qWarning("key is null");
           disconnectFromHost();
           return false;
        }
        bool b = addCaCertificates(CACERTIFICATES_FILE);
        if (!b) {
           qWarning("Couldn't add CA certificates (\"%s\")"
              , CACERTIFICATES_FILE);
        }
        setLocalCertificate(LOCALCERTIFICATE_FILE);
        setPrivateKey(key);

        connectToHostEncrypted(address,port);
	bool result = waitForConnected(3000);
	return result;
}



bool Socket::disconnectFromServer()
{
	if(this->state()!=ConnectedState) return true;
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


