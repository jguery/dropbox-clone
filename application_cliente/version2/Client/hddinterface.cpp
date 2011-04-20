#include "hddinterface.h"
#include "widget.h"


//Pour allouer un objet de type hddInterface
HddInterface *HddInterface::createHddInterface(ConfigurationData *configurationData, NetworkInterface *networkInterface,QStandardItemModel *model)
{
	//On vérifie qu'aucun des paramètres n'est NULL
	if(configurationData==NULL || networkInterface==NULL)
	{
		if(model) Widget::addRowToTable("Erreur d'allocation du module de transferts",model,MSG_1);
		return NULL;
	}

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

	Widget::addRowToTable("Le module de transferts a bien été alloué",model,MSG_1);
}



void HddInterface::run()
{
	this->networkInterface->sendIdentification();
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
	Widget::addRowToTable(QString("Traitement du ")+QString(m->isDirectory()?"repertoire":"ficher")+QString(" qui est passé à l'état ")+Media::stateToString(m->getDetectionState()->first()),model,MSG_1);
	if(state==MediaIsCreating) detectedCreatedMedia(m);
	else if(state==MediaIsUpdating) detectedUpdatedMedia(m);
	else if(state==MediaIsRemoving) detectedRemovedMedia(m);
}


//Lorsqu'on recoit une requete
void HddInterface::receivedRequest(Request *r)
{
	Widget::addRowToTable("Traitement d'une requête reçu du serveur",model,MSG_1);
	RequestEnum e=r->getType();
	if(e==CREATE_FILE_INFO)
	{
		QString realPath=r->getParameters()->value("realPath","");
		bool isDirectory=(r->getParameters()->value("isDirectory","")=="true")?true:false;
		receivedCreatedRequest(realPath,isDirectory);
	}
	else if(e==UPDATE_FILE_INFO)
	{
		QString realPath=r->getParameters()->value("realPath","");
		QByteArray content=r->getParameters()->value("content","");
		receivedUpdatedRequest(realPath,content);
	}
	else if(e==REMOVE_FILE_INFO)
	{
		QString realPath=r->getParameters()->value("realPath","");
		receivedRemovedRequest(realPath);
	}
}




//Lorsqu'un média est créé... On prévient l'interface réseau (pour prévenir les autres clients connectés)
//Principalement appelée par Dir::directoryChangedAction (car le watcher est installé dans la classe Dir)
int HddInterface::detectedCreatedMedia(Media *m)
{
	//On récupère le realPath de l'objet
	QString realPath=m->getRealPath();

	Widget::addRowToTable("Envoi de la requête au serveur",model,MSG_1);
	//On prévient l'interface réseau, en passant le realPath
	ResponseEnum response=networkInterface->sendMediaCreated(realPath,m->isDirectory());

	if(response==ACCEPT_FILE_INFO)
	{
		configurationData->getConfigurationFile()->removeMediaDetection();
		Widget::addRowToTable("Le serveur a accepté le changement",model,MSG_2);
	}
	else if(response==REJECT_FILE_INFO_FOR_IDENTIFICATION)
	{
		Widget::addRowToTable("Le serveur requiert une identification",model,MSG_3);
	}
	else if(response==REJECT_FILE_INFO_FOR_PARAMETERS)
	{
		Widget::addRowToTable("Le serveur a détecté une requete eronnée",model,MSG_3);
	}
	else if(response==REJECT_FILE_INFO_FOR_RIGHT)
	{
		configurationData->getConfigurationFile()->removeMediaDetection();
		Widget::addRowToTable("Le serveur a signalé une insufisance de droits",model,MSG_3);
	}
	else if(response==REJECT_FILE_INFO_FOR_CONFLICT)
	{
		configurationData->getConfigurationFile()->removeMediaDetection();
		Widget::addRowToTable("Le serveur a détecté un conflit",model,MSG_3);
	}
	else if(response==NOT_CONNECT)
	{
		Widget::addRowToTable("Echec d'envoi, vous n'etes pas connecté au serveur",model,MSG_3);
	}
	else if(response==NOT_IDENTIFICATE)
	{
		Widget::addRowToTable("Echec d'envoi, vous n'etes pas identifié",model,MSG_3);
		this->networkInterface->sendIdentification();
	}
	else if(response==NOT_SEND)
	{
		Widget::addRowToTable("La requête n'a pas pu être envoi",model,MSG_3);
	}
	else if(response==NOT_TIMEOUT)
	{
		Widget::addRowToTable("Le serveur tarde trop à répondre",model,MSG_3);
	}
	else if(response==NOT_PARAMETERS)
	{
		Widget::addRowToTable("Le message à envoyer contient des parametres érronnés",model,MSG_3);
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
		Widget::addRowToTable("Le serveur a accepté le changement",model,MSG_2);
	}
	else if(response==REJECT_FILE_INFO_FOR_IDENTIFICATION)
	{
		Widget::addRowToTable("Le serveur requiert une identification",model,MSG_3);
	}
	else if(response==REJECT_FILE_INFO_FOR_PARAMETERS)
	{
		Widget::addRowToTable("Le serveur a détecté une requete eronnée",model,MSG_3);
	}
	else if(response==REJECT_FILE_INFO_FOR_RIGHT)
	{
		m->getParent()->delSubMedia(m);
		configurationData->getConfigurationFile()->removeMediaDetection();
		Widget::addRowToTable("Le serveur a signalé une insufisance de droits",model,MSG_3);
	}
	else if(response==REJECT_FILE_INFO_FOR_CONFLICT)
	{
		m->getParent()->delSubMedia(m);
		configurationData->getConfigurationFile()->removeMediaDetection();
		Widget::addRowToTable("Le serveur a détecté un conflit",model,MSG_3);
	}
	else if(response==NOT_CONNECT)
	{
		Widget::addRowToTable("Echec d'envoi, vous n'etes pas connecté au serveur",model,MSG_3);
	}
	else if(response==NOT_IDENTIFICATE)
	{
		Widget::addRowToTable("Echec d'envoi, vous n'etes pas identifié",model,MSG_3);
		this->networkInterface->sendIdentification();
	}
	else if(response==NOT_SEND)
	{
		Widget::addRowToTable("La requête n'a pas pu être envoi",model,MSG_3);
	}
	else if(response==NOT_TIMEOUT)
	{
		Widget::addRowToTable("Le serveur tarde trop à répondre",model,MSG_3);
	}
	else if(response==NOT_PARAMETERS)
	{
		Widget::addRowToTable("Le message à envoyer contient des parametres érronnés",model,MSG_3);
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
		Widget::addRowToTable("Le serveur a accepté le changement",model,MSG_2);
	}
	else if(response==REJECT_FILE_INFO_FOR_IDENTIFICATION)
	{
		Widget::addRowToTable("Le serveur requiert une identification",model,MSG_3);
	}
	else if(response==REJECT_FILE_INFO_FOR_PARAMETERS)
	{
		Widget::addRowToTable("Le serveur a détecté une requete eronnée",model,MSG_3);
	}
	else if(response==REJECT_FILE_INFO_FOR_RIGHT)
	{
		configurationData->getConfigurationFile()->removeMediaDetection();
		Widget::addRowToTable("Le serveur a signalé une insufisance de droits",model,MSG_3);
	}
	else if(response==REJECT_FILE_INFO_FOR_CONFLICT)
	{
		configurationData->getConfigurationFile()->removeMediaDetection();
		Widget::addRowToTable("Le serveur a détecté un conflit",model,MSG_3);
	}
	else if(response==NOT_CONNECT)
	{
		Widget::addRowToTable("Echec d'envoi, vous n'etes pas connecté au serveur",model,MSG_3);
	}
	else if(response==NOT_IDENTIFICATE)
	{
		Widget::addRowToTable("Echec d'envoi, vous n'etes pas identifié",model,MSG_3);
		this->networkInterface->sendIdentification();
	}
	else if(response==NOT_SEND)
	{
		Widget::addRowToTable("La requête n'a pas pu être envoi",model,MSG_3);
	}
	else if(response==NOT_TIMEOUT)
	{
		Widget::addRowToTable("Le serveur tarde trop à répondre",model,MSG_3);
	}
	else if(response==NOT_PARAMETERS)
	{
		Widget::addRowToTable("Le message à envoyer contient des parametres érronnés",model,MSG_3);
	}

	configurationData->save();
	return 0;
}




//on a recu la modif d'un fichier
void HddInterface::receivedUpdatedRequest(QString realPath, QByteArray content)
{
	Widget::addRowToTable("La requete signale que le media de realPath "+realPath+" est à l'état MediaIsUpdating",model,MSG_2);
	Media *m=configurationData->getConfigurationFile()->findMediaByRealPath(realPath);
	if(m==NULL)
	{
		Widget::addRowToTable("Le media est introuvable",model,MSG_3);
		return;
	}
	if(m->isDirectory())
	{
		Widget::addRowToTable("Le media est un repertoire",model,MSG_3);
		return ;
	}
	File *f=(File*)m;

	//On récupère le dossier parent du fichier updated
	Dir *parent=f->getParent();
	if(!parent)
	{
		Widget::addRowToTable("Le media n'a pas de parent",model,MSG_3);
		return ;
	}

	//Met l'écoute de ce dossier à NULL, pour qu'aucun signal de modification
	//de dossier ne soit envoyé alors que nous contrôlons cette modification
	parent->setListenning(false);

	//Écrit le nouveau fichier
	QFile file(f->getLocalPath());
	if(!file.open(QIODevice::WriteOnly))
	{
		parent->setListenning(true);
		Widget::addRowToTable("Impossible d'ouvrir le media en écriture",model,MSG_3);
		return;
	}
	file.write(content);
	file.close();
	f->updateHash();     //Met à jour la signature du fichier

	//Remet le dossier sur écoute
	parent->setListenning(true);

	Widget::addRowToTable("Le media "+f->getLocalPath()+" a bien été modifié en local",model,MSG_2);

	//Sauve toute la config
	configurationData->save();
}




//on a recu la création d'un media
void HddInterface::receivedCreatedRequest(QString realPath, bool isDirectory)
{
	Widget::addRowToTable(QString("La requete signale que le ")+QString(isDirectory?"repertoire":"fichier")+QString(" de realPath ")+realPath+QString(" est à l'état MediaIsCreating"),model,MSG_2);
	Media *mParent=configurationData->getConfigurationFile()->findMediaByRealPath(Media::extractParentPath(realPath));
	if(mParent==NULL)
	{
		Widget::addRowToTable("Le repertoire parent du media est introuvable",model,MSG_3);
		return;
	}
	if(!mParent->isDirectory())
	{
		Widget::addRowToTable("Le parent du media n'est pas un repertoire",model,MSG_3);
		return;
	}
	Dir *parent=(Dir*)mParent;

	//Stope l'écoute du dossier parent
	parent->setListenning(false);

	QString realName=Media::extractName(realPath);

	if(isDirectory)      //Si le media créé est un dossier
	{
		QDir dir(parent->getLocalPath());
		if(!dir.mkdir(realName))        //On crée le répertoire créé chez un autre client
		{
			Widget::addRowToTable("Impossible de créer le repertoire "+realName+" dans "+parent->getLocalPath()+"/",model,MSG_3);
			return;
		}

		//Crée le répertoire dans l'arborescence de synchronisation du client
		Dir *d=parent->addSubDir(parent->getLocalPath()+"/"+realName,parent->getRealPath()+"/"+realName);
		if(!d)
		{
			Widget::addRowToTable("L'allocation du repertoire dans l'arborescence a échoué",model,MSG_3);
			return;
		}

		//Met le dossier créé sur écoute
		d->setListenning(true);

		Widget::addRowToTable("Message du serveur: le repertoire "+d->getLocalPath()+" a été créé",model,MSG_2);
	}

	else                //Le media créé est un fichier, il est vide (un autre message sera envoyé quand il sera plein)
	{
		QFile file(parent->getLocalPath()+"/"+realName);

		//Crée le fichier vide sur le disque
		if(!file.open(QIODevice::WriteOnly))
		{
			Widget::addRowToTable("Impossible de créer le fichier "+realName+" dans "+parent->getLocalPath()+"/",model,MSG_3);
			return;
		}

		file.write("");
		file.close();

		//Crée le fichier à l'arbo de synchronisation
		File *f=parent->addSubFile(parent->getLocalPath()+"/"+realName,parent->getRealPath()+"/"+realName);
		if(!f)
		{
			Widget::addRowToTable("L'allocation du fichier dans l'arborescence a échoué",model,MSG_3);
			return;
		}

		Widget::addRowToTable("Message du serveur: le fichier "+f->getLocalPath()+" a été créé",model,MSG_2);
	}

	//On remet le dossier parent sur écoute
	parent->setListenning(true);

	configurationData->save();
}



void HddInterface::receivedRemovedRequest(QString realPath)
{
	Widget::addRowToTable("La requete signale que le media de realPath "+realPath+" est à l'état MediaIsRemoving",model,MSG_2);
	Media *m=configurationData->getConfigurationFile()->findMediaByRealPath(realPath);
	if(m==NULL)
	{
		Widget::addRowToTable("Le media est introuvable",model,MSG_3);
		return;
	}

	//Trouve le dossier parent du media supprimé
	Dir *parent=m->getParent();
	if(!parent)
	{
		Widget::addRowToTable("Le repertoire parent du media est introuvable",model,MSG_3);
		return;
	}

	//Met à off l'écoute de parent
	parent->setListenning(false);

	parent->delSubMedia(m);

	if(m->isDirectory())    //On supprime un répertoire
	{
		Dir *d=(Dir*)m;
		d->setListenning(false);
		Dir::removeNonEmptyDirectory(d->getLocalPath());
		Widget::addRowToTable("Message du serveur: le repertoire "+d->getLocalPath()+" a été supprimé",model,MSG_2);
		delete d;
	}

	else                    //On supprime un fichier
	{
		File *f=(File*)m;
		QFile::remove(f->getLocalPath());

		Widget::addRowToTable("Message du serveur: le fichier "+f->getLocalPath()+" a été supprimé",model,MSG_2);
		delete f;
	}

	//Remet l'écoute de parent à ON
	parent->setListenning(true);

	configurationData->save();
}


