#ifndef WIDGET_H
#define WIDGET_H

#include <QtGui>
#include "server.h"

/*
 Cette classe implémente une fenetre de test pour le serveur.
 Elle permet de voir tous les évenements par l'intermediaire d'une qtableview
*/

#define MSG_1 QColor::fromRgb(0,255,155)
#define MSG_2 QColor::fromRgb(0,255,255)
#define MSG_3 QColor::fromRgb(255,0,0)

class Widget : public QWidget
{
	Q_OBJECT

public:
	//Constructeur
	Widget();
	//Une méthode statique pour écrire dans la qtableview
	static void addRowToTable(QString sentence,QStandardItemModel *model, QColor color);
private:
	//Lee serveur
	Server *server;
	//Le modèle de la qtableview
	QStandardItemModel *model;
};

#endif // WIDGET_H
