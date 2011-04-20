#include <QtGui/QApplication>
#include "widget.h"


//La méthode main pour tester l'application
///////////////////////////////////////////

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	//On crèe juste la 1ere fenetre et on l'affiche
	Widget w1;
	w1.show();
/*
	//On crèe juste la 2e fenetre et on l'affiche
	Widget w2;
	w2.show();
*/
	//Exécution de l'application
	return a.exec();
}
