#ifndef DEPOT_H
#define DEPOT_H

#include "dir.h"

class Depot : public Dir
{
public:
	//Des fonctions statiques pour allouer un objet Depot
	static Depot *createDepot(QString localPath,QString realPath,int revision,bool readOnly);
	static Depot *loadDepot(QDomNode noeud);

	//Les accesseurs et mutateurs
	int getRevision();
	bool isReadOnly();
	void setRevision(int revision);
	void incRevision();
	void decRevision();
	void setReadOnly(bool readOnly);

	//Retourner le code xml
	virtual QDomElement toXml(QDomDocument *document);

private:
	Depot(QString localPath,QString realPath,int revision,bool readOnly);


	//Pour savoir si le media est en lecture seule
	bool readOnly;

	//Pour le numero de révision du média
	int revision;

};

#endif // DEPOT_H
