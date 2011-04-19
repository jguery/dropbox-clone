#include "depot.h"


Depot *Depot::createDepot(QString globalTmpPath,QString depotName)
{
	if(globalTmpPath.isEmpty() || depotName.isEmpty())
		return NULL;
	if(!globalTmpPath.endsWith("/")) globalTmpPath=globalTmpPath+"/";
	if(!depotName.endsWith("/")) depotName=depotName+"/";

	QDir dir(globalTmpPath);
	if(!dir.exists()) return NULL;

	return new Depot(globalTmpPath+depotName,depotName);
}


Depot::Depot(QString localPath, QString realPath)
{
	this->localPath=localPath;
	this->realPath=realPath;
	this->revision=0;
}




bool Depot::updateFileContent(QString fileRealPath,QByteArray content)
{
	if(!fileRealPath.startsWith(realPath)) return false;
	QString name=fileRealPath.right(fileRealPath.length()-realPath.length());
	QString fileLocalPath=localPath+name;
	QFile file(fileLocalPath);
	if(!file.open(QIODevice::WriteOnly))
		return false;
	file.write(content);
	file.close();
	revision++;
	return true;
}



bool Depot::createDir(QString dirRealPath)
{
	QString dirParentRealPath=extractParentPath(dirRealPath)+"/";
	QString dirRealName=extractName(dirRealPath);
	if(!dirParentRealPath.startsWith(this->realPath)) return false;
	QString dirParentName=dirParentRealPath.right(dirParentRealPath.length()-realPath.length());
	QString dirParentLocalPath=localPath+dirParentName;
	QDir dir(dirParentLocalPath);
	if(!dir.exists()) return false;
	if(!dir.mkdir(dirRealName)) return false;
	revision++;
	return true;
}


bool Depot::createFile(QString fileRealPath)
{
	QString fileParentRealPath=extractParentPath(fileRealPath)+"/";
	QString fileRealName=extractName(fileRealPath);
	if(!fileParentRealPath.startsWith(this->realPath)) return false;
	QString fileParentName=fileParentRealPath.right(fileParentRealPath.length()-realPath.length());
	QString fileParentLocalPath=localPath+fileParentName;
	QDir dir(fileParentLocalPath);
	if(!dir.exists()) return false;
	QFile file(fileParentLocalPath+fileRealName);
	if(!file.open(QIODevice::WriteOnly)) return false;
	file.close();
	revision++;
	return true;
}





bool Depot::deleteMedia(QString mediaRealPath)
{
	if(!mediaRealPath.startsWith(this->realPath)) return false;
	QDir dir(mediaRealPath);
	if(dir.exists())
	{
		if(!removeNonEmptyDirectory(mediaRealPath)) return false;
	}
	else
	{
		if(!QFile::remove(mediaRealPath)) return false;
	}
	revision++;
	return true;
}


void Depot::lock()
{
	mutex.lock();
}

void Depot::unlock()
{
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
	QFileInfoList fileList = dir.entryInfoList( QDir::Files | QDir::Hidden );
	foreach(QFileInfo file, fileList) if(!dir.remove( file.absoluteFilePath())) return false;

	//Supprime tous les dossiers contenus dans le dossier path
	QFileInfoList dirList = dir.entryInfoList( QDir::AllDirs | QDir::Hidden | QDir::NoDotAndDotDot );
	foreach(QFileInfo dir, dirList) if(!removeNonEmptyDirectory(dir.absoluteFilePath())) return false;

	//On supprime enfin le dossier name
	if(!dir.rmdir(dir.absolutePath())) return false;
	return true;
}




