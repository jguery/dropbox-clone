#ifndef WIDGET_H
#define WIDGET_H

#include "server.h"
#include <QtGui>

/*
 Cette classe implémente une fenetre de test pour le serveur.
 Elle permet de voir tous les évenements par l'intermediaire d'une qtableview
*/

class Widget : public QWidget
{
	Q_OBJECT

public:
	//Constructeur
	Widget();
	//Une méthode statique pour écrire dans la qtableview
	static void addRowToTable(QString sentence,QStandardItemModel *model,bool changeColor=true);
private:
	//Lee serveur
	Server *server;
	//Le modèle de la qtableview
	QStandardItemModel *model;
};

#endif // WIDGET_H
