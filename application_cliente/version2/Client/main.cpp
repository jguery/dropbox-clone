#include <QtGui/QApplication>
#include "widget.h"


//La méthode main pour tester l'application
///////////////////////////////////////////

int main(int argc, char *argv[])
{
        QApplication app(argc, argv);

	//On crèe juste la fenetre et on l'affiche
	Widget w;
        w.show();

	//Exécution de l'application
        return app.exec();
}
