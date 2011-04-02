#ifndef DIR_H
#define DIR_H

#include "file.h"
#include <QtCore/QFileSystemWatcher>
#include <QtCore/QDir>


class HddInterface;

/*
  Cette classe représente un repertoire sur le disque dur.
  Elle hérite de la classe Media. Un Dir possède une liste de Media qui
  peuvent être eux aussi des Dir ou des File.
*/


class Dir: public Media
{
	Q_OBJECT

public:
	//Des fonctions statiques createDir et loadDir pour allouer un objet Dir
	static Dir *createDir(QString localPath,QString realPath,int revision,bool readOnly);
	static Dir *loadDir(QDomNode noeud);

	//Pour récupérer la liste de ses sous médias
	QVector<Media*> *getSubMedias();

	//implémentation des fonctions virtuelles de la classe Media dont elle hérite
	bool isDirectory();
	bool hasBeenRemoved();

	//Retourne le code xml du repertoire
	QDomElement toXml(QDomDocument *document);

	//Des fonctions pour la recherche dans l'arborescence
	Media *findMediaByLocalPath(QString localPath);
	Media *findMediaByRealPath(QString realPath);
	Dir *findMediaParentByLocalPath(QString localPath);
	Dir *findMediaParentByRealPath(QString realPath);

	//Accesseur pour son listener(objet qu'il averti en cas de modifications)
	void setSignalListener(HddInterface *hddInterface);

	//Une methode statique qui sert à supprimer un repertoire non vide
	static void removeNonEmptyDirectory(QString path);

	//Une méthode pour chercher les modifications dans l'arborescence
	bool searchChanges(bool recursif=false);

	//Destructeur
	virtual ~Dir();

private slots:
	//Le slot qui sera appelé lorsque le repertoire a subit un changement
	void directoryChangedAction();

private:
	//Constructeur
	Dir(QString localPath,QString realPath,int revision,bool readOnly);

	//La liste de ses sous médias
	QVector<Media*> *subMedias;

	//Le watcher pour détecter les modifs
	QFileSystemWatcher *watcher;

	//Le listener pour récuperer les évenements de modifications
	HddInterface *hddInterface;
};

#endif // DIR_H
