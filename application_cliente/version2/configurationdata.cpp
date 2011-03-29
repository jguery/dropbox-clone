#include "configurationdata.h"


//Fonction statique pour créer une config initiale
ConfigurationNetwork *ConfigurationNetwork::createConfigurationNetwork(QString address, int port)
{
        if(address.isEmpty() || port<0 || port>=65536)
            return NULL;
	ConfigurationNetwork *configurationNetwork=new ConfigurationNetwork(address,port);
	return configurationNetwork;
}

//Permet de charger l'objet à partir d'un noeud xml
ConfigurationNetwork *ConfigurationNetwork::loadConfigurationNetwork(QDomNode noeud)
{
	QDomElement element=noeud.toElement();
        if(element.tagName()!="ConfigurationNetwork")
            return NULL;

	QString address=element.attribute("address","");
	QString port=element.attribute("port","");
        if(address.isEmpty() || port.isEmpty())
            return NULL;

	bool ok=false;int p=port.toInt(&ok);
        if(!ok)
            return NULL;

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


//Les accesseurs et mutateurs

QString ConfigurationNetwork::getAddress()
{
	return address;
}

void ConfigurationNetwork::setAddress(QString address)
{
	this->address=address;
}

//les accesseurs et mutateurs

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
	QDomElement element=document->createElement("ConfigurationNetwork");
	element.setAttribute("address",address);
	element.setAttribute("port",QString::number(port));

	return element;
}









//Pour créer une configuration d'identification initiale
ConfigurationIdentification *ConfigurationIdentification::createConfigurationIdentification(QString pseudo, QString password)
{
        if(pseudo.isEmpty() || password.isEmpty())
            return NULL;

	ConfigurationIdentification *configurationIdentification=new ConfigurationIdentification(pseudo,password);
	return configurationIdentification;
}


//Pour charger l'objet à partir d'un noeud xml
ConfigurationIdentification *ConfigurationIdentification::loadConfigurationIdentification(QDomNode noeud)
{
	QDomElement element=noeud.toElement();
        if(element.tagName()!="ConfigurationIdentification")
            return NULL;

	QString pseudo=element.attribute("pseudo","");
	QString password=element.attribute("password","");
        if(pseudo.isEmpty() || password.isEmpty())
            return NULL;

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

//Les accesseurs et mutateurs

QString ConfigurationIdentification::getPseudo()
{
	return pseudo;
}

void ConfigurationIdentification::setPseudo(QString pseudo)
{
	this->pseudo=pseudo;
}

//les accesseurs et mutateurs

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
	QDomElement element=document->createElement("ConfigurationIdentification");
	element.setAttribute("pseudo",pseudo);
	element.setAttribute("password",password);

	return element;
}










//Une fonction statique pour créer une configuration initiale
ConfigurationFile *ConfigurationFile::createConfigurationFile(QList<Dir*> *depots)
{
        if(depots==NULL)
            return NULL;
        if(depots->length()==0)
            return NULL;

	ConfigurationFile *config=new ConfigurationFile(depots);
	return config;
}


//Pour charger la configuration des répertoires et fichiers depuis un noeud xml
//Typiquement, appelé dés le lancement du client, en chargant la config
//préalablement enregistrée dans un config.xml, grâce à ConfigurationData::save
ConfigurationFile *ConfigurationFile::loadConfigurationFile(QDomNode noeud)
{
	QList<Dir*> *depots=new QList<Dir*>();
	QDomElement element=noeud.toElement();
        if(element.tagName()!="ConfigurationFile")
            return NULL;

        //Parcours l'ensemble des éléments fils de l'élement "COnfigurationFile"
        //Ce sont donc des fichiers et des dossiers (toute l'arborescence  à synchroniser enfaite)
	QDomNodeList list=noeud.childNodes();
	for(unsigned int i=0;i<list.length();i++)
	{
		QDomNode n=list.at(i);
		Dir *d=Dir::loadDir(n);
                if(d==NULL)
                    return NULL;
		depots->append(d);
	}
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
	QDomElement element=document->createElement("ConfigurationFile");
	for(int i=0;i<depots->size();i++)
	{
		QDomElement e=depots->at(i)->toXml(document);
		element.appendChild(e);
	}
	return element;
}


//Recherche quel média a ce localPath
Media *ConfigurationFile::findMediaByLocalPath(QString localPath)
{
        QList<Dir*>::iterator i;
        for(i=depots->begin(); i!=depots->end(); i++)
	{
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
        for(i=depots->begin(); i!=depots->end(); i++)
	{
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
        for(i=depots->begin(); i!=depots->end(); i++)
	{
                Dir *find=(*i)->findMediaParentByLocalPath(localPath);
		if(find!=NULL) return find;
	}
	return NULL;
}


//Recherche quel est le repertoire du média qui a ce realPath
Dir *ConfigurationFile::findMediaParentByRealPath(QString realPath)
{
        QList<Dir*>::iterator i;
        for(i=depots->begin(); i!=depots->end(); i++)
	{
                Dir *find=(*i)->findMediaParentByRealPath(realPath);
		if(find!=NULL) return find;
	}
	return NULL;
}


//Permet de fixer l'objet à prevenir en cas de modif
void ConfigurationFile::setSignalListener(HddInterface *hddInterface)
{
        QList<Dir*>::iterator i;
        for(i=depots->begin(); i!=depots->end(); i++)
	{
                (*i)->setSignalListener(hddInterface);
	}
}

//Destructeur
ConfigurationFile::~ConfigurationFile()
{
        QList<Dir*>::iterator i;
        for(i=depots->begin(); i!=depots->end(); i++)
	{
                delete *i;
	}
	depots->clear();
	delete depots;
}


















//Permet de créer une config totale initiale
ConfigurationData *ConfigurationData::createConfigurationData(ConfigurationNetwork *configurationNetwork,
                         ConfigurationIdentification *configurationIdentification,
                         ConfigurationFile *configurationFile,QString savePath)
{
        if(configurationNetwork==NULL)
            return NULL;
        if(configurationIdentification==NULL)
            return NULL;
        if(configurationFile==NULL)
            return NULL;
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

        //Vérifie qu'aucune des confs chargées n'est dans un sal état
	if(configurationNetwork==NULL || configurationIdentification==NULL || configurationFile==NULL)
	{
		delete configurationFile;
		delete configurationNetwork;
		delete configurationIdentification;
		return NULL;
	}
        ConfigurationData *configuration=new ConfigurationData(configurationNetwork,
                              configurationIdentification,configurationFile,savePath);
	return configuration;
}


//Le constructeur
ConfigurationData::ConfigurationData(ConfigurationNetwork *configurationNetwork,
                                     ConfigurationIdentification *configurationIdentification,
                                     ConfigurationFile *configurationFile,QString savePath)
{
	this->configurationNetwork=configurationNetwork;
	this->configurationIdentification=configurationIdentification;
	this->configurationFile=configurationFile;
	this->savePath=savePath;
}

// Accesseurs et mutateurs
void ConfigurationData::setConfigurationNetwork(ConfigurationNetwork *configurationNetwork)
{
	this->configurationNetwork=configurationNetwork;
}

ConfigurationNetwork *ConfigurationData::getConfigurationNetwork()
{
	return configurationNetwork;
}

// Accesseurs et mutateurs
void ConfigurationData::setConfigurationIdentification(ConfigurationIdentification *configurationIdentification)
{
	this->configurationIdentification=configurationIdentification;
}

ConfigurationIdentification *ConfigurationData::getConfigurationIdentification()
{
	return configurationIdentification;
}

// Accesseurs et mutateurs
void ConfigurationData::setConfigurationFile(ConfigurationFile *configurationFile)
{
	this->configurationFile=configurationFile;
}

ConfigurationFile *ConfigurationData::getConfigurationFile()
{
	return configurationFile;
}

//Accesseurs et mutateurs
//Chemin absolu où sera enregistrée la config xml
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

                //Ajoute à l'élèment ConfDate, les sous élèments confNetwork, confFile, et confIdent.
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
	delete configurationNetwork;
	delete configurationIdentification;
	delete configurationFile;
}

