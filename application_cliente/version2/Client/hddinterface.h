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
	int detectedRemovedMedia(Media *m);
	int detectedCreatedMedia(Media *m);
	int detectedUpdatedMedia(Media *m);

	//Les fonctions appelées à la reception d'une requete
	void receivedRemovedRequest(QString realPath);
	void receivedCreatedRequest(QString realPath,bool isDirectory);
	void receivedUpdatedRequest(QString realPath,QByteArray content);

	//Pour l'exécution du Thread
	void run();

public slots:
	void startTransaction();

private:
	//Le constructeur
	HddInterface(ConfigurationData *configurationData,NetworkInterface *networkInterface,QStandardItemModel *model);

	//On a recu une requete
	void receivedRequest(Request *r);
	void detectedMedia(Media *m);

	bool sendDepotsRevisions();

	//Les autres interfaces
	ConfigurationData *configurationData;
	NetworkInterface *networkInterface;

	//Pour reveiller le thread dès la détection d'une modification
	QMutex mutexWaitCondition;
	QWaitCondition waitCondition;

	//Juste pour l'affichage
	QStandardItemModel *model;

	//pour savoir si le mode permet d'envoyer des requetes
	bool canSend;

};

#endif // HDDINTERFACE_H
