#ifndef WIDGET_H
#define WIDGET_H

#include "configurationdata.h"
#include "networkinterface.h"
#include "hddinterface.h"
#include <QtGui>

/*
 Cette classe implémente une fenetre de test pour le client.
 Elle permet de voir tous les évenements par l'intermediaire d'une qtableview
*/

class Widget : public QWidget
{
	Q_OBJECT

public:
	//Constructeur
	Widget();

	//Une méthode statique pour écrire dans la qtableview
	static void addRowToTable(QString sentence,QStandardItemModel *model);

private:
	//la configuration
	ConfigurationData *configurationData;

	//L'interface réseau
	NetworkInterface *networkInterface;

	//L'interface Hdd
	HddInterface *hddInterface;

	//Le modèle de la qtableview
	QStandardItemModel *model;
};

#endif // WIDGET_H
