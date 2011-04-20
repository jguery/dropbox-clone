#ifndef DEPOT_H
#define DEPOT_H

#include<QtCore>
#include<QtGui>
#include "svnmanager.h"




class Depot
{
public:
	static QString GLOBAL_DEPOTS_PATH;
	static Depot *createDepot(QString depotName,SvnManager *svnManager);
	static Depot *loadDepot(QString depotName,SvnManager *svnManager);

	QString getDepotName();

	static QString extractParentPath(QString path);
	static QString extractName(QString path);
	static bool removeNonEmptyDirectory(QString path);

	void lock();
	void unlock();

	bool updateFileContent(QString fileRealPath,QByteArray content,QString login,QString password);
	bool createDir(QString dirRealPath,QString login,QString password);
	bool createFile(QString fileRealPath,QString login,QString password);
	bool deleteMedia(QString mediaRealPath,QString login,QString password);

private:
	Depot(QString depotName,SvnManager *svnManager);
	QString depotName;
	SvnManager *svnManager;
	QMutex mutex;
	int revision;
};

#endif // DEPOT_H