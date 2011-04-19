#ifndef SVNMANAGER_H
#define SVNMANAGER_H

#include<QtCore>

#define SVN_PATH "svn"
#define SVN_ADMIN_PATh "svnadmin"


class SvnManager
{
public:
	SvnManager *createSvnManager(QString svnLogin,QString svnPassword,QString svnCommand="svn",QString svnAdminCommand="svn");

private:
	SvnManager(QString svnLogin,QString svnPassword,QString svnCommand="svn",QString svnAdminCommand="svn");
	QString svnLogin;
	QString svnPassword;
	QString svnCommand;
	QString svnAdminCommand;
};

#endif // SVNMANAGER_H
