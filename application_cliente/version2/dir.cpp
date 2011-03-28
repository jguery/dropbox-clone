#include "dir.h"
#include "hddinterface.h"

//Pour créer un repertoire à partir de son localPath et de son realPath
//Dans ce cas, le repertoire doit être vide.
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


//Permet de charger le repertoire à partir d'un noeud xml.
//A noter qu'il faut que le contenu du repertoire tel qu'il est décrit dans le noeud corresponde
//exactement au contenu réel du repertoire sur le disuqee. C'est à dire qu'il n'y ai eu aucune modif
//quand l'application n'était pas en marche
Dir *Dir::loadDir(QDomNode noeud)
{
        if(noeud.toElement().tagName()!="dir")
            return NULL;

	QString localPath=noeud.toElement().attribute("localPath","");
	QString realPath=noeud.toElement().attribute("realPath","");
        if(localPath.isEmpty() || realPath.isEmpty())
            return NULL;

	if(localPath.endsWith("/")) localPath=localPath.left(localPath.length()-1);
	if(realPath.endsWith("/")) realPath=realPath.left(realPath.length()-1);

        //Vérifie que le répertoire existe effectivement en réalité
	QDir localDir(localPath);
        if(!localDir.exists())
            return NULL;

        //On stocke notre répertoire(ou dépot) dans un objet Dir qui contiendra
        //tous les sous répertoires de ce répertoire, sotckés dans "subMedias"
	Dir *dir=new Dir(localPath,realPath);

        //Parcours tous les sous élément de notre dépot
        QDomNodeList list=noeud.childNodes();
        QList<QString> contenu1;
        for(unsigned int i=0;i<list.length();i++)
	{
                QDomNode n=list.at(i); QDomElement e=n.toElement();
                if(e.tagName()=="dir")          //Le sous-élément est un répertoire
		{
			Dir *d=Dir::loadDir(n);
			if(d==NULL)
			{
				delete dir;
				return NULL;
			}
                        //On ajoute ce répertoire à la liste des subMedias de notre répertoire
			dir->getSubMedias()->append(d);
		}
                else if(e.tagName()=="file")    //Le sous-élément est un fichier
		{
			File *f=File::loadFile(n);
			if(f==NULL)
			{
				delete dir;
				return NULL;
			}
                        //On ajoute ce fichier à la liste des subMedias de notre répertoire
			dir->getSubMedias()->append(f);
		}
                //On ajoute à la liste de strings contenu1 le chemin local du Media qu'on vient de traiter
		contenu1.append(e.attribute("localPath",""));
	}

        //On charge dans la liste contenu2 tous les médias contenus dans localDir
        //On vérifie que les médias qu'on a chargé (et donc présents dans le xml de config)
        //sont bien présents dans la réalité
	QList<QString> contenu2=localDir.entryList(QDir::AllEntries | QDir::NoDotAndDotDot);
        QList<QString>::iterator i;
        for(i=contenu2.begin(); i!=contenu2.end(); i++)
	{
                if(!contenu1.contains(localPath+"/"+*i))
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


//Le constructeur fait les initialisations, puis alloue le watcher, et connecte son signal directoryChanged
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
        //On parcourt tous les subMedias pour voir si la modification les concerne
	for(int i=0;i<subMedias->size();i++)
	{
		Media *m=subMedias->at(i);
                if(!m->isDirectory())   //Si le Media est un fichier
		{
                        File *f=(File*)m;   //On le stocke dans un objet File

                        //Le fichier a été supprimé
                        if(f->hasBeenRemoved() && hddInterface!=NULL)
			{
                                //On l'enlève de la liste des subMedias et on prévient
                                //le module d'interface DD (qui assurera la comm sur le réseau)
                                subMedias->remove(i);
				hddInterface->mediaHasBeenRemoved(f);
				i=i-1;
				continue;
			}
                        //Le fichier a été modifié
			else if(f->hasBeenUpdated() && hddInterface!=NULL)
			{
                                hddInterface->fileHasBeenUpdated(f); //Prévient l'interface DD
                                f->updateContent(); //Change son hash
			}
		}

                else                    //Si le Media est un répertoire
		{
                        Dir *d=(Dir*)m; //On le stocke dans un objet Dir

                        //Le répertoire a été supprimé
			if(d->hasBeenRemoved() && hddInterface!=NULL)
			{
                                subMedias->remove(i);   //On le vire des subMedias
                                hddInterface->mediaHasBeenRemoved(d);   //On prévient l'interace DD
				i=i-1;
				continue;
			}
		}
                //Si on a pas fait de remove, ajoute le chemin du media à une liste
		contenu1.append(m->getLocalPath());
	}

        //On va parcourir les fichiers et rép contenus physiquement dans le dossier
        //pour voir si la modification concerne la création d'un fichier ou d'un dossier
	QDir dir(localPath);
	QList<QString> contenu2=dir.entryList(QDir::AllEntries | QDir::NoDotAndDotDot);
        QList<QString>::iterator i;
        for(i=contenu2.begin(); i!=contenu2.end(); i++)
	{
                QString p=localPath+"/"+*i; //Chemin d'un média dans notre répertoire

                //Si ce média c'est pas dans notre liste subMedias
                if(!contenu1.contains(p))
		{
                        Media *m;

                        //Un répertoire a été créé
                        //On l'ajoute à la synchronisation (avec Dir::createDir)
                        QDir d(p);
                        if(d.exists()) m=Dir::createDir(p,realPath+"/"+*i);

                        //Sinon, c'est un fichier qui a été créé
                        //on l'ajoute aussi à la synchronisation
                        else m=File::createFile(p,realPath+"/"+*i);

                        if(m==NULL) continue; //Une erreur s'est produite

                        //On informe l'interface DD et on ajoute le media à la liste des subMedias
                        if(hddInterface!=NULL)
			{
				hddInterface->mediaHasBeenCreated(m);
				subMedias->append(m);
				contenu1.append(p);
			}
		}
	}
}


//Détecte si le repertoire a été supprimé
bool Dir::hasBeenRemoved()
{
	QDir dir(localPath);
        if(!dir.exists())
            return true;
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
//Fonction récursive qui agit sur tous les submedias
Media *Dir::findMediaByLocalPath(QString localPath)
{
	if(localPath.endsWith("/")) localPath=localPath.left(localPath.length()-1);
        if(this->localPath==localPath)
            return this;

	for(int i=0;i<subMedias->size();i++)
	{
		Media *find=subMedias->at(i)->findMediaByLocalPath(localPath);
                if(find!=NULL)
                    return find;
	}
	return NULL;
}


//Recherche à quel média correspond ce realPath
//Fonction récursive qui agit sur tous les submedias
Media *Dir::findMediaByRealPath(QString realPath)
{
	if(realPath.endsWith("/")) realPath=realPath.left(realPath.length()-1);
        if(this->realPath==realPath)
            return this;

	for(int i=0;i<subMedias->size();i++)
	{
		Media *find=subMedias->at(i)->findMediaByRealPath(realPath);
                if(find!=NULL)
                    return find;
	}
	return NULL;
}


//Recherche quel est le repertoire parent du média auquel correspond ce localPath
Dir *Dir::findMediaParentByLocalPath(QString localPath)
{
	if(localPath.endsWith("/")) localPath=localPath.left(localPath.length()-1);
	for(int i=0;i<subMedias->size();i++)
	{
                if(subMedias->at(i)->getLocalPath()==localPath)
                    return this;
	}

	for(int i=0;i<subMedias->size();i++)
	{
                if(!subMedias->at(i)->isDirectory()) continue;
		Dir *find=((Dir*)subMedias->at(i))->findMediaParentByLocalPath(localPath);
                if(find!=NULL)
                    return find;
	}
	return NULL;
}


//Recherche quel est le repertoire parent du média auquel correspond ce realPath
Dir *Dir::findMediaParentByRealPath(QString realPath)
{
	if(realPath.endsWith("/")) realPath=realPath.left(realPath.length()-1);
	for(int i=0;i<subMedias->size();i++)
	{
                if(subMedias->at(i)->getRealPath()==realPath)
                    return this;
	}

	for(int i=0;i<subMedias->size();i++)
	{
		if(!subMedias->at(i)->isDirectory()) continue;
		Dir *find=((Dir*)subMedias->at(i))->findMediaParentByRealPath(realPath);
                if(find!=NULL)
                    return find;
	}
	return NULL;
}


//Permet de donner l'objet à prevenir en cas de modif
void Dir::setSignalListener(HddInterface *hddInterface)
{
	this->hddInterface=hddInterface;

        //Change tous les signalListener de tous les répertoires contenus dans subMedias
	for(int i=0;i<subMedias->size();i++)
	{
		if(!subMedias->at(i)->isDirectory()) continue;
		Dir *d=(Dir*)subMedias->at(i);
		d->setSignalListener(hddInterface);
	}
}


//Le destructeur qui détruit tout ce qu'il trouve sur son passage :P
Dir::~Dir()
{
	while(!subMedias->isEmpty())
	{
		delete subMedias->at(0);
		subMedias->remove(0);
	}
	delete subMedias;
	delete watcher;
}

