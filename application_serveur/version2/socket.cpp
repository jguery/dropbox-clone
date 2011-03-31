#include "socket.h"


#define CACERTIFICATES_FILE "../ssl/ca.pem"
#define LOCALCERTIFICATE_FILE "../ssl/server-crt.pem"
#define PRIVATEKEY_FILE "../ssl/server-key.pem"



//Le constructeur
Socket::Socket() : QSslSocket()
{
	blockSize=0;
	QObject::connect(this, SIGNAL(readyRead()), this, SLOT(inputStream()));
}





//Permet de renseigner du client avec qui le socket doir communiquer
bool Socket::setDescriptor(int socketDescriptor)
{
	if(this->state()==QAbstractSocket::ConnectedState) return false;
	if(!this->setSocketDescriptor(socketDescriptor)) return false;


	//On récupère la clé privée du serveur
	QFile file(PRIVATEKEY_FILE);
	if(!file.open(QIODevice::ReadOnly))
	{
		qDebug("La clé privée du serveur est introuvable.");
		return false;
	}

	QSslKey key(&file, QSsl::Rsa, QSsl::Pem, QSsl::PrivateKey, "dropbox");
	if (key.isNull())
	{
		qDebug("La clé privée du serveur est nulle");
		return false;
	}
	file.close();
	setPrivateKey(key);

	//on charge le certificat du client
	setLocalCertificate( LOCALCERTIFICATE_FILE );

	//on charge le certificat de notre ca
	if(!addCaCertificates(CACERTIFICATES_FILE))
	{
		qDebug("Impossible de charger le certificat du CA.");
		return false;
	}

	//on supprime la vérification du serveur
	setPeerVerifyMode(QSslSocket::VerifyNone);

	//on ignore les erreurs car on a un certificat auto signé
	ignoreSslErrors();

	//on se connecte au serveur
	startServerEncryption();

	//On attends au plus 3secondes pour que la connexion s'établisse
	bool result = waitForConnected(3000);

	return result;
}







//Se déconnecter
bool Socket::disconnectClient()
{
	if(this->state()!=QAbstractSocket::ConnectedState) return true;
	disconnectFromHost();
	bool result = waitForDisconnected(3000);
	return result;
}


//Le slot de reception de données
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


//Pour envoyer un message
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


