#include "clientmanager.h"


#define CACERTIFICATES_FILE "../../certs/ca/ca.pem"
#define LOCALCERTIFICATE_FILE "../../certs/server-crt.pem"
#define PRIVATEKEY_FILE "../../certs/server-key.pem"

ClientManager::ClientManager(): QObject()
{
	socket=new Socket();
	QObject::connect(socket, SIGNAL(receiveMessage(QByteArray*)), this, SLOT(receiveMessageAction(QByteArray*)));
}


ClientManager *ClientManager::createClientManager(int clientSocket)
{
	ClientManager *cl=new ClientManager();

        cl->socket->setProtocol(QSsl::AnyProtocol);

        if (!cl->socket->setDescriptor(clientSocket)) {
           qWarning("couldn't set socket descriptor");
           delete cl->socket;
           return NULL;
        }
        cl->connectSocketSignals();
        cl->startServerEncryption();

        return cl;
}


/****************************************************
  Récupère les clés et certificats SSL (créés avec
  openssl) pour la phase de handshake
****************************************************/
void ClientManager::startServerEncryption ()
{
   QFile file(PRIVATEKEY_FILE);

   file.open(QIODevice::ReadOnly);
   if (!file.isOpen()) {
      qWarning("could'n open %s", PRIVATEKEY_FILE);
      socket->disconnectFromHost();
      return;
   }

   //Récupère la clé privée qui se trouve dans le fichier que l'on vient d'ouvrir
   QSslKey key(&file, QSsl::Rsa, QSsl::Pem, QSsl::PrivateKey, "serveurS2");

   if (key.isNull()) {      //Vérifie que la clé n'est pas nulle
      qWarning("key is null");
      socket->disconnectFromHost();
      return;
   }

   //Ajoute le certificat à la base de données de certificats que le serveur
   //peut reconnaitre
   bool b = socket->addCaCertificates(CACERTIFICATES_FILE);
   if (!b) {
      qWarning("Couldn't add CA certificates (\"%s\")"
         , CACERTIFICATES_FILE);
   }
   else {
      socket->setLocalCertificate(LOCALCERTIFICATE_FILE);   //Notre certificat est celui de notre serveur
      socket->setPrivateKey(key);                           //Attribue la clé key comme clé privé de notre serveur
      socket->startServerEncryption();                      //Lance la phase de handshake
   }
}


void ClientManager::receiveMessageAction(QByteArray *message)
{
	//Cette fonction est automatiquement appelé lorsqu'un client envoi un message.

}


/********************************************************
  Sert au débbugage. Chaque fois que la socket qui lie
  le serveur à un client émet un signal, on l'affiche sur
  la sortie du déboggeur standard.
********************************************************/
void ClientManager::connectSocketSignals ()
{
   connect(socket, SIGNAL(encrypted()), this, SLOT(slot_encrypted()));
   connect(socket, SIGNAL(encryptedBytesWritten(qint64)),
      this, SLOT(slot_encryptedBytesWritten(qint64)));
   connect(socket, SIGNAL(modeChanged(QSslSocket::SslMode)),
      this, SLOT(slot_modeChanged(QSslSocket::SslMode)));
   connect(socket, SIGNAL(peerVerifyError(const QSslError &)),
      this, SLOT(slot_peerVerifyError (const QSslError &)));
   connect(socket, SIGNAL(sslErrors(const QList<QSslError> &)),
      this, SLOT(slot_sslErrors(const QList<QSslError> &)));
   connect(socket, SIGNAL(readyRead()),
      this, SLOT(slot_readyRead()));
   connect(socket, SIGNAL(connected()),
      this, SLOT(slot_connected()));
   connect(socket, SIGNAL(disconnected()),
      this, SLOT(slot_disconnected()));
   connect(socket, SIGNAL(error(QAbstractSocket::SocketError)),
      this, SLOT(slot_error(QAbstractSocket::SocketError)));
   connect(socket, SIGNAL(hostFound()),
      this, SLOT(slot_hostFound()));
   connect(socket, SIGNAL(proxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *)),
      this, SLOT(slot_proxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *)));
   connect(socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)),
      this, SLOT(slot_stateChanged(QAbstractSocket::SocketState)));
}

void ClientManager::slot_encrypted ()
{
   qDebug("ClientManager::slot_encrypted");
}

void ClientManager::slot_encryptedBytesWritten (qint64 written)
{
   qDebug("ClientManager::slot_encryptedBytesWritten(%ld)", (long) written);
}

void ClientManager::slot_modeChanged (QSslSocket::SslMode mode)
{
   qDebug("ClientManager::slot_modeChanged(%d)", mode);
}

void ClientManager::slot_peerVerifyError (const QSslError &err)
{
   qDebug() << "ClientManager::slot_peerVerifyError( " << err.errorString() << " ) ";
}

void ClientManager::slot_sslErrors (const QList<QSslError> &errs)
{
   QList<QSslError> er = errs;

   qDebug("ClientManager::slot_sslErrors");
   QList<QSslError>::iterator i;
   for(i=er.begin();
       i!=er.end();
       i++)
   {
       qDebug() << i->errorString();
   }
}

void ClientManager::slot_readyRead ()
{
   qDebug("ClientManager::slot_readyRead");
}

void ClientManager::slot_connected ()
{
   qDebug("ClientManager::slot_connected");
}

void ClientManager::slot_disconnected ()
{
   qDebug("ClientManager::slot_disconnected");
}

void ClientManager::slot_error (QAbstractSocket::SocketError err)
{
   qDebug() << "ClientManager::slot_error(" << err << ")";
   qDebug() << socket->errorString();
}

void ClientManager::slot_hostFound ()
{
   qDebug("ClientManager::slot_hostFound");
}

void ClientManager::slot_proxyAuthenticationRequired (const QNetworkProxy &, QAuthenticator *)
{
   qDebug("ClientManager::slot_proxyAuthenticationRequired");
}

void ClientManager::slot_stateChanged (QAbstractSocket::SocketState state)
{
   qDebug() << "Server::slot_stateChanged(" << state << ")";
}
