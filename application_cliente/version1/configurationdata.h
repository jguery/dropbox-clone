
#ifndef CONFIGURATIONNETWORK_H
#define CONFIGURATIONNETWORK_H

#include<QtCore>
#include<QtXml>

/*
Cette classe gère la configuration du réseau (addresse et port) du serveur à joindre).
*/
class ConfigurationNetwork
{
public:
	//Des méthodes statiques pour créer l'objet
	static ConfigurationNetwork *createConfigurationNetwork(QString address, int port);
	static ConfigurationNetwork *loadConfigurationNetwork(QDomNode noeud);
	//Des accesseurs et mutateurs
	QString getAddress();
	int getPort();
	void setAddress(QString address);
	void setPort(int port);
	//Méthode pour retourner le code xml correspondant.
	QDomElement toXml(QDomDocument *document);
private:
	ConfigurationNetwork(QString address,int port);
	QString address;
	int port;
};

#endif // CONFIGURATIONNETWORK_H













#ifndef CONFIGURATIONIDENTIFICATION_H
#define CONFIGURATIONIDENTIFICATION_H

#include<QtCore>
#include<QtXml>

/*
 Cette classe gère la configuration d'identification (pseudo et mot de passe de l'utilisateur)
*/

class ConfigurationIdentification
{
public:
	//Des fonctions statiques pour créer l'objet
	static ConfigurationIdentification *createConfigurationIdentification(QString pseudo, QString password);
	static ConfigurationIdentification *loadConfigurationIdentification(QDomNode noeud);
	//Des accesseurs et mutateurs
	QString getPseudo();
	QString getPassword();
	void setPseudo(QString pseudo);
	void setPassword(QString password);
	//Retourne le code xml de l'objet
	QDomElement toXml(QDomDocument *document);
private:
	ConfigurationIdentification(QString pseudo,QString password);
	QString pseudo;
	QString password;
};

#endif // CONFIGURATIONIDENTIFICATION_H











#ifndef CONFIGURATIONFILE_H
#define CONFIGURATIONFILE_H

#include "dir.h"
#include <QtCore>
#include <QtXml>

/*
 Cette classe gère la configuration des fichiers et repertoires synchronisés
*/

class ConfigurationFile
{
public:
	//Des fonctions statiques pour créer l'objet
	static ConfigurationFile *createConfigurationFile(QList<Dir*> *depots);
	static ConfigurationFile *loadConfigurationFile(QDomNode noeud);
	//Pour retourner le code xml de la configuration
	QDomElement toXml(QDomDocument *document);
	//Des fonctions pour rechercher dans l'arborescence des fichiers
	Media *findMediaByLocalPath(QString localPath);
	Media *findMediaByRealPath(QString realPath);
	Dir *findMediaParentByLocalPath(QString localPath);
	Dir *findMediaParentByRealPath(QString realPath);
	void setSignalListener(HddInterface *hddInterface);
	//Destructeur
	~ConfigurationFile();
private:
	ConfigurationFile(QList<Dir*> *depots);
	//La liste des dépots à surveiller
	QList<Dir*> *depots;
};

#endif // CONFIGURATIONFILE_H













#ifndef CONFIGURATIONDATA_H
#define CONFIGURATIONDATA_H


/*
 Cette classe regroupe toutes les configurations (network, identification et file)
*/

class ConfigurationData
{
public:
	static ConfigurationData *createConfigurationData(ConfigurationNetwork *configurationNetwork,ConfigurationIdentification *configurationIdentification,ConfigurationFile *configurationFile,QString savePath="");
	static ConfigurationData *loadConfigurationData(QString savePath);
	//Les accesseurs et mutateurs
	ConfigurationNetwork *getConfigurationNetwork();
	void setConfigurationNetwork(ConfigurationNetwork *configurationNetwork);
	//Les accesseurs et mutateurs
	ConfigurationIdentification *getConfigurationIdentification();
	void setConfigurationIdentification(ConfigurationIdentification *configurationIdentification);
	//Les accesseurs et mutateurs
	ConfigurationFile *getConfigurationFile();
	void setConfigurationFile(ConfigurationFile *configurationFile);
	//Les accesseurs et mutateurs
	QString getSavePath();
	void setSavePath(QString savePath);
	//Enregistre la config
	bool save(QString savePath="");
	//Destructeur
	~ConfigurationData();
private:
	//Constructeur
	ConfigurationData(ConfigurationNetwork *configurationNetwork,ConfigurationIdentification *configurationIdentification,ConfigurationFile *configurationFile,QString filePath);
	//Les configurations
	ConfigurationNetwork *configurationNetwork;
	ConfigurationFile *configurationFile;
	ConfigurationIdentification *configurationIdentification;
	//Le chemin du fichier ou doit être enregistrée la configuration
	QString savePath;
};

#endif // CONFIGURATIONDATA_H



