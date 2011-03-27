#ifndef SOCKet_H
#define SOCKet_H

#include <QtNetwork/QtNetwork>

class Socket : public QSslSocket
{
	Q_OBJECT

public:
	Socket();
	bool setDescriptor(int socketDescriptor);
	bool disconnectClient();
	bool sendMessage(QByteArray *message);

signals:
	void receiveMessage(QByteArray *message);

private slots:
	void inputStream();

private:
	quint64 blockSize;
};

#endif // SOCKet_H
