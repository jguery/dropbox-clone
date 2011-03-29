#include "dir.h"
#include "hddinterface.h"

//Pour créer un repertoire à partir de son localPath et de son realPath
//Dans ce cas, le repertoire doit être vide.
//Le répertoire existe déjà en réalité, cette fonction
Dir *Dir::createDir(QString localPath,QString realPath)
{
	if(localPath.isEmpty() || realPath.isEmpty()) return NULL;
	if(localPath.endsWith("/")) localPath=localPath.left(localPath.length()-1);
	if(realPath.endsWith("/")) realPath=realPath.left(realPath.length()-1);
	QDir localDir(localPath);
	if(!localDir.exists()) return NULL;
	if(localDir.entryList(QDir::AllEntries | QDir::NoDotAndDotDot).length()!=0) return NULL;
	return new Dir(localPath,realPath);
}


//Permet de charger le repertoire à partir d'un noeud xml. A noter qu'il faut que le contenu du repertoire corresponde
//exactement au contenu actuel du repertoire. C'est à dire qu'il n'y ai eu aucune modif.
Dir *Dir::loadDir(QDomNode noeud)
{
	if(noeud.toElement().tagName()!="dir") return NULL;

	QString localPath=noeud.toElement().attribute("localPath","");
	QString realPath=noeud.toElement().attribute("realPath","");

	if(localPath.isEmpty() || realPath.isEmpty()) return NULL;

	if(localPath.endsWith("/")) localPath=localPath.left(localPath.length()-1);
	if(realPath.endsWith("/")) realPath=realPath.left(realPath.length()-1);

	QDir localDir(localPath);
	if(!localDir.exists()) return NULL;
	Dir *dir=new Dir(localPath,realPath);

	QDomNodeList list=noeud.childNodes();
	QList<QString> contenu1;
	for(unsigned int i=0;i<list.length();i++)
	{
		QDomNode n=list.at(i);QDomElement e=n.toElement();
		if(e.tagName()=="dir")
		{
			Dir *d=Dir::loadDir(n);
			if(d==NULL)
			{
				delete dir;
				return NULL;
			}
			dir->getSubMedias()->append(d);
		}
		else if(e.tagName()=="file")
		{
			File *f=File::loadFile(n);
			if(f==NULL)
			{
				delete dir;
				return NULL;
			}
			dir->getSubMedias()->append(f);
		}
		contenu1.append(e.attribute("localPath",""));
	}
	QList<QString> contenu2=localDir.entryList(QDir::AllEntries | QDir::NoDotAndDotDot);
	for(int i=0;i<contenu2.length();i++)
	{
		if(!contenu1.contains(localPath+"/"+contenu2.at(i)))
		{
			delete dir;
			return NULL;
		}
	}
	return dir;
}


//Oui, c'est bien un repertoire
bool Dir::isDirectory()
{
	return true;
}


//Le constructeur fait les initialisations, puis alloue le watcher,
//et connecte son signal directoryChanged
Dir::Dir(QString localPath,QString realPath): Media(localPath,realPath)
{
	this->subMedias=new QVector<Media*>();
	watcher=new QFileSystemWatcher();
	watcher->addPath(localPath);
	QObject::connect(watcher,SIGNAL(directoryChanged(QString)),this,SLOT(directoryChangedAction(QString)));
	this->hddInterface=NULL;
}


//Ce slot est appellé à chaque fois qu'une modification est détectée dans le repertoire
void Dir::directoryChangedAction(QString path)
{
	QList<QString> contenu1;
	for(int i=0;i<subMedias->size();i++)
	{
		Media *m=subMedias->at(i);
		if(!m->isDirectory())
		{
			File *f=(File*)m;
			if(f->hasBeenRemoved()) if(hddInterface!=NULL) hddInterface->mediaHasBeenRemoved(f,this);
			else if(f->hasBeenUpdated()) if(hddInterface!=NULL) hddInterface->fileHasBeenUpdated(f,this);
		}
		else
		{
			Dir *d=(Dir*)m;
			if(d->hasBeenRemoved()) if(hddInterface!=NULL) hddInterface->mediaHasBeenRemoved(d,this);
		}
		contenu1.append(m->getLocalPath());
	}
	QDir dir(localPath);
	QList<QString> contenu2=dir.entryList(QDir::AllEntries | QDir::NoDotAndDotDot);
	for(int i=0;i<contenu2.length();i++)
	{
		QString p=localPath+"/"+contenu2.at(i);
		if(!contenu1.contains(p))
		{
			Media *m;QDir d(p);
			if(d.exists()) {m=Dir::createDir(p,realPath+"/"+contenu2.at(i));QMessageBox::information(NULL,"","repertoire créé");}
			else m=File::createFile(p,realPath+"/"+contenu2.at(i));
			if(m==NULL) continue;
			if(hddInterface!=NULL) hddInterface->mediaHasBeenCreated(m,this);
		}
	}
}


//Détecte si le repertoire a été supprimé
bool Dir::hasBeenRemoved()
{
	QDir dir(localPath);
	if(!dir.exists()) return true;
	return false;
}

//Retourne la liste des sous médias du repertoire
QVector<Media*> *Dir::getSubMedias()
{
	return subMedias;
}


//Retourne le code xml du repertoire et de ses sous medias
QDomElement Dir::toXml(QDomDocument *document)
{
	QDomElement element=document->createElement("dir");
	element.setAttribute("localPath",localPath);
	element.setAttribute("realPath",realPath);
	for(int i=0;i<subMedias->size();i++)
	{
		QDomElement e=subMedias->at(i)->toXml(document);
		element.appendChild(e);
	}
	return element;
}


//Recherche à quel média correspond ce localPath
Media *Dir::findMediaByLocalPath(QString localPath)
{
	if(localPath.endsWith("/")) localPath=localPath.left(localPath.length()-1);
	if(this->localPath==localPath) return this;
	for(int i=0;i<subMedias->size();i++)
	{
		Media *find=subMedias->at(i)->findMediaByLocalPath(localPath);
		if(find!=NULL) return find;
	}
	return NULL;
}


//Recherche à quel média correspond ce realPath
Media *Dir::findMediaByRealPath(QString realPath)
{
	if(realPath.endsWith("/")) realPath=realPath.left(realPath.length()-1);
	if(this->realPath==realPath) return this;
	for(int i=0;i<subMedias->size();i++)
	{
		Media *find=subMedias->at(i)->findMediaByRealPath(realPath);
		if(find!=NULL) return find;
	}
	return NULL;
}


//Recherche quel est le repertoire parent du média auquel correspond ce localPath
Dir *Dir::findMediaParentByLocalPath(QString localPath)
{
	if(localPath.endsWith("/")) localPath=localPath.left(localPath.length()-1);
	for(int i=0;i<subMedias->size();i++)
	{
		if(subMedias->at(i)->getLocalPath()==localPath) return this;
	}
	for(int i=0;i<subMedias->size();i++)
	{
		if(!subMedias->at(i)->isDirectory()) continue;
		Dir *find=((Dir*)subMedias->at(i))->findMediaParentByLocalPath(localPath);
		if(find!=NULL) return find;
	}
	return NULL;
}


//Recherche quel est le repertoire parent du média auquel correspond ce realPath
Dir *Dir::findMediaParentByRealPath(QString realPath)
{
	if(realPath.endsWith("/")) realPath=realPath.left(realPath.length()-1);
	for(int i=0;i<subMedias->size();i++)
	{
		if(subMedias->at(i)->getRealPath()==realPath) return this;
	}
	for(int i=0;i<subMedias->size();i++)
	{
		if(!subMedias->at(i)->isDirectory()) continue;
		Dir *find=((Dir*)subMedias->at(i))->findMediaParentByRealPath(realPath);
		if(find!=NULL) return find;
	}
	return NULL;
}


//Permet de donner l'objet à prevenir en cas de modif
void Dir::setSignalListener(HddInterface *hddInterface)
{
	this->hddInterface=hddInterface;
	for(int i=0;i<subMedias->size();i++)
	{
		if(!subMedias->at(i)->isDirectory()) continue;
		Dir *d=(Dir*)subMedias->at(i);
		d->setSignalListener(hddInterface);
	}
}


//Le destructeur qui détruit tout ce qu'il trouve sur son passage :p
Dir::~Dir()
{
	for(int i=subMedias->size()-1;i>=0;i--)
	{
		delete subMedias->at(i);
	}
	subMedias->clear();
	delete subMedias;
	delete watcher;
}

