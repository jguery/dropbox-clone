#ifndef MESSAGES_H
#define MESSAGES_H

#include <QtCore>
#include <QtXml>

/*
 Cette classe entièrement statique, permet d'écrire et de lire les requetes xml
 Chaque fonction correspond à un type de requête entre le client et le serveur
*/

class Messages
{
public:
	//Requete pour avertir de la modification du contenu d'un fichier
	static QByteArray *createFileContentMessage(QString realPath, QByteArray content);

	//Avertir de la création d'un nouveau média
	static QByteArray *createMediaCreatedMessage(QString realPath);

	//Avertir de la suppression d'un média
	static QByteArray *createMediaRemovedMessage(QString realPath);

	//Pour s'identifier
	static QByteArray *createIdentificationMessage(QString pseudo,QString password);

	//Les messages de reponses (OK et ANNULER)
	static QByteArray *createValidationMessage();
	static QByteArray *createAnnulationMessage();

	//Pour lire et comprendre un message récu.
	static QHash<QString,QByteArray> *parseMessage(QByteArray *message);
};

#endif // MESSAGES_H

