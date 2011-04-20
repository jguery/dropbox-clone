#include "svnmanager.h"



SvnManager *SvnManager::createSvnManager(QString svnLogin, QString svnPassword, QString svnCommand, QString svnAdminCommand)
{
	if(svnLogin.isEmpty() || svnPassword.isEmpty() || svnCommand.isEmpty() || svnAdminCommand.isEmpty())
		return NULL;
	QProcess p;
	p.start(svnCommand);
	p.waitForFinished();
	if(p.error()==QProcess::FailedToStart) return NULL;
	p.start(svnAdminCommand);
	p.waitForFinished();
	if(p.error()==QProcess::FailedToStart) return NULL;
	return new SvnManager(svnLogin,svnPassword,svnCommand,svnAdminCommand);
}




SvnManager::SvnManager(QString svnLogin, QString svnPassword, QString svnCommand, QString svnAdminCommand)
{
	this->svnServerPath="svn://pchky.maisel.enst-bretagne.fr/";
	this->svnLogin=svnLogin;
	this->svnPassword=svnPassword;
	this->svnCommand=svnCommand;
	this->svnAdminCommand=svnAdminCommand;
}




bool SvnManager::checkoutDepot(QString localPath,QString depotName)
{
	QDir dir(localPath);
	if(!dir.mkdir(depotName)) return false;
	QString cmd=svnCommand+" checkout --username "+svnLogin+" --password "+svnPassword+" "+svnServerPath+depotName+" "+localPath+depotName;
	int rep=QProcess::execute(cmd);
	return (rep==0);
}





bool SvnManager::updateDepot(QString depotPath)
{
	QString cmd=svnCommand+" update --username "+svnLogin+" --password "+svnPassword+" "+depotPath;
	int rep=QProcess::execute(cmd);
	return (rep==0);
}




bool SvnManager::commitDepot(QString depotPath,QString login,QString password)
{
	QString cmd=svnCommand+" commit -m " " --username "+login+" --password "+password+" "+depotPath;
	int rep=QProcess::execute(cmd);
	return (rep==0);
}




bool SvnManager::addFileToDepot(QString depotPath,QString filePath,QString login,QString password)
{
	QString cmd=svnCommand+" add "+filePath;
	int rep=QProcess::execute(cmd);
	if(rep!=0) return false;
	return commitDepot(depotPath,login,password);
}



bool SvnManager::removeFileToDepot(QString depotPath,QString filePath,QString login,QString password)
{
	QString cmd=svnCommand+" rm "+filePath;
	int rep=QProcess::execute(cmd);
	if(rep!=0) return false;
	return commitDepot(depotPath,login,password);
}


int SvnManager::getRevision(QString depotPath)
{
	QString cmd=svnCommand+" log "+depotPath;
	QProcess p;
	p.start(cmd);
	p.waitForFinished();
	if(p.exitCode()!=0) return -1;
	QByteArray rep=p.readAll();
	QRegExp rx("(\\d+)");
	int index=0;
	if(index=rx.indexIn(rep,0)==-1) return -1;
	return rx.cap(1).toInt();
}


