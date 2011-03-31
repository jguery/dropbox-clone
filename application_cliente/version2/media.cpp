#include "media.h"

//le constructeur pour initialiser les attributs

Media::Media(QString localPath,QString realPath,int revision,bool readOnly)
{
	this->localPath=localPath;
	this->realPath=realPath;
	this->revision=revision;
	this->readOnly=readOnly;
}



//les accesseurs d'accès et de modifications au localPath

QString Media::getLocalPath()
{
	return localPath;
}

void Media::setLocalPath(QString localPath)
{
	this->localPath=localPath;
}




//les accesseurs d'accès et de modifications au realPath

QString Media::getRealPath()
{
	return realPath;
}

void Media::setRealPath(QString realPath)
{
	this->realPath=realPath;
}





//les accesseurs d'accès et de modifications à la révision

int Media::getRevision()
{
	return revision;
}

void Media::setRevision(int revision)
{
	this->revision=revision;
}

void Media::incRevision()
{
	this->revision++;
}





//les accesseurs d'accès et de modifications au readOnly

bool Media::isReadOnly()
{
	return readOnly;
}

void Media::setReadOnly(bool readOnly)
{
	this->readOnly=readOnly;
}





//méthode statique qui extrait d'un chemin, son repertoire parent.
QString Media::extractParentPath(QString path)
{
	QString path2=path;
	if(path2.endsWith("/")) path2=path2.left(path2.length()-1);
	QString parentPath=path2.left(path2.lastIndexOf("/"));
	if(path.endsWith("/")) parentPath=parentPath+"/";
	return parentPath;
}





//méthode statique qui extrait d'un chemin, son nom
QString Media::extractName(QString path)
{
	QString path2=path;
	if(path2.endsWith("/")) path2=path2.left(path2.length()-1);
	QString name=path2.right(path2.length()-path2.lastIndexOf("/")-1);
	if(path.endsWith("/")) name=name+"/";
	return name;
}

