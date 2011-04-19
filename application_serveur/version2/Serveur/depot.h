#ifndef DEPOT_H
#define DEPOT_H

#include<QtCore>
#include<QtGui>


class Depot
{
public:
	static Depot *loadDepot(QString globalTmpPath,QString depotName);

	static QString extractParentPath(QString path);
	static QString extractName(QString path);
	static bool removeNonEmptyDirectory(QString path);

	//Depot(QString svnPath,QString login, QString password,QString depotName,QString localPath);
	Depot(QString localPath,QString realPath);
	void lock();
	void unlock();

	bool updateFileContent(QString fileRealPath,QByteArray content);
	bool createDir(QString dirRealPath);
	bool createFile(QString fileRealPath);
	bool deleteMedia(QString mediaRealPath);

private:
	QString localPath;
	QString realPath;
	QMutex mutex;
	int revision;
};

#endif // DEPOT_H
