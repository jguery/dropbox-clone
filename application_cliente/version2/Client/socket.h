#ifndef SOCKET_H
#define SOCKET_H

#include <QtNetwork>


/*
 Cette classe héritant de QSslSocket implémente une socket sécurisée
 capable de recevoir des messages xml complets, et d'en envoyer
*/


class Socket : public QSslSocket
{
	Q_OBJECT

public:
	//Constructeur et fonctions de connexion, déconnexions
	Socket(QObject *parent);

	//Pour se connecter et se déconnecter
	bool connectToServer(QString address,int port);
	bool disconnectFromServer();

	//Pour envoyer un message
	bool sendMessage(QByteArray *message);

signals:
	//Signal émit lorsqu'on recoit un message complet
	void receiveMessage(QByteArray *message);

private slots:
	//Slot privé qui sera appelé à chaque fois que des petits paquets seront recus
	void inputStream();

private:
	//Pour initialiser la communication ssl
	//bool initialiseSSL();

	//attribut privé pour stocker la taille du message qui est entrain d'etre recu par paquet
	qint64 blockSize;
};

#endif // SOCKET_H
