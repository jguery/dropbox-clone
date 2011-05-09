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





//Permet de renseigner le client avec qui le socket doit communiquer
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

        QSslKey key(&file, QSsl::Rsa, QSsl::Pem, QSsl::PrivateKey, "pass");
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

        //on supprime la vérification des certificats des clients
        //seuls ces derniers vérifient les clés et certificat du serveur
        setPeerVerifyMode(QSslSocket::VerifyNone);

	//on ignore les erreurs car on a un certificat auto signé
        ignoreSslErrors();

	//on se connecte au serveur
	startServerEncryption();

	//On attends au plus 30secondes pour que la connexion s'établisse
	bool result = waitForConnected();
	if(!result) return result;

	result=waitForEncrypted();

	return result;
}





//Se déconnecter
bool Socket::disconnectClient()
{
	disconnectFromHost();
	bool result = waitForDisconnected();
	return result;
}





//Ce slot privé est appellé automatiquement à chaque fois qu'un bout de paquet arrive
void Socket::inputStream()
{
	QDataStream in(this);
	in.setVersion(QDataStream::Qt_4_0);

	//Si on a pas encore la taille du message
	if(blockSize==0)
	{
		//On vérifie si le message contient assez de bits pour lire la taille
		if (bytesAvailable() < (int)sizeof(qint64))
			return;
		//On lit la taille
		in >> blockSize;
	}

	//On vérifie si le message a été completement recu
	if (bytesAvailable() < blockSize)
		return;

	//On réinitialise la taille pour un autre futur message
	blockSize=0;

	//On lit le message recu
	QByteArray *message=new QByteArray();
	in >> (*message);

	qDebug("Recu=");
	qDebug(*message);

	//On emet un signal pour donner le message recu
	emit receiveMessage(message);
}






//Pour envoyer un message complet
bool Socket::sendMessage(QByteArray *message)
{
	if(message==NULL) return false;

	qDebug("Envoi=");
	qDebug(*message);

	//On construit le message à envoyer (avec les entetes)
	QByteArray block;
	QDataStream out(&block, QIODevice::WriteOnly);

	//Tous les messages envoyés ont la version Qt 4.0 (pour assurer les compatibilités)
	out.setVersion(QDataStream::Qt_4_0);

	//On laisse de la place au début pour écrire la taille du message
	out << (qint64)0;

	//On écrit le message
	out << (*message);

	//On se replace au début et on écrit la taille du message
	out.device()->seek(0);
	out << (qint64)(block.size() - sizeof(qint64));

	//On envoi le message final et vérifie que l'envoi a bien commencé
	if(write(block)==-1) return false;

	//On attends pour l'envoi
	waitForBytesWritten();

	//On supprime le message et on retourne le résultat de l'envoi
	delete message;
	return true;
}


