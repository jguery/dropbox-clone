#include <QApplication>

#include "socket.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    Socket socket;
    socket.connectToServer("127.0.0.1",50885);

    QObject::connect(&socket,SIGNAL(disconnected()),qApp,SLOT(quit()));

    //Teste l'envoie de données cryptées
    QByteArray * msg = new QByteArray("Salut les garçons!");
    socket.sendMessage(msg);

    return app.exec();
}

