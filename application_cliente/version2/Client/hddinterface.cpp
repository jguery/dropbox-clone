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

	//On démarre l'exécution
	this->start();

	//On demande au network de nous prevenir lorsqu'ils détectent une modif
	this->networkInterface->setWaitReceiveRequestList(&waitCondition);

	//On demande aux repertoires de nous prevenir lorsqu'ils détectent une modif
	this->configurationData->getConfigurationFile()->setWaitConditionDetection(&waitCondition);
	this->configurationData->getConfigurationFile()->setListenning(true);
}



void HddInterface::run()
{
	forever
	{
		mutexWaitCondition.lock();
		waitCondition.wait(&mutexWaitCondition,5000);
		mutexWaitCondition.unlock();

		//On vérifie qu'on est toujours bien connecté
		networkInterface->blockWhileDisconnected();

		bool detect;
		//y a t-il des medias détecté?
		do
		{
			detect=false;
			Media *m=configurationData->getConfigurationFile()->getMediaDetection();
			if(m!=NULL)
			{
				this->detectedMedia(m);
				detect=true;
				continue;
			}
			//y a t-il des requetes recues?
			Request *r=networkInterface->getReceiveRequestList();
			if(r!=NULL)
			{
				this->receivedRequest(r);
				detect=true;
				continue;
			}
		}
		while(detect);
	}
}


//Lorsqu'on détecte un média
void HddInterface::detectedMedia(Media *m)
{
	State state=m->getDetectionState()->first();
	qDebug("%s : %s",m->getLocalPath().toStdString().c_str(),Media::stateToString(state).toStdString().c_str());
	if(state==MediaIsCreating) detectedCreatedMedia(m);
	else if(state==MediaIsUpdating) detectedUpdatedMedia(m);
	else if(state==MediaIsRemoving) detectedRemovedMedia(m);
	this->sleep(3);
}


//Lorsqu'on recoit une requete
void HddInterface::receivedRequest(Request *r)
{
	RequestEnum e=r->getType();
	if(e==CREATE_FILE_INFO)
	{
		QString realPath=r->getParameters().value("realPath","");
		bool isDirectory=(r->getParameters().value("isDirectory","")=="true")?true:false;
		receivedCreatedRequest(realPath,isDirectory);
	}
	else if(e==UPDATE_FILE_INFO)
	{
		QString realPath=r->getParameters().value("realPath","");
		QByteArray content=r->getParameters().value("content","");
		receivedUpdatedRequest(realPath,content);
	}
	else if(e==REMOVE_FILE_INFO)
	{
		QString realPath=r->getParameters().value("realPath","");
		receivedRemovedRequest(realPath);
	}
}




//Lorsqu'un média est créé... On prévient l'interface réseau (pour prévenir les autres clients connectés)
//Principalement appelée par Dir::directoryChangedAction (car le watcher est installé dans la classe Dir)
int HddInterface::detectedCreatedMedia(Media *m)
{
	//On récupère le realPath de l'objet
	QString realPath=m->getRealPath();

	//On prévient l'interface réseau, en passant le realPath
	ResponseEnum response=networkInterface->sendMediaCreated(realPath,m->isDirectory());

	if(response==ACCEPT_FILE_INFO)
	{
		configurationData->getConfigurationFile()->removeMediaDetection();
		m->getDetectionState()->removeFirst();
		Widget::addRowToTable("Réponse: Le serveur a accepté la modif",model,MSG_HDD);
	}
	else if(response==REJECT_FILE_INFO_FOR_IDENTIFICATION)
	{
		Widget::addRowToTable("Réponse: Le serveur requiert une identification",model,MSG_HDD);
	}
	else if(response==REJECT_FILE_INFO_FOR_PARAMETERS)
	{
		Widget::addRowToTable("Réponse: Le serveur a détecté des parametres erronés dans le message",model,MSG_HDD);
	}
	else if(response==REJECT_FILE_INFO_FOR_RIGHT)
	{
		configurationData->getConfigurationFile()->removeMediaDetection();
		m->getDetectionState()->removeFirst();
		Widget::addRowToTable("Réponse: Vos droits sont insuffisants",model,MSG_HDD);
	}
	else if(response==REJECT_FILE_INFO_FOR_CONFLICT)
	{
		configurationData->getConfigurationFile()->removeMediaDetection();
		m->getDetectionState()->removeFirst();
		Widget::addRowToTable("Réponse: Un conflit a été détecté",model,MSG_HDD);
	}
	else if(response==NOT_CONNECT)
	{
		Widget::addRowToTable("Vous n'etes pas connecté au serveur",model,MSG_HDD);
	}
	else if(response==NOT_IDENTIFICATE)
	{
		Widget::addRowToTable("Vous n'etes pas identifié sur serveur",model,MSG_HDD);
	}
	else if(response==NOT_SEND)
	{
		Widget::addRowToTable("Le message n'a pas été envoyé",model,MSG_HDD);
	}
	else if(response==NOT_TIMEOUT)
	{
		Widget::addRowToTable("Le serveur tarde trop à répondre",model,MSG_HDD);
	}
	else if(response==NOT_PARAMETERS)
	{
		Widget::addRowToTable("Le message à envoyer contient des parametres érronnés",model,MSG_HDD);
	}

	//Sauve la nouvelle config des fichiers synchronisés
	configurationData->save();
	return 0;
}




//Lorsqu'un média est supprimé
//Même shéma que la méthode du dessus
int HddInterface::detectedRemovedMedia(Media *m)
{
	//On récupère le realPath et on prévient le networkInterface
	QString realPath=m->getRealPath();

	ResponseEnum response=networkInterface->sendMediaRemoved(realPath);

	if(response==ACCEPT_FILE_INFO)
	{
		m->getParent()->delSubMedia(m);
		configurationData->getConfigurationFile()->removeMediaDetection();
		m->getDetectionState()->removeFirst();
		Widget::addRowToTable("Réponse: Le serveur a accepté la modif",model,MSG_HDD);
	}
	else if(response==REJECT_FILE_INFO_FOR_IDENTIFICATION)
	{
		Widget::addRowToTable("Réponse: Le serveur requiert une identification",model,MSG_HDD);
	}
	else if(response==REJECT_FILE_INFO_FOR_PARAMETERS)
	{
		Widget::addRowToTable("Réponse: Le serveur a détecté des parametres erronés dans le message",model,MSG_HDD);
	}
	else if(response==REJECT_FILE_INFO_FOR_RIGHT)
	{
		m->getParent()->delSubMedia(m);
		configurationData->getConfigurationFile()->removeMediaDetection();
		m->getDetectionState()->removeFirst();
		Widget::addRowToTable("Réponse: Vos droits sont insuffisants",model,MSG_HDD);
	}
	else if(response==REJECT_FILE_INFO_FOR_CONFLICT)
	{
		m->getParent()->delSubMedia(m);
		configurationData->getConfigurationFile()->removeMediaDetection();
		m->getDetectionState()->removeFirst();
		Widget::addRowToTable("Réponse: Un conflit a été détecté",model,MSG_HDD);
	}
	else if(response==NOT_CONNECT)
	{
		Widget::addRowToTable("Vous n'etes pas connecté au serveur",model,MSG_HDD);
	}
	else if(response==NOT_IDENTIFICATE)
	{
		Widget::addRowToTable("Vous n'etes pas identifié sur serveur",model,MSG_HDD);
	}
	else if(response==NOT_SEND)
	{
		Widget::addRowToTable("Le message n'a pas été envoyé",model,MSG_HDD);
	}
	else if(response==NOT_TIMEOUT)
	{
		Widget::addRowToTable("Le serveur tarde trop à répondre",model,MSG_HDD);
	}
	else if(response==NOT_PARAMETERS)
	{
		Widget::addRowToTable("Le message à envoyer contient des parametres érronnés",model,MSG_HDD);
	}

	//Sauve la nouvelle config des fichiers synchronisés
	configurationData->save();
	return 0;
}


//Lorsqu'un fichier est modifié
int HddInterface::detectedUpdatedMedia(Media *m)
{
	if(m->isDirectory()) return 1;
	File *f=(File*)m;
	QString realPath=f->getRealPath();

	QFile file(f->getLocalPath());      
	file.open(QIODevice::ReadOnly);
	QByteArray content=file.readAll();
	file.close();

	//Envoie le contenu de tout le fichier à l'interface réseau
	ResponseEnum response=networkInterface->sendMediaUpdated(realPath,content);

	if(response==ACCEPT_FILE_INFO)
	{
		configurationData->getConfigurationFile()->removeMediaDetection();
		m->getDetectionState()->removeFirst();
		Widget::addRowToTable("Réponse: Le serveur a accepté la modif",model,MSG_HDD);
	}
	else if(response==REJECT_FILE_INFO_FOR_IDENTIFICATION)
	{
		Widget::addRowToTable("Réponse: Le serveur requiert une identification",model,MSG_HDD);
	}
	else if(response==REJECT_FILE_INFO_FOR_PARAMETERS)
	{
		Widget::addRowToTable("Réponse: Le serveur a détecté des parametres erronés dans le message",model,MSG_HDD);
	}
	else if(response==REJECT_FILE_INFO_FOR_RIGHT)
	{
		configurationData->getConfigurationFile()->removeMediaDetection();
		m->getDetectionState()->removeFirst();
		Widget::addRowToTable("Réponse: Vos droits sont insuffisants",model,MSG_HDD);
	}
	else if(response==REJECT_FILE_INFO_FOR_CONFLICT)
	{
		configurationData->getConfigurationFile()->removeMediaDetection();
		m->getDetectionState()->removeFirst();
		Widget::addRowToTable("Réponse: Un conflit a été détecté",model,MSG_HDD);
	}
	else if(response==NOT_CONNECT)
	{
		Widget::addRowToTable("Vous n'etes pas connecté au serveur",model,MSG_HDD);
	}
	else if(response==NOT_IDENTIFICATE)
	{
		Widget::addRowToTable("Vous n'etes pas identifié sur serveur",model,MSG_HDD);
	}
	else if(response==NOT_SEND)
	{
		Widget::addRowToTable("Le message n'a pas été envoyé",model,MSG_HDD);
	}
	else if(response==NOT_TIMEOUT)
	{
		Widget::addRowToTable("Le serveur tarde trop à répondre",model,MSG_HDD);
	}
	else if(response==NOT_PARAMETERS)
	{
		Widget::addRowToTable("Le message à envoyer contient des parametres érronnés",model,MSG_HDD);
	}

	configurationData->save();
	return 0;
}




//on a recu la modif d'un fichier
void HddInterface::receivedUpdatedRequest(QString realPath, QByteArray content)
{
	Media *m=configurationData->getConfigurationFile()->findMediaByRealPath(realPath);
	if(m==NULL) return;
	if(m->isDirectory()) return ;
	File *f=(File*)m;

	//On récupère le dossier parent du fichier updated
	Dir *parent=f->getParent();
	if(!parent)
		return;

	//Met l'écoute de ce dossier à NULL, pour qu'aucun signal de modification
	//de dossier ne soit envoyé alors que nous contrôlons cette modification
	parent->setListenning(false);

	//Écrit le nouveau fichier
	QFile file(f->getLocalPath());
	if(!file.open(QIODevice::WriteOnly))
	{
		parent->setListenning(true);
		return;
	}
	file.write(content);
	file.close();
	f->updateHash();     //Met à jour la signature du fichier

	//Remet le dossier sur écoute
	parent->setListenning(true);

	Widget::addRowToTable("Message du serveur: le fichier "+f->getLocalPath()+" a été modifié",model,MSG_HDD);

	//Sauve toute la config
	configurationData->save();
}




//on a recu la création d'un media
void HddInterface::receivedCreatedRequest(QString realPath, bool isDirectory)
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
		Dir *d=parent->addSubDir(parent->getLocalPath()+"/"+realName,parent->getRealPath()+"/"+realName,0,false);
		if(!d)
			return;

		//Met le dossier créé sur écoute
		d->setListenning(true);

		Widget::addRowToTable("Message du serveur: le repertoire "+d->getLocalPath()+" a été créé",model,MSG_HDD);
	}

	else                //Le media créé est un fichier, il est vide (un autre message sera envoyé quand il sera plein)
	{
		QFile file(parent->getLocalPath()+"/"+realName);

		//Crée le fichier vide sur le disque
		if(!file.open(QIODevice::WriteOnly))
			return;

		file.write("");
		file.close();

		//Crée le fichier à l'arbo de synchronisation
		File *f=parent->addSubFile(parent->getLocalPath()+"/"+realName,parent->getRealPath()+"/"+realName,0,false);
		if(!f)
			return;

		Widget::addRowToTable("Message du serveur: le fichier "+f->getLocalPath()+" a été créé",model,MSG_HDD);
	}

	//On remet le dossier parent sur écoute
	parent->setListenning(true);

	configurationData->save();
}



void HddInterface::receivedRemovedRequest(QString realPath)
{
	Media *m=configurationData->getConfigurationFile()->findMediaByRealPath(realPath);
	if(m==NULL) return ;

	//Trouve le dossier parent du media supprimé
	Dir *parent=m->getParent();
	if(!parent)
		return;

	//Met à off l'écoute de parent
	parent->setListenning(false);

	parent->delSubMedia(m);

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


