#include <QtGui/QApplication>
#include "widget.h"



QString APPLICATION_NAME="Clone-DropBox";



//La méthode main pour tester le serveur
///////////////////////////////////////////



int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	//On créé la fenetre et on l'affiche
	Widget w;
	w.show();

	//On exécute l'application
	return a.exec();
}
