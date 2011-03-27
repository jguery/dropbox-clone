#ifndef CLIENTMANAGER_H
#define CLIENTMANAGER_H

#include <QFile>
#include "socket.h"


class ClientManager: private QObject
{
	Q_OBJECT

public:
	static ClientManager *createClientManager(int clientSocket);

public slots:
	void receiveMessageAction(QByteArray *message);

private slots:
        void slot_encrypted ();
        void slot_encryptedBytesWritten (qint64 written);
        void slot_modeChanged (QSslSocket::SslMode mode);
        void slot_peerVerifyError (const QSslError &error);
        void slot_sslErrors (const QList<QSslError> &errors);
        void slot_connected ();
        void slot_disconnected ();
        void slot_error (QAbstractSocket::SocketError);
        void slot_hostFound ();
        void slot_proxyAuthenticationRequired (const QNetworkProxy &, QAuthenticator *);
        void slot_stateChanged (QAbstractSocket::SocketState);
        void slot_readyRead ();

private:
	ClientManager();
	Socket *socket;

        void connectSocketSignals();
        void startServerEncryption();

};

#endif // CLIENTMANAGER_H
