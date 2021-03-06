#include "configurationdata.h"
#include "widget.h"



/*

 Ce fichier contient les implémentations de quatre classes:
 ConfigurationNetwork : Pour la gestion du réseau
 ConfigurationIdentification : Pour la configuration de l'identification
 ConfigurationFile: Pour la configuration des fichiers et repertoires synchronisés
 ConfigurationData: Pour regrouper toutes les configurations

*/




//Fonction statique pour créer une configuration network initiale
ConfigurationNetwork *ConfigurationNetwork::createConfigurationNetwork(QString address, int port)
{
	//On vérifie la validité de l'adresse et du port demandés
	if(address.isEmpty() || port<0 || port>=65536)
		return NULL;

	//On crèe l'objet et on le renvoi
	ConfigurationNetwork *configurationNetwork=new ConfigurationNetwork(address,port);
	return configurationNetwork;
}



//Permet de charger l'objet à partir d'un noeud xml
ConfigurationNetwork *ConfigurationNetwork::loadConfigurationNetwork(QDomNode noeud)
{
	QDomElement element=noeud.toElement();

	//On vérifie que le nom du noeud est bien ConfigurationNetwork
	if(element.tagName()!="ConfigurationNetwork")
		return NULL;

	//On récupère l'adresse et le port et on vérifie qu'ils ne sont pas vides
	QString address=element.attribute("address","");
	QString port=element.attribute("port","");
	if(address.isEmpty() || port.isEmpty())
		return NULL;

	//On convertit le port en format int
	bool ok=false;int p=port.toInt(&ok);
	if(!ok) return NULL;

	//On crèe l'objet et on le retourne
	ConfigurationNetwork *configurationNetwork =
				ConfigurationNetwork::createConfigurationNetwork(address,p);
	return configurationNetwork;
}




//Constructeur
ConfigurationNetwork::ConfigurationNetwork(QString address, int port)
{
	this->address=address;
	this->port=port;
}



//Les accesseurs et mutateurs de l'attribut address
QString ConfigurationNetwork::getAddress()
{
	return address;
}

QString ConfigurationNetwork::getFullAddress()
{
	return address+":"+QString::number(port);
}

void ConfigurationNetwork::setAddress(QString address)
{
	this->address=address;
}



//les accesseurs et mutateurs de l'attribut port
int ConfigurationNetwork::getPort()
{
	return port;
}

void ConfigurationNetwork::setPort(int port)
{
	this->port=port;
}



//Retourne le code xml correspondant à l'objet
QDomElement ConfigurationNetwork::toXml(QDomDocument *document)
{
	//On crèe le noeud xml avec le nom ConfigurationNetwork
	QDomElement element=document->createElement("ConfigurationNetwork");

	//On écrit ses attributs
	element.setAttribute("address",address);
	element.setAttribute("port",QString::number(port));

	return element;
}









//Pour créer une configuration d'identification initiale
ConfigurationIdentification *ConfigurationIdentification::createConfigurationIdentification(QString pseudo, QString password)
{
	//On vérifie que le pseudo et password ne sont pas vides
	if(pseudo.isEmpty() || password.isEmpty())
		return NULL;

	//On crèe l'objet et on le renvoi
	ConfigurationIdentification *configurationIdentification=new ConfigurationIdentification(pseudo,password);
	return configurationIdentification;
}



//Pour charger l'objet à partir d'un noeud xml
ConfigurationIdentification *ConfigurationIdentification::loadConfigurationIdentification(QDomNode noeud)
{
	QDomElement element=noeud.toElement();

	//On vérifie que le nom du noeud xml est bien ConfigurationIdentification
	if(element.tagName()!="ConfigurationIdentification")
		return NULL;

	//On récupère les pseudo et password du noeud et on vérifie qu'ils ne soient pas vides.
	QString pseudo=element.attribute("pseudo","");
	QString password=element.attribute("password","");
	if(pseudo.isEmpty() || password.isEmpty())
		return NULL;

	//On crèe l'objet et on le retourne
	ConfigurationIdentification *configurationIdentification=
					ConfigurationIdentification::createConfigurationIdentification(pseudo,password);
	return configurationIdentification;
}



//Le constructeur
ConfigurationIdentification::ConfigurationIdentification(QString pseudo,QString password)
{
	this->pseudo=pseudo;
	this->password=password;
}



//Les accesseurs et mutateurs de l'attribut pseudo
QString ConfigurationIdentification::getPseudo()
{
	return pseudo;
}

void ConfigurationIdentification::setPseudo(QString pseudo)
{
	this->pseudo=pseudo;
}



//les accesseurs et mutateurs de l'attribut password
QString ConfigurationIdentification::getPassword()
{
	return password;
}

void ConfigurationIdentification::setPassword(QString password)
{
	this->password=password;
}


//Retourne le code xml correspondant à la configuration
QDomElement ConfigurationIdentification::toXml(QDomDocument *document)
{
	//On crèe le noeud xml avec le nom ConfigurationIdentification
	QDomElement element=document->createElement("ConfigurationIdentification");

	//On écrit ses attributs pseudo et password et on le renvoi
	element.setAttribute("pseudo",pseudo);
	element.setAttribute("password",password);
	return element;
}










//Une fonction statique pour créer une configuration de dépots initiale
ConfigurationFile *ConfigurationFile::createConfigurationFile(QList<Depot*> *depots,QStandardItemModel *model)
{
	//La liste de dépots passée ne doit être ni NULLe, ni vide
	if(depots==NULL || depots->length()==0)
	{
		if(model!=NULL) Widget::addRowToTable("Echec à l'allocation du module de détections",model,MSG_1);
		return NULL;
	}

	//On crèe la configuration et on la retourne
	ConfigurationFile *config=new ConfigurationFile(model);
	if(config==NULL) return NULL;
	for(int i=0;i<depots->length();i++) config->addDepot(depots->at(i));
	return config;
}





//Pour charger la configuration des répertoires et fichiers depuis un noeud xml
//Typiquement, appelé dés le lancement du client, en chargant la config
//préalablement enregistrée dans un config.xml, grâce à ConfigurationData::save
ConfigurationFile *ConfigurationFile::loadConfigurationFile(QDomNode noeud,QStandardItemModel *model)
{
	//On vérifie que le nom du noeud est bien ConfigurationFile
	QDomElement element=noeud.toElement();
	if(element.tagName()!="ConfigurationFile")
	{
		Widget::addRowToTable("Erreur lors du chargement de la configuration de fichier.",model,MSG_1);
		return NULL;
	}

	//On cree la configuration et on s'assure qu'elle est créé
	ConfigurationFile *config=new ConfigurationFile(model);
	if(config==NULL) return NULL;

	//Parcours l'ensemble des éléments fils de l'élement "COnfigurationFile"
	//Ce sont donc des dépots qui contiennent des fichiers et des dossiers (toute l'arborescence à synchroniser enfaite)
	QDomNodeList list=noeud.childNodes();
	for(unsigned int i=0;i<list.length();i++)
	{
		QDomNode n=list.at(i);

		//On charge le noeud avec la méthode statique loadDepot
		Depot *d=Depot::loadDepot(n);
		if(d==NULL)
		{
			Widget::addRowToTable("Erreur lors du chargement de la configuration de fichier.",model,MSG_1);
			return NULL;
		}

		//On l'ajoute à la liste de dépots
		config->addDepot(d);
	}

	//On le retourne
	return config;
}





//Constructeur
ConfigurationFile::ConfigurationFile(QStandardItemModel *model) : QObject()
{
	//On fait les initialisations
	this->depots=new QList<Depot*>();
	this->depotsMutex=new QMutex(QMutex::Recursive);

	//La liste des détections en cours
	this->detectMediaList=new QList<Media*>();
	this->detectMediaListMutex=new QMutex(QMutex::Recursive);

	this->waitConditionDetect=NULL;
	this->isListen=false;
	this->model=model;

	//On s'occupe du watcher
	this->watcher=new QFileSystemWatcher(this);
	QObject::connect(watcher,SIGNAL(directoryChanged(QString)),this,SLOT(depotHasProbablyBeenRemoved()),Qt::QueuedConnection);
	Widget::addRowToTable("Le module de détections a bien été allouée",model,MSG_1);
}




//Pour détecter les dépots supprimés
void ConfigurationFile::depotHasProbablyBeenRemoved()
{
	bool found=false;
	//On parcours les dépots
	this->depotsMutex->lock();
	for(int i=0;i<depots->size();i++)
	{
		//On récupère un dépot
		Depot *d=depots->at(i);
		if(d->hasBeenRemoved()) //Si il a été supprimé
		{
			depots->removeAt(i); //On le supprime de la liste
			i--; //Attention pour le parcours de la boucle car un element est supprimé
			d->setListenning(false); //On l'empeche de détecter
			emit depotHasBeenRemoved(d); //On émet le signal
			found=true;
		}
	}
	this->depotsMutex->unlock();
}





bool ConfigurationFile::removeDepot(QString localPath)
{
	if(localPath.endsWith("/")) localPath=localPath.left(localPath.length()-1);
	bool found=false;
	this->depotsMutex->lock();
	for(int i=0;i<depots->size();i++)
	{
		//On récupère un dépot
		Depot *d=depots->at(i);
		if(d->getLocalPath()==localPath) //Si c'est le dépot à supprimer
		{
			depots->removeAt(i); //On le supprime de la liste
			i--; //Attention pour le parcours de la boucle car un element est supprimé
			d->setListenning(false); //On l'empeche de détecter
			emit depotHasBeenRemoved(d); //On émet le signal
			found=true;
		}
	}
	return found;
}





//Pour ajouter des dépots
bool ConfigurationFile::addDepot(Depot *depot)
{
	if(depot==NULL) return false;
	this->depotsMutex->lock();
	if(!depots->contains(depot))
	{
		for(int i=0;i<depots->size();i++)
		{
			if(depots->at(i)->getLocalPath().startsWith(depot->getLocalPath()+"/")
			|| depot->getLocalPath().startsWith(depots->at(i)->getLocalPath()+"/"))
			{
				depotsMutex->unlock();
				return false;
			}
		}
		depots->append(depot);
		watcher->addPath(Media::extractParentPath(depot->getLocalPath()));
		QObject::connect(depot,SIGNAL(detectChangement(Media*)),this,SLOT(putMediaDetection(Media*)),Qt::QueuedConnection);
		depot->setListenning(this->isListen);
	}
	this->depotsMutex->unlock();
	return true;
}




//Pour créer et ajouter des dépots
bool ConfigurationFile::addCreatingDepot(QString localPath,QString realPath,int revision,bool readOnly)
{
	Depot *depot=Depot::createDepot(localPath,realPath,revision,readOnly);
	return this->addDepot(depot);
}




//Pour charger et ajouter des dépots
bool ConfigurationFile::addLoadingDepot(QDomNode noeud)
{
	Depot *depot=Depot::loadDepot(noeud);
	return this->addDepot(depot);
}





//Pour savoir si les objet sont à l'écoute
bool ConfigurationFile::isListenning()
{
	return isListen;
}




//Retourne le code xml de la configuration
QDomElement ConfigurationFile::toXml(QDomDocument *document)
{
	//On crèe le noeud xml avec le nom ConfigurationFile
	QDomElement element=document->createElement("ConfigurationFile");

	depotsMutex->lock();
	//On parcours les dépots de la configuration
	for(int i=0;i<depots->size();i++)
	{
		//On récupére leurs noeuds xml et on les ajoute au noeud principal
		QDomElement e=depots->at(i)->toXml(document);
		element.appendChild(e);
	}
	depotsMutex->unlock();

	//On retourne le noeud
	return element;
}





//Recherche quel média a ce localPath
Media *ConfigurationFile::findMediaByLocalPath(QString localPath)
{
	QList<Depot*>::iterator i;

	depotsMutex->lock();
	//On parcours la liste des dépots
	for(i=depots->begin(); i!=depots->end(); i++)
	{
		//On recherche le localPath dans l'arborescence de ce dépot
		Media *find=(*i)->findMediaByLocalPath(localPath);
		if(find!=NULL)
		{
			depotsMutex->unlock();
			return find;
		}
	}
	depotsMutex->unlock();
	return NULL;
}





//Recherche quel média a ce realPath
Media *ConfigurationFile::findMediaByRealPath(QString realPath)
{
	QList<Depot*>::iterator i;

	depotsMutex->lock();
	//On parcours la liste des dépots
	for(i=depots->begin(); i!=depots->end(); i++)
	{
		//On recherche le realPath dans l'arborescence de ce dépot
		Media *find=(*i)->findMediaByRealPath(realPath);
		if(find!=NULL)
		{
			depotsMutex->unlock();
			return find;
		}
	}
	depotsMutex->unlock();
	return NULL;
}





//Pour récupérer le dépot auquel appartient un media
Depot *ConfigurationFile::getMediaDepot(Media *m)
{
	return getMediaDepot(m->getRealPath());
}




//Pour récupérer le dépot auquel appartient un media
Depot *ConfigurationFile::getMediaDepot(QString realPath)
{
	depotsMutex->lock();
	for(int i=0;i<depots->length();i++)
	{
		if(realPath.startsWith(depots->at(i)->getRealPath()+"/"))
		{
			depotsMutex->unlock();
			return depots->at(i);
		}
	}
	depotsMutex->unlock();
	return NULL;
}




//Pour récupérer les révisions des dépots
QHash<QString,int> ConfigurationFile::getDepotsRevisions()
{
	QHash<QString,int> hash;
	depotsMutex->lock();

	//On parcours et on récupère le realPath et la révision
	for(int i=0;i<depots->length();i++)
		hash.insert(depots->at(i)->getRealPath(),depots->at(i)->getRevision());
	depotsMutex->unlock();

	//On retourne le hash
	return hash;
}




//Pour récupérer la prochaine détection à traiter
Media *ConfigurationFile::getMediaDetection()
{
	//On verrouille d'abord l'accès aux autres threads
	detectMediaListMutex->lock();

	//On récupère le premier media de la liste (s'il existe)
	Media *m=NULL;
	if(detectMediaList->size()>0)
	{
		m=detectMediaList->first();
	}

	//On déverrouille
	detectMediaListMutex->unlock();

	//On retourne l'objet
	return m;
}



//Pour récupérer le nombre de détections en attente de traitement
int ConfigurationFile::getNumberMediaDetection()
{
	//On verrouille d'abord l'accès aux autres threads
	detectMediaListMutex->lock();

	int nb=detectMediaList->size();

	//On déverrouille
	detectMediaListMutex->unlock();

	//On retourne le nombre
	return nb;
}




//Pour supprimer une détection qui a été traitée
void ConfigurationFile::removeMediaDetection()
{
	//On verrouille l'objet
	detectMediaListMutex->lock();

	//On supprime le premier media de la file
	if(detectMediaList->size()>0)
	{
		Media *m=detectMediaList->first();
		//On enlève son etat de détection correspondant
		m->getDetectionState()->removeFirst();
		//On enleve de la liste
		detectMediaList->removeFirst();
	}

	//On déverouille l'objet
	detectMediaListMutex->unlock();
}




//Le slot qui répond aux détections de changements
void ConfigurationFile::putMediaDetection(Media *m)
{
	//On vérrouille les autres accès
	detectMediaListMutex->lock();

	//On recherche le dépot auquel appartient le media m
	Depot *d=this->getMediaDepot(m);

	//Si le dépot est en lecture seule, on laisse tomber
	if(!d || d->isReadOnly())
		return ;

	//S'il y a plusieurs détection sur un même fichier, alors la dernière prévaut sur les autres
	if(m->getDetectionState()->last()!=MediaIsCreating && detectMediaList->length()>1)
	{
		//On supprime les détections précédentes qui sont annulées par celle ci
		//sauf la première (i=0) qui est très probablement en cours de traitement par le thread HddInterface
		int k=0;
		QList<State> *list=m->getDetectionState();
		for(int i=0;i<list->size()-1;i++)
		{
			k=detectMediaList->indexOf(m,k);
			if(k==0)
			{
				k++;
				continue;
			}

			if(list->at(i)==MediaIsUpdating)
			{
				list->removeAt(i);i--;
				detectMediaList->removeAt(k);
			}
			else k++;
		}
	}

	//On ajoute le media
	detectMediaList->append(m);

	int index=0;
	//On recherche le rang du state à traiter sur le media m
	//Il est proportionnel au nombre de fois où m se retrouve dans le liste detectMediaList.
	//S'il il est 3 fois, alors index=3 et on traite le premier changement apparu (chronologiquement),
	//donc le dernier dans le liste detectionState du media (l'indice index-1 )
	for(int i=0;i<detectMediaList->length();i++)
		if(detectMediaList->at(i)==m) index++;

	//On affiche la détection dans le model
	Widget::addRowToTable(QString("Le ")+(m->isDirectory()?QString("repertoire "):QString("fichier "))+m->getLocalPath()+QString(" est passé à l'état ")+Media::stateToString(m->getDetectionState()->at(index-1)),model,MSG_2);

	//On demande un enregistrement de la config
	emit saveRequest();

	//On reveille le thread de hddInterface
	if(waitConditionDetect!=NULL)
		waitConditionDetect->wakeAll();

	//on déverrouille les accès
	detectMediaListMutex->unlock();
}




//Pour renseigner l'objet de reveil du thread de hddInterface
void ConfigurationFile::setWaitConditionDetection(QWaitCondition *waitConditionDetect)
{
	this->waitConditionDetect=waitConditionDetect;
}




//Permet de mettre les dépots à l'écoute de changements
void ConfigurationFile::setListenning(bool listen)
{
	QList<Depot*>::iterator i;

	//On l'affiche dans le model
	if(listen)
		Widget::addRowToTable("Le module de détection est mis à l'écoute de changements",model,MSG_2);
	else
		Widget::addRowToTable("Le module de détection a été désactivé",model,MSG_3);

	depotsMutex->lock();
	//On parcours tous les dépots et on appelle récursivement la fonction setListenning
	for(i=depots->begin(); i!=depots->end(); i++)
	{
		(*i)->setListenning(listen);
	}
	depotsMutex->unlock();

	//on met à jour l'attribut
	this->isListen=listen;
}





//Destructeur
ConfigurationFile::~ConfigurationFile()
{
	depotsMutex->lock();
	//On parcours la liste des dépots et on des supprime
	for(int i=0; i<depots->length(); i++)
	{
		delete depots->at(i);
	}

	//On vide la liste des dépots
	depots->clear();

	//On la supprime
	delete depots;
	depotsMutex->unlock();
}













//Permet de créer une config totale initiale
ConfigurationData *ConfigurationData::createConfigurationData(ConfigurationNetwork *configurationNetwork,
                         ConfigurationIdentification *configurationIdentification,
                         ConfigurationFile *configurationFile,QString savePath)
{
	//On vérifie qu'aucun des attributs n'est NULL
	if(configurationNetwork==NULL)
		return NULL;
	if(configurationIdentification==NULL)
		return NULL;
	if(configurationFile==NULL)
		return NULL;

	//On crèe l'objet et on le retourne
	ConfigurationData *config=new ConfigurationData(configurationNetwork,
				configurationIdentification,configurationFile,savePath);

	return config;
}





//Pour charger toutes les config à partir du fichier xml
ConfigurationData *ConfigurationData::loadConfigurationData(QString savePath, QString cle, QStandardItemModel *model)
{
	QFile file(savePath);
	if(!file.open(QIODevice::ReadOnly))     //On tente d'ouvrir le fichier de config
	{
		Widget::addRowToTable("Echec d'ouverture en lecture du fichier de configuration.",model,MSG_1);
		return NULL;
	}

	//Décrypte la config
	CipherTool cipherTool(cle);
	QCA::SecureArray *config = cipherTool.decrypteByteArray(file.readAll());

	QDomDocument document;
	if(!document.setContent(config->toByteArray()))         //On charge son contenu dans un objet QDomDocument
	{
		file.close();
		Widget::addRowToTable("Echec de format XML du fichier de configuration.",model,MSG_1);
		return NULL;
	}

	//On charge la liste des élèments fils du document
	QDomNodeList noeuds=document.documentElement().childNodes();

	//On initialise toutes les configuration à NULL
	ConfigurationNetwork *configurationNetwork                  = NULL;
	ConfigurationIdentification *configurationIdentification    = NULL;
	ConfigurationFile *configurationFile                        = NULL;

	//On parcours tous ces élèment
	for(int i=0;i<noeuds.count();i++)
	{
		QDomNode n=noeuds.at(i);
		QDomElement element=n.toElement();
		if(element.isNull()) continue;

		if(element.tagName()=="ConfigurationNetwork")   //On charge la conf réseau
		configurationNetwork=ConfigurationNetwork::loadConfigurationNetwork(n);

		else if(element.tagName()=="ConfigurationIdentification")   //On charge la conf d'identification
		configurationIdentification=ConfigurationIdentification::loadConfigurationIdentification(n);

		else if(element.tagName()=="ConfigurationFile")     //On charge la conf de l'arborescence des fichiers synchronisés
		configurationFile=ConfigurationFile::loadConfigurationFile(n,model);
	}

	file.close();

	//Vérifie qu'aucune des confs chargées n'est dans un sale état
	if(configurationNetwork==NULL || configurationIdentification==NULL || configurationFile==NULL)
	{
		delete configurationFile;
		delete configurationNetwork;
		delete configurationIdentification;
		Widget::addRowToTable("Fichier de configuration non valide.",model,MSG_1);
		return NULL;
	}

	//On cree l'objet et on le retourne
	ConfigurationData *configuration=new ConfigurationData(configurationNetwork,
					configurationIdentification,configurationFile,savePath);
	return configuration;
}




//Le constructeur
ConfigurationData::ConfigurationData(ConfigurationNetwork *configurationNetwork,
						ConfigurationIdentification *configurationIdentification,
						ConfigurationFile *configurationFile,QString savePath)
{
	//On ne fait qu'initialiser
	this->configurationNetwork=configurationNetwork;
	this->configurationIdentification=configurationIdentification;
	this->configurationFile=configurationFile;
	QObject::connect(configurationFile,SIGNAL(saveRequest()),this,SLOT(save()));
	this->savePath=savePath;
	this->cipherTool = new CipherTool(configurationIdentification->getPassword());
}


// Accesseurs et mutateurs de ConfigurationNetwork
void ConfigurationData::setConfigurationNetwork(ConfigurationNetwork *configurationNetwork)
{
	this->configurationNetwork=configurationNetwork;
}

ConfigurationNetwork *ConfigurationData::getConfigurationNetwork()
{
	return configurationNetwork;
}


// Accesseurs et mutateurs de ConfigurationIdentification
void ConfigurationData::setConfigurationIdentification(ConfigurationIdentification *configurationIdentification)
{
	this->configurationIdentification=configurationIdentification;
}

ConfigurationIdentification *ConfigurationData::getConfigurationIdentification()
{
	return configurationIdentification;
}


// Accesseurs et mutateurs de ConfigurationFile
void ConfigurationData::setConfigurationFile(ConfigurationFile *configurationFile)
{
	this->configurationFile=configurationFile;
}

ConfigurationFile *ConfigurationData::getConfigurationFile()
{
	return configurationFile;
}


//Accesseurs et mutateurs du savePath (Chemin absolu où sera enregistrée la config xml)
void ConfigurationData::setSavePath(QString savePath)
{
	this->savePath=savePath;
}

QString ConfigurationData::getSavePath()
{
	return savePath;
}

//Pour enregistrer la configuration dans un fichier xml
bool ConfigurationData::save(QString savePath)
{
	//En appelant cette fonction, il y a possibilité de choisir un chemin
	//différent de celui de this->savePath.
	if(savePath=="")
	{
		savePath=this->savePath;
		if(savePath=="") return false;
	}
	else this->savePath=savePath;

	//On empeche un autre thread de faire un save en même temps
	saveMutex.lock();

	//Crée le document xml et tous ses élèments
	QDomDocument document;
	QDomElement element=document.createElement("ConfigurationData");

		//Ajoute à l'élèment ConfData, les sous élèments confNetwork, confFile, et confIdent.
		QDomElement network=configurationNetwork->toXml(&document);
		element.appendChild(network);
		QDomElement identification=configurationIdentification->toXml(&document);
		element.appendChild(identification);
		QDomElement files=configurationFile->toXml(&document);
		element.appendChild(files);

	document.appendChild(element);

	QFile file(savePath);
	if(!file.open(QIODevice::WriteOnly))
	{
		saveMutex.unlock();
		return false;
	}

	//On crypte le fichier de config
	QCA::SecureArray tmp(document.toByteArray());
	QByteArray * config = cipherTool->encrypteByteArray(tmp);

	//On écrit ce doc xml dans un nouveau fichier (écrasé s'il existe déjà)
	file.write(*config);

	file.close();

	saveMutex.unlock();

	return true;
}




//Destructeur
ConfigurationData::~ConfigurationData()
{
	//on détruit toutes les autres configurations
	delete configurationNetwork;
	delete configurationIdentification;
	delete configurationFile;
}

