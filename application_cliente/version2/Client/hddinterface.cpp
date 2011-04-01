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
                           QStandardItemModel *model) : QObject()
{
	//On fait les initialisations
	this->networkInterface=networkInterface;
	this->configurationData=configurationData;
	this->model=model;

	//On demande aux repertoires de nous prevenir lorsqu'ils détectent une modif
	this->configurationData->getConfigurationFile()->setSignalListener(this);

        //Connecte les signaux de réceptions de messages de l'interface réseau à des slots de cette classe
        //Le traitement des messages est donc fait ICI
	QObject::connect(networkInterface,SIGNAL(receiveCreatedMediaMessage(Dir*,QString)),this,SLOT(receiveCreatedMediaMessageAction(Dir*,QString)));
	QObject::connect(networkInterface,SIGNAL(receiveModifiedFileMessage(File*,QByteArray)),this,SLOT(receiveModifiedFileMessageAction(File*,QByteArray)));
	QObject::connect(networkInterface,SIGNAL(receiveRemovedMediaMessage(Media*)),this,SLOT(receiveRemovedMediaMessageAction(Media*)));
	QObject::connect(networkInterface,SIGNAL(receiveErrorMessage(QString)),this,SLOT(receiveErrorMessageAction(QString)));
}


//Lorsqu'un média est créé... On prévient l'interface réseau (pour prévenir les autres clients connectés)
//Principalement appelée par Dir::directoryChangedAction (car le watcher est installé dans la classe Dir)
void HddInterface::mediaHasBeenCreated(Media *m)
{
	//On récupère le realPath de l'objet
	QString realPath=m->getRealPath();
	if(m->isDirectory()) realPath=realPath+"/";

        //On prévient l'interface réseau, en passant le path dans le svn
	networkInterface->sendMediaCreated(realPath);

	//Si c'est un repertoire, on annule son signalListener
	if(m->isDirectory())
	{
		Dir *d=(Dir*)m;
		d->setSignalListener(this);
	}
        Widget::addRowToTable("Le media "+m->getLocalPath()+" a été créé",model,MSG_HDD);

        //Sauve la nouvelle config des fichiers synchronisés
        configurationData->save();
}


//Lorsqu'un média est supprimé
//Même shéma que la méthode du dessus
void HddInterface::mediaHasBeenRemoved(Media *m)
{
	//On récupère le realPath et on prévient le networkInterface
	QString realPath=m->getRealPath();
	networkInterface->sendMediaRemoved(realPath);

	//Si c'est un repertoire on annule son signalListener
	if(m->isDirectory())
	{
		Dir *d=(Dir*)m;
		d->setSignalListener(NULL);
	}
        Widget::addRowToTable("Le media "+m->getLocalPath()+" a été supprimé",model,MSG_HDD);
	delete m;

	//Sauve la nouvelle config des fichiers synchronisés
	configurationData->save();
}


//Lorsqu'un fichier est modifié
void HddInterface::fileHasBeenUpdated(File *f)
{
	QString realPath=f->getRealPath();

	QFile file(f->getLocalPath());      
	file.open(QIODevice::ReadOnly);

        //Envoie le contenu de tout le fichier à l'interface réseau
	networkInterface->sendFileModified(realPath,file.readAll());

	file.close();

        Widget::addRowToTable("Le fichier "+f->getLocalPath()+" a été modifié",model,MSG_HDD);

	configurationData->save();
}




//on a recu la modif d'un fichier
void HddInterface::receiveModifiedFileMessageAction(File *f,QByteArray content)
{
	  //On récupère le dossier parent du fichier updated
        Dir *dir=configurationData->getConfigurationFile()->findMediaParentByLocalPath(f->getLocalPath());
        if(!dir)
            return;

        //Met l'écoute de ce dossier à NULL, pour qu'aucun signal de modification
        //de dossier ne soit envoyé alors que nous contrôlons cette modification
	dir->setSignalListener(NULL);

        //Écrit le nouveau fichier
	QFile file(f->getLocalPath());
        if(!file.open(QIODevice::WriteOnly))
	{
		  dir->setSignalListener(this);
            return;
	}
	file.write(content);
	file.close();
        f->updateContent();     //Met à jour la signature du fichier

        //Remet le dossier sur écoute
	dir->setSignalListener(this);

        Widget::addRowToTable("Message du serveur: le fichier "+f->getLocalPath()+" a été modifié",model,MSG_HDD);

        //Sauve toute la config
        configurationData->save();
}


//on a recu la création d'un media
void HddInterface::receiveCreatedMediaMessageAction(Dir *parent,QString realName)
{
        //Stope l'écoute du dossier parent
        parent->setSignalListener(NULL);

        if(realName.endsWith("/"))      //Si le media créé est un dossier
	{
		realName=realName.left(realName.length()-1);
		QDir dir(parent->getLocalPath());
                if(!dir.mkdir(realName))        //On crée le répertoire créé chez un autre client
                    return;

                //Crée le répertoire dans l'arborescence de synchronisation du client
		Dir *d=Dir::createDir(parent->getLocalPath()+"/"+realName,parent->getRealPath()+"/"+realName);
                if(!d)
                    return ;
		parent->getSubMedias()->append(d);

                //Met le dossier créé sur écoute
		d->setSignalListener(this);

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
		File *f=File::createFile(parent->getLocalPath()+"/"+realName,parent->getRealPath()+"/"+realName);
                if(!f)
                    return;
		parent->getSubMedias()->append(f);

                Widget::addRowToTable("Message du serveur: le fichier "+f->getLocalPath()+" a été créé",model,MSG_HDD);
	}

        //On remet le dossier parent sur écoute
	parent->setSignalListener(this);

	configurationData->save();
}



void HddInterface::receiveRemovedMediaMessageAction(Media *m)
{
        //Trouve le dossier parent du media supprimé
        Dir *parent=configurationData->getConfigurationFile()->findMediaParentByLocalPath(m->getLocalPath());
        if(!parent)
            return;

        //Met à off l'écoute de parent
	parent->setSignalListener(NULL);

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
		d->setSignalListener(NULL);
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
	parent->setSignalListener(this);

	configurationData->save();
}




//Un message inconnu a été recu, on l'affiche (pour le débogage)
void HddInterface::receiveErrorMessageAction(QString s)
{
	QMessageBox::information(NULL,"",s);
}



