#ifndef DIR_H
#define DIR_H

#include "file.h"
#include <QtCore/QFileSystemWatcher>
#include <QtCore/QDir>
#include <QtCore/QVector>



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
	static Dir *createDir(QString localPath,QString realPath,Dir *parent,int revision,bool readOnly);
	static Dir *loadDir(QDomNode noeud,Dir *parent);

	//Pour gérer la liste de ses sous médias
	int numberSubMedia();
	Media *getSubMedia(int i);
	File *addSubFile(QString localPath,QString realPath,int revision,bool readOnly);
	Dir *addSubDir(QString localPath,QString realPath,int revision,bool readOnly);
	void delSubMedia(Media *m);

	//implémentation des fonctions virtuelles de la classe Media dont elle hérite
	bool isDirectory();
	bool hasBeenRemoved();

	//Retourne le code xml du repertoire
	QDomElement toXml(QDomDocument *document);

	//Des fonctions pour la recherche dans l'arborescence
	Media *findMediaByLocalPath(QString localPath);
	Media *findMediaByRealPath(QString realPath);

	//Demande à l'objet d'écouter les signaux
	void setListenning(bool listen);

	//Une methode statique qui sert à supprimer un repertoire non vide
	static void removeNonEmptyDirectory(QString path);

	//Destructeur
	virtual ~Dir();

private slots:
	//Le slot qui sera appelé lorsque le repertoire a subit un changement
	void directoryChangedAction();

signals:
	void detectChangement(Media *m);

protected:
	//Constructeur
	Dir(QString localPath,QString realPath,Dir *parent,int revision,bool readOnly);

	//La liste de ses sous médias
	QVector<Media*> *subMedias;

	//Le watcher pour détecter les modifs
	QFileSystemWatcher *watcher;

	//Le boolean, pour indiquer si le Dir doit détecter ou non les changement
	bool listen;
};

#endif // DIR_H
