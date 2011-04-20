#ifndef SVNMANAGER_H
#define SVNMANAGER_H

#include<QtCore>


class SvnManager
{
public:
	static SvnManager *createSvnManager(QString svnLogin,QString svnPassword,QString svnCommand="svn",QString svnAdminCommand="svn");
	bool checkoutDepot(QString localPath,QString depotName);
	bool updateDepot(QString depotPath);
	bool commitDepot(QString depotPath,QString login,QString password);
	bool addFileToDepot(QString depotPath,QString filePath,QString login,QString password);
	bool removeFileToDepot(QString depotPath,QString filePath,QString login,QString password);
	int getRevision(QString depotPath);

private:
	SvnManager(QString svnLogin,QString svnPassword,QString svnCommand="svn",QString svnAdminCommand="svn");
	QString svnServerPath;
	QString svnLogin;
	QString svnPassword;
	QString svnCommand;
	QString svnAdminCommand;
};

#endif // SVNMANAGER_H
