#ifndef MEDIA_H
#define MEDIA_H

#include<QtCore/QObject>
#include<QtCore/QString>
#include<QtXml/QDomDocument>
#include<QtXml/QDomElement>



/*
    Cette classe représente un média sur le disque dur.
Cela peut être un repertoire ou un simple fichier
Elle justement est héritée par les classes Dir et File
C'est une classe abstraite; elle possède des méthodes virtuelles
qui seront implémentées par ses classes filles.
NB: Un média est réprésenté par son localPath qui est son chemin sur le disque de l'user
    et son realPath qui est son chemin sur le svn du serveur. L'attribut readOnly sert à signaler si l'utilisateur a le droit de modifier le média
    L'attribut revision sert à stocker le numero de révision du fichier sur le serveur
*/



class Media: public QObject
{
	Q_OBJECT
// le constructeur et les attributs sont protected pour être accéssible depuis les classes filles
protected:
	Media(QString localPath,QString realPath,int revision,bool readOnly);

	//Les attributs
	QString localPath;
	QString realPath;

	//Pour savoir si le media est en lecture seule
	bool readOnly;

	//Pour le numero de révision du média
	int revision;

public:
	// la méthode virtuelle isDirectory pour savoir si c'est un repertoire ou un simple fichier
	virtual bool isDirectory()=0;

	// d'autres méthodes virtuelles qui seront utiles pour plus tard.
	virtual QDomElement toXml(QDomDocument *document)=0;
	virtual Media *findMediaByLocalPath(QString localPath)=0;
	virtual Media *findMediaByRealPath(QString realPath)=0;

	//les accesseurs pour que les attributs soient visibles depuis l'extérieur
	QString getLocalPath();
	QString getRealPath();
	int getRevision();
	bool isReadOnly();

	//Les mutateurs
	void setLocalPath(QString localPath);
	void setRealPath(QString realPath);
	void setRevision(int revision);
	void incRevision();
	void setReadOnly(bool readOnly);

	//Quelques méthodes statiques pour la gestion des fichiers et repertoires en général
	static QString extractParentPath(QString path);
	static QString extractName(QString path);
};

#endif // MEDIA_H
