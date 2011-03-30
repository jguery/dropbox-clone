#include "configurationdata.h"


/*

 Ce fichier contient les implémentations de trois classes:
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
        if(!ok)
            return NULL;

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
ConfigurationFile *ConfigurationFile::createConfigurationFile(QList<Dir*> *depots)
{
	//La liste de dépots passée ne doit être ni NULLe, ni vide
	  if(depots==NULL)        return NULL;
	  if(depots->length()==0)      return NULL;

	  //On crèe la configuration et on la retourne
	ConfigurationFile *config=new ConfigurationFile(depots);
	return config;
}


//Pour charger la configuration des répertoires et fichiers depuis un noeud xml
//Typiquement, appelé dés le lancement du client, en chargant la config
//préalablement enregistrée dans un config.xml, grâce à ConfigurationData::save
ConfigurationFile *ConfigurationFile::loadConfigurationFile(QDomNode noeud)
{
	//On alloue la liste des dépots dans laquelle seront chargées les dépots lues
	QList<Dir*> *depots=new QList<Dir*>();

	//On vérifie que le nom du noeud est bien ConfigurationData
	QDomElement element=noeud.toElement();
        if(element.tagName()!="ConfigurationFile")
            return NULL;

        //Parcours l'ensemble des éléments fils de l'élement "COnfigurationFile"
        //Ce sont donc des fichiers et des dossiers (toute l'arborescence  à synchroniser enfaite)
	QDomNodeList list=noeud.childNodes();
	for(unsigned int i=0;i<list.length();i++)
	{
		QDomNode n=list.at(i);

		//On charge le noeud avec la méthode statique loadDir
		Dir *d=Dir::loadDir(n);
                if(d==NULL)
                    return NULL;

		 //On l'ajoute à la liste de dépots
		depots->append(d);
	}

	//On crèe l'objet et on le retourne
	ConfigurationFile *config=new ConfigurationFile(depots);
	return config;
}



//Constructeur
ConfigurationFile::ConfigurationFile(QList<Dir*> *depots)
{
	this->depots=depots;
}



//Retourne le code xml de la configuration
QDomElement ConfigurationFile::toXml(QDomDocument *document)
{
	//On crèe le noeud xml avec le nom ConfigurationFile
	QDomElement element=document->createElement("ConfigurationFile");

	//On parcours les dépots de la configuration
	for(int i=0;i<depots->size();i++)
	{
		//On récupére leurs noeuds xml et on les ajoute au noeud principal
		QDomElement e=depots->at(i)->toXml(document);
		element.appendChild(e);
	}

	//On retourne le noeud
	return element;
}


//Recherche quel média a ce localPath
Media *ConfigurationFile::findMediaByLocalPath(QString localPath)
{
        QList<Dir*>::iterator i;

	//On parcours la liste des dépots
	  for(i=depots->begin(); i!=depots->end(); i++)
	{
		  //On recherche le localPath dans l'arborescence de ce dépot
                Media *find=(*i)->findMediaByLocalPath(localPath);
                if(find!=NULL)
                    return find;
	}
	return NULL;
}

//Recherche quel média a ce realPath
Media *ConfigurationFile::findMediaByRealPath(QString realPath)
{
        QList<Dir*>::iterator i;

	  //On parcours la liste des dépots
        for(i=depots->begin(); i!=depots->end(); i++)
	{
		  //On recherche le realPath dans l'arborescence de ce dépot
                Media *find=(*i)->findMediaByRealPath(realPath);
                if(find!=NULL)
                    return find;
	}
	return NULL;
}


//Recherche quel est le repertoire parent du média qui a ce localPath
Dir *ConfigurationFile::findMediaParentByLocalPath(QString localPath)
{

        QList<Dir*>::iterator i;

	  //On parcours la liste des dépots
        for(i=depots->begin(); i!=depots->end(); i++)
	{
		  //On recherche le parent du localPath dans l'arborescence de ce dépot
                Dir *find=(*i)->findMediaParentByLocalPath(localPath);
		if(find!=NULL) return find;
	}
	return NULL;
}


//Recherche quel est le repertoire du média qui a ce realPath
Dir *ConfigurationFile::findMediaParentByRealPath(QString realPath)
{
        QList<Dir*>::iterator i;

	  //On parcours la liste des dépots
        for(i=depots->begin(); i!=depots->end(); i++)
	{
		  //On recherche le parent du realPath dans l'arborescence de ce dépot
                Dir *find=(*i)->findMediaParentByRealPath(realPath);
		if(find!=NULL) return find;
	}
	return NULL;
}



//Permet de fixer l'objet à prevenir en cas de modif
void ConfigurationFile::setSignalListener(HddInterface *hddInterface)
{
        QList<Dir*>::iterator i;

	  //On parcours tous les dépots et on appelle récursivement la fonction setSignalListener
        for(i=depots->begin(); i!=depots->end(); i++)
	{
                (*i)->setSignalListener(hddInterface);
	}
}

//Destructeur
ConfigurationFile::~ConfigurationFile()
{
     //On parcours la liste des dépots et on des supprime
	for(int i=0; i<depots->length(); i++)
	{
		delete depots->at(i);
	}

	//On vide la liste des dépots
	depots->clear();

	//On la supprime
	delete depots;
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
ConfigurationData *ConfigurationData::loadConfigurationData(QString savePath)
{
	QFile file(savePath);
        if(!file.open(QIODevice::ReadOnly))     //On tente d'ouvrir le fichier de config
            return NULL;

	QDomDocument document;
        if(!document.setContent(&file))         //On charge son contenu dans un objet QDomDocument
	{
		file.close();
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
			configurationFile=ConfigurationFile::loadConfigurationFile(n);
	}

	file.close();

	  //Vérifie qu'aucune des confs chargées n'est dans un sale état
	if(configurationNetwork==NULL || configurationIdentification==NULL || configurationFile==NULL)
	{
		delete configurationFile;
		delete configurationNetwork;
		delete configurationIdentification;
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
	this->savePath=savePath;
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
        //différent de celui de this-savePath.
        if(savePath=="")
	{
		savePath=this->savePath;
		if(savePath=="") return false;
	}
	else this->savePath=savePath;

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
            return false;

        //On écrit ce doc xml dans un nouveau fichier (écrasé s'il existe déjà)
	file.write(document.toByteArray());

	file.close();

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

