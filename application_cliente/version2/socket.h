#ifndef SOCKET_H
#define SOCKET_H

#include <QtNetwork>


/*
 Cette classe héritant de QtcpSocket implémente une socket
 capable de recevoir des messages xml complets, et d'en envoyer
 Elle ne gère pas le ssl pour l'instant
*/


class Socket : public QTcpSocket
{
	Q_OBJECT

public:
	//Constructeur et fonctions de connexion, déconnexions
	Socket();

	bool connectToServer(QString address,int port);
	bool disconnectFromServer();

	//Pour envoyer un message
	bool sendMessage(QByteArray *message);

signals:
	//Signal émit lorsqu'on recoit un message complet
	void receiveMessage(QByteArray *message);

private slots:
	//Slot privé qui sera appelé à chaque fois que des petits paquets sont recus
	void inputStream();

private:
	//attribut privé pour stocker la taille du message qui est entrain d'etre recu par paquet
	quint64 blockSize;
};

#endif // SOCKET_H
