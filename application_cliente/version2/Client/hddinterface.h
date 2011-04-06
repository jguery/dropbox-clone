#ifndef HDDINTERFACE_H
#define HDDINTERFACE_H

#include <QtCore>
#include <QtGui>
#include "configurationdata.h"
#include "networkinterface.h"

/*
 Cette classe représente l'interaction avec le disque dur.
 Elle est avertie à chaque fois qu'un média est créé, modifié ou supprimé.
*/

class HddInterface: public QThread
{
	Q_OBJECT

public:
	//Une méthode statique pour l'allocation d'un objet hddInterface
	static HddInterface *createHddInterface(ConfigurationData *configurationData,NetworkInterface *networkInterface,QStandardItemModel *model);

	//Les fonctions qui sont appélées lors d'une modif sur un média
	int detectRemovedMedia(Media *m);
	int detectCreatedMedia(Media *m);
	int detectUpdatedMedia(Media *m);

	//Pour l'exécution du Thread
	void run();

public slots:
	//Les slots appelés lorsqu'on recoit un message du serveur
	void receiveRemovedMedia(QString realPath);
	void receiveCreatedMedia(QString realPath,bool isDirectory);
	void receiveUpdatedMedia(QString realPath,QByteArray content);
	void receiveError(int errorNumber);

private:
	//Le constructeur
	HddInterface(ConfigurationData *configurationData,NetworkInterface *networkInterface,QStandardItemModel *model);

	//Les autres interfaces
	ConfigurationData *configurationData;
	NetworkInterface *networkInterface;

	//Pour reveiller le thread dès la détection d'une modification
	QMutex mutexWaitCondition;
	QWaitCondition waitConditionDetect;

	//Juste pour l'affichage
	QStandardItemModel *model;
};

#endif // HDDINTERFACE_H
