#include "media.h"




//le constructeur pour initialiser les attributs
Media::Media(QString localPath,QString realPath,Dir *parent,int revision,bool readOnly): QObject((QObject*)parent)
{
	this->localPath=localPath;
	this->realPath=realPath;
	this->parent=parent;
	this->revision=revision;
	this->readOnly=readOnly;
	this->detectionState=new QList<State>();
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




//les accesseurs d'accès et de modifications au parent

Dir *Media::getParent()
{
	return parent;
}

void Media::setParent(Dir *parent)
{
	this->parent=parent;
}



//les accesseurs d'accès à l'attribut state

QList<State> *Media::getDetectionState()
{
	return detectionState;
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

void Media::decRevision()
{
	this->revision--;
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



//Méthode qui convertit une QString en une vrai state
State Media::stateFromString(QString stateString)
{
	State state;
	if(stateString=="MediaIsCreating") state=MediaIsCreating;
	else if(stateString=="MediaIsUpdating") state=MediaIsUpdating;
	else if(stateString=="MediaIsRemoving") state=MediaIsRemoving;
	else state=MediaNormalState;
	return state;
}



//Méthode qui convertit une state en une QString
QString Media::stateToString(State state)
{
	QString stateString;
	if(state==MediaIsCreating) stateString="MediaIsCreating";
	else if(state==MediaIsUpdating) stateString="MediaIsUpdating";
	else if(state==MediaIsRemoving) stateString="MediaIsRemoving";
	else stateString="MediaNormalState";
	return stateString;
}



//destructeur
Media::~Media()
{
	delete detectionState;
}
