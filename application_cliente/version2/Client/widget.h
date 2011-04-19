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

#define MSG_1 QColor::fromRgb(0,255,155)
#define MSG_2 QColor::fromRgb(0,255,255)
#define MSG_3 QColor::fromRgb(255,0,0)

class Widget : public QWidget
{
	Q_OBJECT

public:
	//Constructeur à 3 paramètres : le localPath et le realPath du dépot sur lequel on veut travailler
	Widget(QString localPath, QString realPath, QString configSavePath);

	//Constructeur à 1 paramètre: le fichier de config du programme
	Widget(QString configPath);

	//Pour construire l'interface graphique
	void buildInterface();

	//Une méthode statique pour écrire dans la qtableview
	static void addRowToTable(QString sentence,QStandardItemModel *model, QColor color);

public slots:
	//les slots des boutons:
	void networkButtonSlot();
	void detectionButtonSlot();

private:
	//la configuration
	ConfigurationData *configurationData;

	//L'interface réseau
	NetworkInterface *networkInterface;

	//L'interface Hdd
	HddInterface *hddInterface;

	//Les onglets
	QTabWidget *onglets;

	//Onglet réseau
	QWidget *networkOnglet;
	QStandardItemModel *networkModel;
	QPushButton *networkButton;

	//Onglet des transferts
	QWidget *transfertOnglet;
	QStandardItemModel *transfertModel;

	//Onglet des détections
	QWidget *detectionOnglet;
	QStandardItemModel *detectionModel;
	QPushButton *detectionButton;

};

#endif // WIDGET_H
