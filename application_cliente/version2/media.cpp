#include "media.h"

//le constructeur pour initialiser les attributs

Media::Media(QString localPath,QString realPath)
{
	this->localPath=localPath;
	this->realPath=realPath;
}

//les accesseurs d'accès et de modifications

QString Media::getLocalPath()
{
	return localPath;
}

void Media::setLocalPath(QString localPath)
{
	this->localPath=localPath;
}

//les accesseurs d'accès et de modifications

QString Media::getRealPath()
{
	return realPath;
}

void Media::setRealPath(QString realPath)
{
	this->realPath=realPath;
}

