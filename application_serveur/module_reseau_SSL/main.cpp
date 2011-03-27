#include <QCoreApplication>
#include "server.h"

int main (int argc, char *argv[])
{
   QCoreApplication app(argc, argv);
   Server server;

   if (!server.beginListenning(50885)) {
       qDebug("Le serveur n'a pas pu se binder au port choisi");
       return -1;
   }

   fprintf(stdout, "Listening at port %d\n", server.serverPort());

   return app.exec();
}

