#ifndef HDDINTERFACE_H
#define HDDINTERFACE_H

#include <QtCore>
#include <QtGui>
#include "configurationdata.h"
#include "networkinterface.h"

/*
 Cette classe représente l'interaction avec le disque dur.
 Elle est avertie à chaque fois qu'un média est créé, modifié ou supprimé.
 Elle est encore en cours de tests et de modifications
*/


class HddInterface: public QObject
{
	Q_OBJECT

public:
	//Une méthode statique pour l'allocation d'un objet hddInterface
	static HddInterface *createHddInterface(ConfigurationData *configurationData,NetworkInterface *networkInterface,QStandardItemModel *model);
	//Les fonctions qui sont appélées lors d'une modif sur un média
	void mediaHasBeenRemoved(Media *m,Dir *parent);
	void mediaHasBeenCreated(Media *m,Dir *parent);
	void fileHasBeenUpdated(File *f,Dir *parent);
public slots:
	void receiveModifiedFileMessageAction(File*,QByteArray);
	void receiveCreatedMediaMessageAction(Dir *parent,QString realName);
	void receiveRemovedMediaMessageAction(Media*);
	void receiveErrorMessageAction(QString s);
private:
	//Le constructeur
	HddInterface(ConfigurationData *configurationData,NetworkInterface *networkInterface,QStandardItemModel *model);
	//Les autres interfaces
	ConfigurationData *configurationData;
	NetworkInterface *networkInterface;
	QStandardItemModel *model;
};

#endif // HDDINTERFACE_H
