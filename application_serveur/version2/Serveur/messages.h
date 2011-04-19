#ifndef MESSAGES_H
#define MESSAGES_H

#include <QtCore>
#include <QtXml>




/*
 Classe abstraite qui représente un message
*/
class Message
{
public:
	virtual bool isRequest()=0;
};





/*
 Enumerations pour les différents types de requêtes
*/
enum RequestEnum
{
	CREATE_FILE_INFO,
	UPDATE_FILE_INFO,
	REMOVE_FILE_INFO,

	IDENTIFICATION,
};

/*
 Classe simple implémentant une requete.
*/
class Request: public Message
{
public:
	Request();
	void setType(RequestEnum type);
	RequestEnum getType();
	QHash<QString,QByteArray> *getParameters();
	QByteArray *toXml();
	bool isRequest();
private:
	RequestEnum type;
	QHash<QString,QByteArray> *parameters;
};







/*
 Enumerations pour les différents types de réponses
*/
enum ResponseEnum
{
	//Pour l'envoi de fichiers
	ACCEPT_FILE_INFO,
	REJECT_FILE_INFO_FOR_PARAMETERS,
	REJECT_FILE_INFO_FOR_IDENTIFICATION,
	REJECT_FILE_INFO_FOR_RIGHT,
	REJECT_FILE_INFO_FOR_CONFLICT,

	//Les identifications
	ACCEPT_IDENTIFICATION,
	REJECT_IDENTIFICATION_FOR_PSEUDO,
	REJECT_IDENTIFICATION_FOR_PASSWORD,
	REJECT_IDENTIFICATION_FOR_BLOCK,

	//Les erreurs de connexions
	NOT_CONNECT,
	NOT_IDENTIFICATE,
	NOT_PARAMETERS,
	NOT_TIMEOUT,
	NOT_SEND
};

/*
 Classe simple implémentant une réponse.
*/
class Response: public Message
{
public:
	Response();
	void setType(ResponseEnum type);
	ResponseEnum getType();
	QByteArray *toXml();
	bool isRequest();
private:
	ResponseEnum type;
};









/*
 Cette classe entièrement statique, permet d'écrire et de lire les requetes xml
 Chaque fonction correspond à un type de requête entre le client et le serveur
*/

class Messages
{

	friend class Request;
	friend class Response;

private:
	//Requete pour avertir de la modification du contenu d'un fichier
	static QByteArray *createMediaUpdatedMessage(QString realPath, QByteArray content);

	//Avertir de la création d'un nouveau média
	static QByteArray *createMediaCreatedMessage(QString realPath, bool isDirectory);

	//Avertir de la suppression d'un média
	static QByteArray *createMediaRemovedMessage(QString realPath);

	//Pour s'identifier
	static QByteArray *createIdentificationMessage(QString pseudo,QString password);

	//Pour répondre
	static QByteArray *createResponseMessage(ResponseEnum type);

public:
	//Pour lire et comprendre un message récu.
	static Message *parseMessage(QByteArray *message);
};

#endif // MESSAGES_H

