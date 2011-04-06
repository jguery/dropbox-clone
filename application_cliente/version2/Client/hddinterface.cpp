#include "hddinterface.h"
#include "widget.h"


//Pour allouer un objet de type hddInterface
HddInterface *HddInterface::createHddInterface(ConfigurationData *configurationData, NetworkInterface *networkInterface,QStandardItemModel *model)
{
	//On vérifie qu'aucun des paramètres n'est NULL
	if(configurationData==NULL)
		return NULL;

	if(networkInterface==NULL)
		return NULL;

	if(model==NULL)
		return NULL;

	//On crèe et on retourne l'objet
	return new HddInterface(configurationData,networkInterface,model);
}




//Le constructeur
HddInterface::HddInterface(ConfigurationData *configurationData,
					NetworkInterface *networkInterface,
					QStandardItemModel *model) : QThread()
{
	//On fait les initialisations
	this->networkInterface=networkInterface;
	this->configurationData=configurationData;
	this->model=model;

	this->moveToThread(this);

	//On demande aux repertoires de nous prevenir lorsqu'ils détectent une modif
	this->configurationData->getConfigurationFile()->setWaitConditionDetection(&waitConditionDetect);
	this->configurationData->getConfigurationFile()->setListenning(true);

	//On démarre l'exécution
	this->start();
}



void HddInterface::run()
{
	forever
	{
		mutexWaitCondition.lock();
		waitConditionDetect.wait(&mutexWaitCondition);
		mutexWaitCondition.unlock();
		Media *m=configurationData->getConfigurationFile()->getMediaDetection();
		if(m==NULL) continue;
		int reponse;
		State state=m->getState();
		if(state==MediaIsCreating) reponse=detectCreatedMedia(m);
		else if(state==MediaIsUpdating) reponse=detectUpdatedMedia(m);
		else if(state==MediaIsRemoving) reponse=detectRemovedMedia(m);
		if(reponse!=0) receiveError(reponse);
	}
}





//Lorsqu'un média est créé... On prévient l'interface réseau (pour prévenir les autres clients connectés)
//Principalement appelée par Dir::directoryChangedAction (car le watcher est installé dans la classe Dir)
int HddInterface::detectCreatedMedia(Media *m)
{
	//On récupère le realPath de l'objet
	QString realPath=m->getRealPath();

	//On prévient l'interface réseau, en passant le realPath
	int errorType=networkInterface->sendMediaCreated(realPath,m->isDirectory());

	if(errorType==0) m->setState(MediaNormalState);
	else ; ////////////////////// erreur

	Widget::addRowToTable("Le media "+m->getLocalPath()+" a été créé",model,MSG_HDD);

	//Sauve la nouvelle config des fichiers synchronisés
	configurationData->save();
	return 0;
}




//Lorsqu'un média est supprimé
//Même shéma que la méthode du dessus
int HddInterface::detectRemovedMedia(Media *m)
{
	//On récupère le realPath et on prévient le networkInterface
	QString realPath=m->getRealPath();
	int errorType=networkInterface->sendMediaRemoved(realPath);

	if(errorType==0) m->getParent()->removeSubMedia(m);
	else ; /////////////////////erreur

	Widget::addRowToTable("Le media "+m->getLocalPath()+" a été supprimé",model,MSG_HDD);

	//Sauve la nouvelle config des fichiers synchronisés
	configurationData->save();
	return 0;
}


//Lorsqu'un fichier est modifié
int HddInterface::detectUpdatedMedia(Media *m)
{
	if(m->isDirectory()) return 1;
	File *f=(File*)m;
	QString realPath=f->getRealPath();

	QFile file(f->getLocalPath());      
	file.open(QIODevice::ReadOnly);

	//Envoie le contenu de tout le fichier à l'interface réseau
	int errorType=networkInterface->sendFileModified(realPath,file.readAll());
	if(errorType==0) f->setState(MediaNormalState);
	else ; //errrrrrrrrrreeeurr

	file.close();

	Widget::addRowToTable("Le fichier "+f->getLocalPath()+" a été modifié",model,MSG_HDD);

	configurationData->save();
	return 0;
}




//on a recu la modif d'un fichier
void HddInterface::receiveUpdatedMedia(QString realPath, QByteArray content)
{
	Media *m=configurationData->getConfigurationFile()->findMediaByRealPath(realPath);
	if(m==NULL) return;
	if(m->isDirectory()) return ;
	File *f=(File*)m;

	//On récupère le dossier parent du fichier updated
	Dir *dir=f->getParent();
	if(!dir)
		return;

	//Met l'écoute de ce dossier à NULL, pour qu'aucun signal de modification
	//de dossier ne soit envoyé alors que nous contrôlons cette modification
	dir->setListenning(false);

	//Écrit le nouveau fichier
	QFile file(f->getLocalPath());
	if(!file.open(QIODevice::WriteOnly))
	{
		dir->setListenning(true);
		return;
	}
	file.write(content);
	file.close();
	f->updateHash();     //Met à jour la signature du fichier

	//Remet le dossier sur écoute
	dir->setListenning(true);

	Widget::addRowToTable("Message du serveur: le fichier "+f->getLocalPath()+" a été modifié",model,MSG_HDD);

	//Sauve toute la config
	configurationData->save();
}


//on a recu la création d'un media
void HddInterface::receiveCreatedMedia(QString realPath, bool isDirectory)
{
	Media *mParent=configurationData->getConfigurationFile()->findMediaByRealPath(Media::extractParentPath(realPath));
	if(mParent==NULL) return;
	if(!mParent->isDirectory()) return ;
	Dir *parent=(Dir*)mParent;

	//Stope l'écoute du dossier parent
	parent->setListenning(false);

	QString realName=Media::extractName(realPath);

	if(isDirectory)      //Si le media créé est un dossier
	{
		QDir dir(parent->getLocalPath());
		if(!dir.mkdir(realName))        //On crée le répertoire créé chez un autre client
			return;

		//Crée le répertoire dans l'arborescence de synchronisation du client
		Dir *d=Dir::createDir(parent->getLocalPath()+"/"+realName,parent->getRealPath()+"/"+realName,parent,MediaNormalState,0,false);
		if(!d)
			return ;
		parent->getSubMedias()->append(d);

		//Met le dossier créé sur écoute
		d->setListenning(true);

		Widget::addRowToTable("Message du serveur: le repertoire "+d->getLocalPath()+" a été créé",model,MSG_HDD);
	}

	else                        //Le media créé est un fichier, il est vide (un autre message sera envoyé quand il sera plein)
	{
		QFile file(parent->getLocalPath()+"/"+realName);

		//Crée le fichier vide sur le disque
		if(!file.open(QIODevice::WriteOnly))
			return;

		file.write("");
		file.close();

		//Crée le fichier à l'arbo de synchronisation
		File *f=File::createFile(parent->getLocalPath()+"/"+realName,parent->getRealPath()+"/"+realName,parent,MediaNormalState,0,false);
		if(!f)
			return;
		parent->getSubMedias()->append(f);

		Widget::addRowToTable("Message du serveur: le fichier "+f->getLocalPath()+" a été créé",model,MSG_HDD);
	}

	//On remet le dossier parent sur écoute
	parent->setListenning(true);

	configurationData->save();
}



void HddInterface::receiveRemovedMedia(QString realPath)
{
	Media *m=configurationData->getConfigurationFile()->findMediaByRealPath(realPath);
	if(m==NULL) return ;

	//Trouve le dossier parent du media supprimé
	Dir *parent=m->getParent();
	if(!parent)
		return;

	//Met à off l'écoute de parent
	parent->setListenning(false);

	//Enlève le media de la liste des subMedias de parent
	for(int i=0;i<parent->getSubMedias()->size();i++)
	{
		if(m==parent->getSubMedias()->at(i))
		{
			parent->getSubMedias()->remove(i);
			break;
		}
	}


	if(m->isDirectory())    //On supprime un répertoire
	{
		Dir *d=(Dir*)m;
		d->setListenning(false);
		Dir::removeNonEmptyDirectory(d->getLocalPath());
		Widget::addRowToTable("Message du serveur: le repertoire "+d->getLocalPath()+" a été supprimé",model,MSG_HDD);
		delete d;
	}

	else                    //On supprime un fichier
	{
		File *f=(File*)m;
		QFile::remove(f->getLocalPath());

		Widget::addRowToTable("Message du serveur: le fichier "+f->getLocalPath()+" a été supprimé",model,MSG_HDD);
		delete f;
	}

	//Remet l'écoute de parent à ON
	parent->setListenning(true);

	configurationData->save();
}




//Un message inconnu a été recu, on l'affiche (pour le débogage)
void HddInterface::receiveError(int errorNumber)
{
	QMessageBox::information(NULL,"",QString::number(errorNumber));
}


