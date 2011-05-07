#ifndef DEPOT_H
#define DEPOT_H

#include "dir.h"




/*
  Cette classe représente un dépot synchronisé.
  Elle hérite de la classe Dir. Un Depot possède en plus, un numéro de révision
  et un attribut pour savoir s'il est accéssible en écriture.
*/



class Depot : public Dir
{
public:
	//Des fonctions statiques pour allouer un objet Depot
	static Depot *createDepot(QString localPath,QString realPath,int revision,bool readOnly);
	static Depot *loadDepot(QDomNode noeud);

	//Les accesseurs
	int getRevision();
	bool isReadOnly();

	//Les mutateurs
	void setRevision(int revision);
	void setReadOnly(bool readOnly);

	//Retourner le code xml
	virtual QDomElement toXml(QDomDocument *document);

private:
	Depot(QString localPath,QString realPath,int revision,bool readOnly);

	//Pour savoir si le media est en lecture seule
	bool readOnly;

	//Pour le numero de révision du dépôt
	int revision;

};

#endif // DEPOT_H
