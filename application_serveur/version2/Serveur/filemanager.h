#ifndef FILEMANAGER_H
#define FILEMANAGER_H


#include "depot.h"
#include "svnmanager.h"

//Cette classe fait un truc pas du tout connu pcq on en a jamais parlé

class FileManager
{
public:
	static FileManager *createFileManager(QString filesPath,SvnManager *svnManager);
	bool addDepot(QString depotName);
	Depot *getDepot(QString depotName);
	Depot *depotContainer(QString mediaPath);
	bool removeDepot(QString depotName);

private:
	FileManager(SvnManager *svnManager);

	QList<Depot*> *depots;
	SvnManager *svnManager;
	QMutex mutex;
};

#endif // FILEMANAGER_H
