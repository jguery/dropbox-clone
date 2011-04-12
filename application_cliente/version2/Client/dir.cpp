#include "dir.h"



//Pour créer un repertoire à partir de son localPath et de son realPath
//Les paramètres localPath et realPath ne doivent pas être vides
//Le repertoire doit exister à l'adresse localPath
//On retourne un repertoire vide, même si celui d'origine contient des fichiers

Dir *Dir::createDir(QString localPath,QString realPath,Dir *parent,int revision,bool readOnly)
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
	Dir *dir=new Dir(localPath,realPath,parent,revision,readOnly);

	//On fait alors l'allocation d'un objet Dir
	return dir;
}




//Permet de charger le repertoire à partir d'un noeud xml.
//A noter qu'il faut que le contenu du repertoire tel qu'il est décrit dans le noeud corresponde
//exactement au contenu réel du repertoire sur le disque. C'est à dire qu'il n'y ai eu aucune modif
//quand l'application n'était pas en marche
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

	//On récupère les attributs révision et readOnly du noeud xml.
	QString revisionString=noeud.toElement().attribute("revision","");
	QString readOnlyString=noeud.toElement().attribute("readOnly","");

	//On convertit ces attributs en int et bool
	int revision;bool readOnly;bool ok;
	revision=revisionString.toInt(&ok); if(!ok) revision=0;
	readOnly=readOnlyString=="true"?true:false;

	//On stocke notre répertoire(ou dépot) dans un objet Dir qui contiendra
	//tous les sous répertoires de ce répertoire, sotckés dans "subMedias"
	Dir *dir=new Dir(localPath,realPath,parent,revision,readOnly);

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
Dir::Dir(QString localPath,QString realPath,Dir *parent,int revision,bool readOnly): Media(localPath,realPath,parent,revision,readOnly)
{
	this->subMedias=new QVector<Media*>();
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

	bool isChanged=false; //contiendra true lorsqu'on aura détecter un changement

	QList<QString> contenu1;

	//On parcourt tous les subMedias pour voir si la modification les concerne
	for(int i=0;i<subMedias->size();i++)
	{
		Media *m=subMedias->at(i);
		if(!(m->getDetectionState()->isEmpty()) && m->getDetectionState()->last()==MediaIsRemoving) continue;

		if(!m->isDirectory())   //Si le Media est un fichier
		{
			File *f=(File*)m;   //On le stocke dans un objet File

			//Si le fichier a été supprimé
			if(f->hasBeenRemoved())
			{
				//Il est à l'état de suppression
				f->getDetectionState()->append(MediaIsRemoving);

				//Si le repertoire n'est pas en lecture seule, Et que le fichier n'est pas en lecture seule,
				//alors on prévient le parent du repertoire
				if(!f->isReadOnly() && !this->isReadOnly())
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

				//Si le fichier n'est pas en lecture seule, alors on previent le parent
				if(!f->isReadOnly())
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
				//Il est à l'état de suppression
				d->getDetectionState()->append(MediaIsRemoving);
				d->setListenning(false);

				//Si le parent n'est pas en lecture seule, Et que le repertoire n'est pas en lecture seule,
				//alors on prévient le parent
				if(!this->isReadOnly() && !d->isReadOnly())
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
			if(d.exists()) m=Dir::createDir(p,realPath+"/"+*i,this,0,this->isReadOnly());

			//Sinon, c'est un fichier qui a été créé
			//on l'ajoute aussi à la synchronisation
			else m=File::createFile(p,realPath+"/"+*i,this,0,this->isReadOnly());

			if(m==NULL) continue; //Une erreur s'est produite

			//on ajoute le media à la liste des subMedias
			subMedias->append(m);
			m->getDetectionState()->append(MediaIsCreating);

			//On informe le parent
			if(!this->isReadOnly())
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

	//Si on a détecté un changement, on reprend le parcours
	if(isChanged) directoryChangedAction();
}






//Détecte si le repertoire a été supprimé
bool Dir::hasBeenRemoved()
{
	QDir dir(localPath);
        if(!dir.exists())
            return true;
	return false;
}




//Récupérer le nombre de sous médias
int Dir::numberSubMedia()
{
	return subMedias->size();
}



//Récupérer un sous média à l'indice i
Media *Dir::getSubMedia(int i)
{
	if(i<0 || i>=subMedias->size()) return NULL;
	return subMedias->at(i);
}





//Ajouter un sous fichier
File *Dir::addSubFile(QString localPath,QString realPath,int revision,bool readOnly)
{
	File *f;
	f=File::createFile(localPath,realPath,this,revision,readOnly);
	if(f==NULL) return NULL;
	subMedias->append(f);
	return f;
}



//Ajouter un sous repertoire
Dir *Dir::addSubDir(QString localPath,QString realPath,int revision,bool readOnly)
{
	Dir *d;
	d=Dir::createDir(localPath,realPath,this,revision,readOnly);
	if(d==NULL) return NULL;
	subMedias->append(d);
	d->setListenning(listen);
	return d;
}




//Supprimer un sous média
void Dir::delSubMedia(Media *m)
{
	if(m==NULL) return;
	int i=subMedias->indexOf(m);
	if(i<0) return;
	if(m->isDirectory())
	{
		Dir *d=(Dir*)m;
		d->setListenning(false);
	}
	subMedias->remove(i);
}





//Retourne le code xml du repertoire et de ses sous medias
QDomElement Dir::toXml(QDomDocument *document)
{
	//On crèe le noeud xml avec le nom "dir"
	QDomElement element=document->createElement("dir");

	//On écrit ses attributs localPath et realPath
	element.setAttribute("localPath",localPath);
	element.setAttribute("realPath",realPath);

	//On écrit ses attributs revision et readOnly
	element.setAttribute("revision",QString::number(revision));
	element.setAttribute("readOnly",readOnly?"true":"false");

	//On parcours ses sous médias pour les ajouter commes des noeuds fils du noeud actuel
	for(int i=0;i<subMedias->size();i++)
	{
		QDomElement e=subMedias->at(i)->toXml(document);
		element.appendChild(e);
	}

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
	  if(this->localPath==localPath)
            return this;

	  //Sinon on recherche dans tous les sous médias
	for(int i=0;i<subMedias->size();i++)
	{
		Media *find=subMedias->at(i)->findMediaByLocalPath(localPath);
                if(find!=NULL)
                    return find;
	}

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
        if(this->realPath==realPath)
            return this;

	  //Sinon on recherche dans tous les sous médias
	for(int i=0;i<subMedias->size();i++)
	{
		Media *find=subMedias->at(i)->findMediaByRealPath(realPath);
                if(find!=NULL)
                    return find;
	}

	//On a rien trouvé dans la recherche, on retourne NULL
	return NULL;
}









//Permet de donner l'objet à prevenir en cas de modif
void Dir::setListenning(bool listen)
{
	this->listen=listen;
	directoryChangedAction();

        //Change tous les signalListener de tous les répertoires contenus dans subMedias
	for(int i=0;i<subMedias->size();i++)
	{
		if(!subMedias->at(i)->isDirectory()) continue;
		Dir *d=(Dir*)subMedias->at(i);
		if(!(d->getDetectionState()->isEmpty()) && d->getDetectionState()->last()==MediaIsRemoving) continue;
		d->setListenning(listen);
	}

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
		delete subMedias->at(0);
		subMedias->remove(0);
	}
	//On delete la liste des sous médias
	delete subMedias;

	//On delete le watcher
	delete watcher;
}

