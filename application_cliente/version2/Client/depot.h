#ifndef DEPOT_H
#define DEPOT_H

#include "dir.h"

class Depot : public Dir
{
public:
	//Des fonctions statiques pour allouer un objet Depot
	static Depot *createDepot(QString localPath,QString realPath,int revision,bool readOnly);
	static Depot *loadDepot(QDomNode noeud);

private:
	Depot(QString localPath,QString realPath,int revision,bool readOnly);
};

#endif // DEPOT_H
