#ifndef WIDGET_H
#define WIDGET_H

#include "configurationdata.h"
#include "networkinterface.h"
#include "hddinterface.h"
#include <QSystemTrayIcon>
#include <QtGui>
#include <QSettings>


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
	//Constructeur
	Widget();

	//Pour construire l'interface graphique
	void buildInterface();

	//Une méthode statique pour écrire dans la qtableview
	static void addRowToTable(QString sentence,QStandardItemModel *model, QColor color);

        // Une méthode permettant de gérer l'icone de notification et les bulles de notifications
        void buildNotification();
        void closeEvent(QCloseEvent *e);//Intercepte la fermeture du programme pour le mettre en fond

public slots:
	//les slots des boutons:
	void networkButtonSlot();
	void detectionButtonSlot();

	//Les slots des configs
	void parcourirLoadConfigSlot();
	void loadConfigSlot();
	void parcourirCreateConfigSlot();
	void createConfigSlot();
	void parcourirSaveConfigSlot();

	//Les slots de l'icone de notification

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

	//Onglet des configurations
	QWidget *configOnglet;
	QLineEdit *loadConfigLineEdit, *loadConfigMdpLineEdit, *serverAddressLineEdit, *serverPortLineEdit, *loginLineEdit, *passwordLineEdit, *depotLocalPathLineEdit, *depotRealNameLineEdit, *saveConfigLineEdit;
	QPushButton *configButton;

	//Les reglages
	QSettings *settings;

};

#endif // WIDGET_H
