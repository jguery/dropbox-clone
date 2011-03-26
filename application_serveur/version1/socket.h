#ifndef SOCKet_H
#define SOCKet_H

#include <QtNetwork>

/*
 Cette classe héritant de QtcpSocket implémente une socket
 capable de recevoir des messages xml complets du client, et de lui en envoyer.
 Elle ne gère pas le ssl pour l'instant
*/

class Socket : public QTcpSocket
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
	quint64 blockSize;
};

#endif // SOCKet_H
