#include "depot.h"
QString Depot::GLOBAL_DEPOTS_PATH="";



Depot *Depot::loadDepot(QString depotName,SvnManager *svnManager)
{
	if(!svnManager) return NULL;
	QString globalTmpPath=Depot::GLOBAL_DEPOTS_PATH;
	if(globalTmpPath.isEmpty() || depotName.isEmpty() || !globalTmpPath.endsWith("/"))
		return NULL;

	if(!depotName.endsWith("/")) depotName=depotName+"/";

	QDir dir(globalTmpPath+depotName);
	if(dir.exists()) Depot::removeNonEmptyDirectory(globalTmpPath+depotName);

	QDir parent(globalTmpPath);
	if(!parent.mkdir(depotName)) return NULL;

	if(!(svnManager->checkoutDepot(globalTmpPath+depotName,depotName)))
	{
		Depot::removeNonEmptyDirectory(globalTmpPath+depotName);
		return NULL;
	}

	return new Depot(depotName,svnManager);
}







Depot::Depot(QString depotName,SvnManager *svnManager)
{
	this->depotName=depotName;
	this->svnManager=svnManager;
	int r=svnManager->getRevision(GLOBAL_DEPOTS_PATH+depotName);
	if(r<0) r=0;
	revision=r;
}






QString Depot::getDepotName()
{
	return depotName;
}



int Depot::getRevision()
{
	return revision;
}




bool Depot::updateFileContent(QString fileRealPath,QByteArray content,QString login,QString password)
{
	if(!fileRealPath.startsWith(depotName)) return false;
	mutex.lock();
	QString fileLocalPath=GLOBAL_DEPOTS_PATH+fileRealPath;
	QFile file(fileLocalPath);
	if(!file.open(QIODevice::WriteOnly))
	{
		mutex.unlock();
		return false;
	}
	file.write(content);
	file.close();
	if(!(svnManager->commitDepot(GLOBAL_DEPOTS_PATH+depotName,login,password)))
	{
		mutex.unlock();
		return false;
	}
	int r=svnManager->getRevision(GLOBAL_DEPOTS_PATH+depotName);
	if(r<0)
	{
		mutex.unlock();
		return false;
	}
	revision=r;
	mutex.unlock();
	return true;
}




bool Depot::createDir(QString dirRealPath,QString login,QString password)
{
	if(!dirRealPath.startsWith(depotName) || dirRealPath==depotName) return false;
	mutex.lock();

	QString dirLocalPath=GLOBAL_DEPOTS_PATH+dirRealPath;
	QString dirParentPath=extractParentPath(dirLocalPath)+"/";
	QString dirName=extractName(dirLocalPath);
	QDir dir(dirParentPath);
	if(!dir.exists() || !dir.mkdir(dirName))
	{
		mutex.unlock();
		return false;
	}
	if(!(svnManager->addFileToDepot(GLOBAL_DEPOTS_PATH+depotName,dirLocalPath,login,password)))
	{
		mutex.unlock();
		return false;
	}
	int r=svnManager->getRevision(GLOBAL_DEPOTS_PATH+depotName);
	if(r<0)
	{
		mutex.unlock();
		return false;
	}
	revision=r;
	mutex.unlock();
	return true;
}





bool Depot::createFile(QString fileRealPath,QString login,QString password)
{
	if(!fileRealPath.startsWith(depotName)) return false;
	mutex.lock();

	QString fileLocalPath=GLOBAL_DEPOTS_PATH+fileRealPath;
	QFile file(fileLocalPath);
	if(!file.open(QIODevice::WriteOnly))
	{
		mutex.unlock();
		return false;
	}
	file.write("");
	file.close();
	if(!(svnManager->addFileToDepot(GLOBAL_DEPOTS_PATH+depotName,fileLocalPath,login,password)))
	{
		mutex.unlock();
		return false;
	}
	int r=svnManager->getRevision(GLOBAL_DEPOTS_PATH+depotName);
	if(r<0)
	{
		mutex.unlock();
		return false;
	}
	revision=r;
	mutex.unlock();
	return true;
}





bool Depot::deleteMedia(QString mediaRealPath,QString login,QString password)
{
	if(!mediaRealPath.startsWith(depotName)) return false;
	mutex.lock();

	QString mediaLocalPath=GLOBAL_DEPOTS_PATH+mediaRealPath;
	QDir dir(mediaLocalPath);
	if(dir.exists())
	{
		if(!removeNonEmptyDirectory(mediaLocalPath))
		{
			mutex.unlock();
			return false;
		}
	}
	else
	{
		if(!QFile::remove(mediaLocalPath))
		{
			mutex.unlock();
			return false;
		}
	}
	if(!(svnManager->removeFileToDepot(GLOBAL_DEPOTS_PATH+depotName,mediaLocalPath,login,password)))
	{
		mutex.unlock();
		return false;
	}
	int r=svnManager->getRevision(GLOBAL_DEPOTS_PATH+depotName);
	if(r<0)
	{
		mutex.unlock();
		return false;
	}
	revision=r;
	mutex.unlock();
	return true;
}




bool Depot::isMediaExists(QString realPath)
{
	if(!realPath.startsWith(depotName)) return false;
	QString localPath=GLOBAL_DEPOTS_PATH+realPath;
	QDir dir(localPath);
	if(dir.exists()) return true;
	QFile file(localPath);
	if(file.exists()) return true;
	return false;
}






QList<Request*> Depot::getUpgradingRequest(int revision)
{
	return svnManager->getRequestDiff(GLOBAL_DEPOTS_PATH+depotName,revision,this->revision);
}



Depot::~Depot()
{
	mutex.lock();
	mutex.unlock();
}








//méthode statique qui extrait d'un chemin, son repertoire parent.
//le resultat est renvoyé sans le "/"
QString Depot::extractParentPath(QString path)
{
	QString path2=path;
	if(path2.endsWith("/")) path2=path2.left(path2.length()-1);
	QString parentPath;
	if(path2.lastIndexOf("/")>=0)
		parentPath=path2.left(path2.lastIndexOf("/"));
	return parentPath;
}




//méthode statique qui extrait d'un chemin, son nom
//si c'est un repertoire, le nom retourné contient bien un "/" final
QString Depot::extractName(QString path)
{
	QString path2=path;
	if(path2.endsWith("/")) path2=path2.left(path2.length()-1);
	QString name=path2;
	if(path2.lastIndexOf("/")>=0)
		name=path2.right(path2.length()-path2.lastIndexOf("/")-1);
	if(path.endsWith("/")) name=name+"/";
	return name;
}





//Une méthode statique qui supprime un répertoire non vide
bool Depot::removeNonEmptyDirectory(QString path)
{
	QDir dir(path);

	//Supprime tous les fichier contenus dans le dossier path
	QFileInfoList fileList = dir.entryInfoList( QDir::Files | QDir::Hidden | QDir::NoDotAndDotDot );
	foreach(QFileInfo file, fileList) dir.remove( file.absoluteFilePath());

	//Supprime tous les dossiers contenus dans le dossier path
	QFileInfoList dirList = dir.entryInfoList( QDir::AllDirs | QDir::Hidden | QDir::NoDotAndDotDot );
	foreach(QFileInfo dir, dirList) if(!removeNonEmptyDirectory(dir.absoluteFilePath())) return false;

	//On supprime enfin le dossier name
	if(!dir.rmdir(dir.absolutePath())) return false;
	return true;
}

