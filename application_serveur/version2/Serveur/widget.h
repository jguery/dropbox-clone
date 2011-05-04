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

public slots:
	void showCreateConfig();
	void showLoadConfig();
	void validerCreateConfig();
	void validerLoadConfig();


	void parcourirFilesPath();
	void ajouterDepot();
	void enleverDepot();
	void ajouterUser();
	void enleverUser();


private:

	//quelques fonctions privées, pour la construction
	void buildInterface();
	void allocateModules();

	//Le serveur
	Server *server;

	//Les onglets
	QTabWidget *onglets;
	QStandardItemModel *model;

	//La boite de dialogue de configuration
	QDialog *createConfigDialog;
	QDialog *loadConfigDialog;

	QLineEdit *portLineEdit;
	QLineEdit *filesPathLineEdit;

	QLineEdit *svnAddressLineEdit;
	QLineEdit *svnUserLineEdit;
	QLineEdit *svnPasswordLineEdit;
	QLineEdit *svnPortLineEdit;

	QLineEdit *dbNameLineEdit;
	QLineEdit *dbUserLineEdit;
	QLineEdit *dbPasswordLineEdit;

	QTableWidget *tableDepots;
	QTableWidget *tableUsers;

	//Les reglages
	QSettings *settings;
};

#endif // WIDGET_H
