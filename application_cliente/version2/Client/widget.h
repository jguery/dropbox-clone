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

#define MSG_HDD QColor::fromRgb(0,255,155)
#define MSG_NETWORK QColor::fromRgb(255,0,0)
#define MSG_INIT QColor::fromRgb(0,255,255)

class Widget : public QWidget
{
	Q_OBJECT

public:
	//Constructeur à 3 paramètres : le localPath et le realPath du dépot sur lequel on veut travailler
	Widget(QString localPath, QString realPath, QString configSavePath);

	//Constructeur à 1 paramètre: le fichier de config du programme
	Widget(QString configPath);

	//Une méthode statique pour écrire dans la qtableview
        static void addRowToTable(QString sentence,QStandardItemModel *model, QColor color=MSG_NETWORK);

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
