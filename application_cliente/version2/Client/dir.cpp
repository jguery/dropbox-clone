#include "dir.h"



//Pour créer un repertoire à partir de son localPath et de son realPath
//Les paramètres localPath et realPath ne doivent pas être vides
//On retourne un repertoire vide, même si celui d'origine contient des fichiers

Dir *Dir::createDir(QString localPath,QString realPath,Dir *parent)
{
	//Les paramètres localPath et realPath ne doivent pas être vides
	if(localPath.isEmpty() || realPath.isEmpty()) return NULL;

	//Par convention les noms de repertoire ne doivent pas se terminer pas "/"
	if(localPath.endsWith("/")) localPath=localPath.left(localPath.length()-1);
	if(realPath.endsWith("/")) realPath=realPath.left(realPath.length()-1);

	//On vérifie si le repertoire existe sur le disque
	QDir localDir(localPath);
	if(!localDir.exists()) return NULL;

	//On crèe l'element
	Dir *dir=new Dir(localPath,realPath,parent);

	//On fait alors l'allocation d'un objet Dir
	return dir;
}




//Permet de charger le repertoire à partir d'un noeud xml.
Dir *Dir::loadDir(QDomNode noeud,Dir *parent)
{

	//On vérifie que le nom du noeud xml est bien "dir"
	if(noeud.toElement().tagName()!="dir")
		return NULL;

	//On récupère le localPath et realPath du noeud xml et on vérifie qu'ils ne sont pas vides
	QString localPath=noeud.toElement().attribute("localPath","");
	QString realPath=noeud.toElement().attribute("realPath","");
	if(localPath.isEmpty() || realPath.isEmpty())
		return NULL;

	//S'ils se terminent pas "/" on normalise en enlevant le slash (simple convention)
	if(localPath.endsWith("/")) localPath=localPath.left(localPath.length()-1);
	if(realPath.endsWith("/")) realPath=realPath.left(realPath.length()-1);

	//On récupère les attributs detectionState
	QString detectionStateString=noeud.toElement().attribute("detectionState","");
	QStringList listDetectionState=detectionStateString.split(",");

	//On stocke notre répertoire dans un objet Dir qui contiendra
	//tous les sous répertoires de ce répertoire, sotckés dans "subMedias"
	Dir *dir=new Dir(localPath,realPath,parent);

	//On ajoute des états de détection
	for(int i=0;i<listDetectionState.size();i++)
	{
		if(listDetectionState.at(i)!=Media::stateToString(MediaDefaultState))
		{
			dir->getDetectionState()->append(Media::stateFromString(listDetectionState.at(i)));
			dir->getParent()->getOldDetections()->append(dir);
		}
	}
	//On récupère les fils du noeud qui sont en fait les sous médias du "dir"
	QDomNodeList list=noeud.childNodes();

	//On parcours tous les sous médias de notre repertoire
	for(unsigned int i=0;i<list.length();i++)
	{
		QDomNode n=list.at(i); QDomElement e=n.toElement();
		if(e.tagName()=="dir")          //Le sous-médias est un répertoire
		{
			Dir *d=Dir::loadDir(n,dir);  //On le charge
			if(d==NULL)
			{
				delete dir;
				//S'il n'est pas valide on retourne NULL
				return NULL;
			}
			//On ajoute ce répertoire à la liste des subMedias de notre répertoire
			dir->subMedias->append(d);
		}

		else if(e.tagName()=="file")    //Le sous-élément est un fichier
		{
			File *f=File::loadFile(n,dir); //On le charge
			if(f==NULL)
			{
				delete dir;
				//S'il n'est pas valide on retourne NULL
				return NULL;
			}
			//On ajoute ce fichier à la liste des subMedias de notre répertoire
			dir->subMedias->append(f);
		}
	}

	//Tout est bon, on retourne le dir
	return dir;
}






//Oui, c'est bien un repertoire
bool Dir::isDirectory()
{
	return true;
}




//Le constructeur fait les initialisations, puis alloue le watcher, et connecte son signal directoryChanged
Dir::Dir(QString localPath,QString realPath,Dir *parent): Media(localPath,realPath,parent)
{
	this->subMedias=new QVector<Media*>();
	this->oldDetections=new QList<Media*>();
	watcher=new QFileSystemWatcher(this);

	QDir dir(localPath);
	if(dir.exists())
		watcher->addPath(localPath);  //Le watcher ne surveille que ce repertoire

	//On connecte son signal au slot
	if(parent!=NULL) QObject::connect(this,SIGNAL(detectChangement(Media*)),parent,SIGNAL(detectChangement(Media*)),Qt::QueuedConnection);
	QObject::connect(watcher,SIGNAL(directoryChanged(QString)),this,SLOT(directoryChangedAction()),Qt::QueuedConnection);

	this->listen=false;
}




//Cherche les éventuels modifications qu'il y a eu dans le repertoire
void Dir::directoryChangedAction()
{
	//Si on ne doit pas détecter les changements, on annule tout
	if(listen==false) return ;

	//On bloque l'accès
	this->lock();

	bool isChanged=false; //contiendra true lorsqu'on aura détecter un changement

	QList<QString> contenu1; // contiendra la liste des chemins de subMedias

	//On parcourt tous les subMedias pour voir si la modification les concerne
	for(int i=0;i<subMedias->size();i++)
	{
		Media *m=subMedias->at(i);

		if(m==NULL) continue;

		//Si le média est en cours de suppression, on ne le straite pas
		if(m->getDetectionState()->contains(MediaIsRemoving)) continue;

		if(!m->isDirectory())   //Si le Media est un fichier
		{
			File *f=(File*)m;   //On le stocke dans un objet File

			//Si le fichier a été supprimé
			if(f->hasBeenRemoved())
			{
				//On le met à l'état de suppression
				f->getDetectionState()->append(MediaIsRemoving);

				//On prévient le parent du repertoire
				emit detectChangement(f);

				isChanged=true; //On a détecter un changement

				continue; //Prochain fichier
			}

			//Le fichier a été modifié
			else if(f->hasBeenUpdated())
			{
				f->updateHash(); //Met à jour son hash

				//Il est à l'état de modification
				f->getDetectionState()->append(MediaIsUpdating);

				//alors on previent le parent
				emit detectChangement(f);

				isChanged=true; //On a détecter un changement
			}
		}

		else        //Si le Media est un répertoire
		{
			Dir *d=(Dir*)m; //On le stocke dans un objet Dir

			//Le répertoire a été supprimé
			if(d->hasBeenRemoved())
			{
				//On le place à l'état de suppression
				d->getDetectionState()->append(MediaIsRemoving);

				//Il ne détecte plus rien
				d->setListenning(false);

				//On prévient le parent
				emit detectChangement(d);   //signal pour prévenir le parent

				isChanged=true; //On a détecter un changement

				continue; //Prochain fichier
			}
		}
		//Si on a pas fait de remove, ajoute le chemin du media à une liste
		contenu1.append(m->getLocalPath());
	}

	//On va parcourir les fichiers et rép contenus physiquement dans le dossier
	//pour voir si la modification concerne la création d'un fichier ou d'un dossier
	QDir dir(this->getLocalPath());

	//On récupère tous les fichiers
	QList<QString> contenu2=dir.entryList(QDir::AllEntries | QDir::NoDotAndDotDot);
	QList<QString>::iterator i;
	for(i=contenu2.begin(); i!=contenu2.end(); i++)
	{
		QString p=this->getLocalPath()+"/"+*i; //Chemin d'un média dans notre répertoire

		//Si ce média c'est pas dans notre liste subMedias
		if(!contenu1.contains(p))
		{
			Media *m;

			QDir d(p);

			//Si c'est un répertoire, on crèe un objet Dir
			if(d.exists()) m=Dir::createDir(p,this->getRealPath()+"/"+*i,this);

			//Sinon, c'est un fichier qui a été créé
			else m=File::createFile(p,this->getRealPath()+"/"+*i,this);

			if(m==NULL) continue; //Une erreur s'est produite

			//on ajoute le media à la liste des subMedias
			subMedias->append(m);

			//Il est à l'état de création
			m->getDetectionState()->append(MediaIsCreating);

			//On informe le parent
			emit detectChangement(m);

			//On le met à l'écoute
			if(m->isDirectory())
			{
				Dir *d=(Dir*)m;
				d->setListenning(listen);
			}

			isChanged=true; //On a détecter un changement

			contenu1.append(p);
		}
	}

	//On relache l'accès
	this->unlock();

	//Si on a détecté un changement, on reprend le parcours
	if(isChanged) directoryChangedAction();
}





//Pour récupérer la liste des anciennes détections faites depuis le dernier
//lancement et qui n'ont pas été traitées
QList<Media*> *Dir::getOldDetections()
{
	return oldDetections;
}




//Détecte si le repertoire a été supprimé
bool Dir::hasBeenRemoved()
{
	QDir dir(this->getLocalPath());
        if(!dir.exists())
            return true;
	return false;
}




//Récupérer le nombre de sous médias
int Dir::numberSubMedia()
{
	//On bloque l'accès
	this->lock();
	int nb=subMedias->size();

	//On relache l'accès
	this->unlock();
	return nb;
}



//Récupérer un sous média à l'indice i
Media *Dir::getSubMedia(int i)
{
	//On bloque l'accès
	this->lock();
	if(i<0 || i>=subMedias->size())
	{
		this->unlock();
		return NULL;
	}
	Media *m=subMedias->at(i);
	//On relache
	this->unlock();
	return m;
}





//Ajouter un sous fichier
File *Dir::addSubFile(QString localPath,QString realPath)
{
	File *f=File::createFile(localPath,realPath,this);
	if(f==NULL) return NULL;
	this->lock();
	subMedias->append(f);
	this->unlock();
	return f;
}



//Ajouter un sous repertoire
Dir *Dir::addSubDir(QString localPath,QString realPath)
{
	Dir *d=Dir::createDir(localPath,realPath,this);
	if(d==NULL) return NULL;
	this->lock();
	subMedias->append(d);
	this->unlock();
	d->setListenning(listen);
	return d;
}




//Supprimer un sous média de la liste des sous médias, mais ne fait pas la désallocation
bool Dir::delSubMedia(Media *m)
{
	if(m==NULL) return false;
	this->lock();
	int i=subMedias->indexOf(m);
	if(i<0)
	{
		this->unlock();
		return false;
	}
	if(m->isDirectory())
	{
		Dir *d=(Dir*)m;
		d->setListenning(false);
	}
	subMedias->remove(i);
	this->unlock();
	return true;
}





//Retourne le code xml du repertoire et de ses sous medias
QDomElement Dir::toXml(QDomDocument *document)
{
	this->lock();
	//On crèe le noeud xml avec le nom "dir"
	QDomElement element=document->createElement("dir");

	//On écrit ses attributs localPath et realPath
	element.setAttribute("localPath",this->getLocalPath());
	element.setAttribute("realPath",this->getRealPath());

	//On écrit ses attributs detectionState, revision et readOnly
	QStringList listDetectionState;
	for(int i=0;i<this->getDetectionState()->length();i++)
	{
		if(this->getDetectionState()->at(i)!=MediaDefaultState)
		listDetectionState.append(Media::stateToString(this->getDetectionState()->at(i)));
	}
	element.setAttribute("detectionState",listDetectionState.join(","));

	//On parcours ses sous médias pour les ajouter commes des noeuds fils du noeud actuel
	for(int i=0;i<subMedias->size();i++)
	{
		QDomElement e=subMedias->at(i)->toXml(document);
		element.appendChild(e);
	}
	this->unlock();
	//On retourne le noeud
	return element;
}









//Recherche à quel média correspond ce localPath
//Fonction récursive qui agit sur tous les submedias
Media *Dir::findMediaByLocalPath(QString localPath)
{
	//On normalise le localPath
	if(localPath.endsWith("/")) localPath=localPath.left(localPath.length()-1);

	//On teste si le localPath est égal au localPath de l'objet
	if(this->getLocalPath()==localPath)
		return this;

	this->lock();
	//Sinon on recherche dans tous les sous médias
	for(int i=0;i<subMedias->size();i++)
	{
		Media *find=subMedias->at(i)->findMediaByLocalPath(localPath);
		if(find!=NULL)
		{
			this->unlock();
			return find;
		}
	}
	this->unlock();
	//On a rien trouvé dans la recherche, on retourne NULL
	return NULL;
}








//Recherche à quel média correspond ce realPath
//Fonction récursive qui agit sur tous les submedias
Media *Dir::findMediaByRealPath(QString realPath)
{
	//On normalise le realPath
	if(realPath.endsWith("/")) realPath=realPath.left(realPath.length()-1);

	//On teste si le realPath est égal au realPath de l'objet
	if(this->getRealPath()==realPath)
		return this;

	this->lock();
	//Sinon on recherche dans tous les sous médias
	for(int i=0;i<subMedias->size();i++)
	{
		Media *find=subMedias->at(i)->findMediaByRealPath(realPath);
		if(find!=NULL)
		{
			this->unlock();
			return find;
		}
	}
	this->unlock();
	//On a rien trouvé dans la recherche, on retourne NULL
	return NULL;
}









//Permet de donner l'objet à prevenir en cas de modif
void Dir::setListenning(bool listen)
{
	this->listen=listen;

	if(listen)
	{
		//S'il y a des anciennes détections, on les traite.
		while(!oldDetections->isEmpty())
		{
			Media *m=oldDetections->first();
			oldDetections->removeFirst();
			emit detectChangement(m);
		}
	}

	directoryChangedAction();

	this->lock();
        //Change tous les signalListener de tous les répertoires contenus dans subMedias
	for(int i=0;i<subMedias->size();i++)
	{
		if(!subMedias->at(i)->isDirectory()) continue;
		Dir *d=(Dir*)subMedias->at(i);
		if(d->getDetectionState()->contains(MediaIsRemoving)) continue;
		d->setListenning(listen);
	}
	this->unlock();
}








//Une méthode statique qui supprime un répertoire non vide
void Dir::removeNonEmptyDirectory(QString path)
{
	QDir dir(path);

	//Supprime tous les fichier contenus dans le dossier path
	QFileInfoList fileList = dir.entryInfoList( QDir::Files | QDir::Hidden );
	foreach(QFileInfo file, fileList) dir.remove( file.absoluteFilePath());

	//Supprime tous les dossiers contenus dans le dossier path
	QFileInfoList dirList = dir.entryInfoList( QDir::AllDirs | QDir::Hidden | QDir::NoDotAndDotDot );
	foreach(QFileInfo dir, dirList) Dir::removeNonEmptyDirectory(dir.absoluteFilePath());

	//On supprime enfin le dossier name
	dir.rmdir(dir.absolutePath());
}








//Le destructeur qui détruit tout ce qu'il trouve sur son passage :P
Dir::~Dir()
{
	//On delete tous les sous médias, et on vide la liste des sous médias
	while(!(subMedias->isEmpty()))
	{
		delete subMedias->first();
		subMedias->remove(0);
	}
	//On delete la liste des sous médias
	delete subMedias;

	//On delete le watcher
	delete watcher;
}

