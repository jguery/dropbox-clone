#ifndef SOCKET_H
#define SOCKET_H

#include <QtNetwork>


class Socket : public QTcpSocket
{
	Q_OBJECT

public:
	Socket();
	bool connectToServer(QString address,int port);
	bool disconnectFromServer();
	bool sendMessage(QByteArray *message);

signals:
	void receiveMessage(QByteArray *message);

private slots:
	void inputStream();

private:
	quint64 blockSize;
};

#endif // SOCKET_H

