#include "socket.h"


#define CACERTIFICATES_FILE "../ssl/ca.pem"
#define LOCALCERTIFICATE_FILE "../ssl/client-crt.pem"
#define PRIVATEKEY_FILE "../ssl/client-key.pem"



//Le constructeur
Socket::Socket() : QSslSocket()
{
	blockSize=0;

	//On connecte le signal de paquets arrivés au slot inputStream
	QObject::connect(this, SIGNAL(readyRead()), this, SLOT(inputStream()));
}




//Pour se connecter au serveur
bool Socket::connectToServer(QString address,int port)
{
	//On vérifie qu'on est pas déjà connecté
	if(this->state()==ConnectedState) return true;

	//On récupère la clé privée du client
	QFile file(PRIVATEKEY_FILE);
	if(!file.open(QIODevice::ReadOnly))
	{
		qDebug("La clé privée du client est introuvable.");
		return false;
	}

	QSslKey key(&file, QSsl::Rsa, QSsl::Pem, QSsl::PrivateKey, "dropbox");
	if (key.isNull())
	{
		qDebug("La clé privée du client est nulle");
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
	connectToHostEncrypted(address, port);

	//On attends au plus 3secondes pour que la connexion s'établisse
	bool result = waitForConnected(3000);

	//On retourne le résultat true/false
	return result;
}




//Se déconnecter du serveur
bool Socket::disconnectFromServer()
{
	//On vérifie qu'on est pas déjà déconnecté
	if(this->state()!=ConnectedState) return true;

	//On se déconnecte. Maximum 3secondes pour la déconnexion
	disconnectFromHost();
	bool result = waitForDisconnected(3000);

	//On retourne le résultat true/false
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
		if (bytesAvailable() < (int)sizeof(quint64))
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

	//On emet un signal pour donner le message recu
	emit receiveMessage(message);
}


//Pour envoyer un message complet
bool Socket::sendMessage(QByteArray *message)
{
	if(message==NULL) return false;

	//On construit le message à envoyer (avec les entetes)
	QByteArray block;
	QDataStream out(&block, QIODevice::WriteOnly);

	//Tous les messages envoyés ont la version Qt 4.0 (pour assurer les compatibilités)
	out.setVersion(QDataStream::Qt_4_0);

	//On laisse de la place au début pour écrire la taille du message
	out << (quint64)0;

	//On écrit le message
	out << (*message);

	//On se replace au début et on écrit la taille du message
	out.device()->seek(0);
	out << (quint64)(block.size() - sizeof(quint64));

	//On envoi le message final
	write(block);

	//On attends maximum 3 secondes pour l'envoi
	bool result = waitForBytesWritten(3000);

	//On supprime le message et on retourne le résultat de l'envoi
	delete message;
	return result;
}


