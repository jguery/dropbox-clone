#ifndef MEDIA_H
#define MEDIA_H

#include<QtCore/QObject>
#include<QtCore/QString>
#include<QtCore/QMutex>
#include<QtXml/QDomDocument>
#include<QtXml/QDomElement>

class Dir;


//Enumeration des différents états de detection d'un state
//Creation, Suppression, et Mise à jour
//Le default state sert juste à donner une valeur par défaut à la structure
enum State
{
	MediaIsCreating,
	MediaIsRemoving,
	MediaIsUpdating,
	MediaDefaultState
};




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
	Media(QString localPath,QString realPath,Dir *parent,int revision,bool readOnly);

	//Les attributs
	QString localPath;
	QString realPath;

	//Le parent du média
	Dir *parent;

	//Pour savoir si le media est en lecture seule
	bool readOnly;

	//Pour le numero de révision du média
	int revision;

	//Les détections qui sont en cours sur le media
	QList<State> *detectionState;

        //Un mutex pour les threads synchronisés
        QMutex mutex;

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
	Dir *getParent();
	int getRevision();
	bool isReadOnly();
	QList<State> *getDetectionState();

	//Les mutateurs
	void setLocalPath(QString localPath);
	void setRealPath(QString realPath);
	void setRevision(int revision);
	void setParent(Dir *parent);
	void incRevision();
	void decRevision();
	void setReadOnly(bool readOnly);

        //Pour réserver et libérer l'objet
        void lock();
        void unlock();

	//Quelques méthodes statiques pour la gestion des fichiers et repertoires en général
	static QString extractParentPath(QString path);
	static QString extractName(QString path);
	static State stateFromString(QString stateString);
	static QString stateToString(State state);

	//destructeur
	virtual ~Media();
};



#endif // MEDIA_H
