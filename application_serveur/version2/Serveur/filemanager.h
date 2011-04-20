#ifndef FILEMANAGER_H
#define FILEMANAGER_H


#include "depot.h"
#include "svnmanager.h"


class FileManager
{
public:
	static FileManager *createFileManager(SvnManager *svnManager);
	bool addDepot(QString depotName);
	Depot *getDepot(QString depotName);
	bool removeDepot(QString depotName);

private:
	FileManager(SvnManager *svnManager);

	QList<Depot*> *depots;
	SvnManager *svnManager;
	QMutex listDepotsMutex;
};

#endif // FILEMANAGER_H
