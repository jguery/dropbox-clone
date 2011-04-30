#include "svnmanager.h"
#include "depot.h"




SvnManager *SvnManager::createSvnManager(QString svnServerPath,QString svnLogin, QString svnPassword, QString svnCommand, QString svnAdminCommand)
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
	return new SvnManager(svnServerPath,svnLogin,svnPassword,svnCommand,svnAdminCommand);
}




SvnManager::SvnManager(QString svnServerPath,QString svnLogin, QString svnPassword, QString svnCommand, QString svnAdminCommand)
{
	svnLogin=normalizePath(svnLogin);
	svnPassword=normalizePath(svnPassword);
	this->svnServerPath=svnServerPath;
	this->svnLogin=svnLogin;
	this->svnPassword=svnPassword;
	this->svnCommand=svnCommand;
	this->svnAdminCommand=svnAdminCommand;
}




bool SvnManager::checkoutDepot(QString localPath,QString depotName)
{
	localPath=normalizePath(localPath);
	depotName=normalizePath(depotName);
	QString cmd=svnCommand+" checkout --username "+svnLogin+" --password "+svnPassword+" "+svnServerPath+depotName+" "+localPath;
	int rep=QProcess::execute(cmd);
	return (rep==0);
}





bool SvnManager::updateDepot(QString depotPath)
{
	depotPath=normalizePath(depotPath);
	QString cmd=svnCommand+" update --username "+svnLogin+" --password "+svnPassword+" "+depotPath;
	int rep=QProcess::execute(cmd);
	return (rep==0);
}




bool SvnManager::commitDepot(QString depotPath,QString login,QString password)
{
	depotPath=normalizePath(depotPath);
	login=normalizePath(login);
	password=normalizePath(password);
	QString cmd=svnCommand+" commit -m \"dropboxCommit\" --username "+login+" --password "+password+" "+depotPath;
	int rep=QProcess::execute(cmd);
	return (rep==0);
}




bool SvnManager::addFileToDepot(QString depotPath,QString filePath,QString login,QString password)
{
	depotPath=normalizePath(depotPath);
	filePath=normalizePath(filePath);
	login=normalizePath(login);
	password=normalizePath(password);
	QString cmd=svnCommand+" add "+filePath;
	int rep=QProcess::execute(cmd);
	if(rep!=0) return false;
	return commitDepot(depotPath,login,password);
}



bool SvnManager::removeFileToDepot(QString depotPath,QString filePath,QString login,QString password)
{
	depotPath=normalizePath(depotPath);
	filePath=normalizePath(filePath);
	login=normalizePath(login);
	password=normalizePath(password);
	QString cmd=svnCommand+" rm "+filePath;
	int rep=QProcess::execute(cmd);
	if(rep!=0) return false;
	return commitDepot(depotPath,login,password);
}




int SvnManager::getRevision(QString depotPath)
{
	depotPath=normalizePath(depotPath);
	QString cmd1=svnCommand+" update "+depotPath;
	QProcess p1;
	p1.start(cmd1);
	p1.waitForFinished();
	if(p1.exitCode()!=0) return -1;
	QString cmd=svnCommand+" log "+depotPath;
	QProcess p;
	p.start(cmd);
	p.waitForFinished();
	if(p.exitCode()!=0) return -1;
	QByteArray rep=p.readAll();
	QRegExp rx("(\\d+)");
	if(rx.indexIn(rep,0)==-1) return -1;
	return rx.cap(1).toInt();
}







QList<Request*> SvnManager::getRequestDiff(QString depotPath,int clientRevision,int svnRevision)
{
	if(clientRevision<0 || svnRevision<0) return QList<Request*>();
	if(clientRevision>=svnRevision) return QList<Request*>();
	depotPath=normalizePath(depotPath);
	QString cmd=svnCommand+" diff --summarize --xml --depth infinity --no-diff-deleted ";
	cmd+=" -r "+QString::number(clientRevision)+":"+QString::number(svnRevision);
	cmd+=" "+depotPath;
	QProcess p;
	p.start(cmd);
	p.waitForFinished();
	if(p.exitCode()!=0) return QList<Request*>();
	QByteArray response=p.readAll();

	QDomDocument document;
	if(!document.setContent(response)) return QList<Request*>();

	//On charge la liste des élèments fils du document
	QDomNodeList noeuds=document.documentElement().childNodes();

	QList<Request*> list;

	depotPath=Depot::GLOBAL_DEPOTS_PATH;

	for(int i=0;i<noeuds.length();i++)
	{
		QDomNode noeud=noeuds.at(i);
		QDomElement element=noeud.toElement();
		if(element.isNull()) continue;
		if(element.tagName()=="paths")
		{
			QDomNodeList noeuds=noeud.childNodes();
			for(int k=noeuds.length()-1;k>=0;k--)
			{
				QDomNode noeud=noeuds.at(k);
				QDomElement element=noeud.toElement();
				if(element.isNull()) continue;
				if(element.tagName()!="path") continue;
				if(!noeud.firstChild().isText()) continue;
				QString type=element.attribute("item");
				QString path=noeud.firstChild().toText().data();
				bool isDirectory=(element.attribute("kind")=="dir");

				if(type=="added")
				{
					Request *r=new Request();
					r->setType(CREATE_FILE_INFO);
					QString realPath=path.right(path.length()-depotPath.length());
					if(realPath.endsWith("/")) realPath=realPath.left(realPath.length()-1);
					r->getParameters()->insert("realPath",realPath.toAscii());
					r->getParameters()->insert("isDirectory",isDirectory?"true":"false");
					r->getParameters()->insert("revision",QByteArray::number(svnRevision));
					list.append(r);
					if(!isDirectory) type="modified";
				}
				if(type=="modified")
				{
					Request *r=new Request();
					r->setType(UPDATE_FILE_INFO);
					QString realPath=path.right(path.length()-depotPath.length());
					if(realPath.endsWith("/")) realPath=realPath.left(realPath.length()-1);
					r->getParameters()->insert("realPath",realPath.toAscii());
					r->getParameters()->insert("revision",QByteArray::number(svnRevision));
					QByteArray content;
					QFile file(path);
					if(file.open(QIODevice::ReadOnly)) {content=file.readAll();file.close();}
					r->getParameters()->insert("content",content);
					list.append(r);
				}
				if(type=="deleted")
				{
					Request *r=new Request();
					r->setType(REMOVE_FILE_INFO);
					QString realPath=path.right(path.length()-depotPath.length());
					if(realPath.endsWith("/")) realPath=realPath.left(realPath.length()-1);
					r->getParameters()->insert("realPath",realPath.toAscii());
					r->getParameters()->insert("revision",QByteArray::number(svnRevision));
					list.append(r);
				}
			}
		}
	}
	return list;
}




QString SvnManager::normalizePath(QString path)
{
	if(path.contains(" "))
	{
		if(!path.startsWith("\"") || !path.endsWith("\""))
			path="\""+path+"\"";
	}
	return path;
}
