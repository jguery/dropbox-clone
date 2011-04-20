#ifndef SOCKET_H
#define SOCKET_H

#include <QSslSocket>
#include <QSslKey>
#include <QFile>

/*
 Cette classe héritant de QSslSocket implémente une socket sécurisée
 capable de recevoir des messages xml complets du client, et de lui en envoyer.
*/

class Socket : public QSslSocket
{
	Q_OBJECT

public:
	//Constructeur et fonctions de connexion, déconnexions
	Socket();

	//Pour renseigner le descripteur du client
	bool setDescriptor(int socketDescriptor);

	bool disconnectClient();

	//Pour envoyer un message
	bool sendMessage(QByteArray *message);

signals:
	//Signal émit lorsqu'on recoit un message complet
	void receiveMessage(QByteArray *message);

private slots:
	void inputStream();

private:
	qint64 blockSize;
};

#endif // SOCKET_H
