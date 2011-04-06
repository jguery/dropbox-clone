#ifndef FILE_H
#define FILE_H

#include "media.h"
#include <QtCore/QFile>
#include <QtCore/QCryptographicHash>

/*
   Cette classe représente un fichier physique.
   Elle hérite de la classe Media
   On peut créer un fichier ou le charger à partir d'un noeud xml
   Un fichier possède une signature (pour détecter si il y a eu modification)
*/

class File : public Media
{
public:
	//Pour créer le fichier on ne passe pas directement par le constructeur, mais plutôt par les fonctions createFile et loadFile
	static File *createFile(QString localPath,QString realPath,Dir *parent,State state,int revision,bool readOnly);
	static File *loadFile(QDomNode noeud,Dir *parent);

	//Ensuite on ré-implemente les fonctions virtuelles de la classe Media
	bool isDirectory();
	QDomElement toXml(QDomDocument *document);
	Media *findMediaByLocalPath(QString localPath);
	Media *findMediaByRealPath(QString realPath);

	//Une méthode statique pour hasher un fichier et récupérer sa signature.
	//Si le chemin est vide, elle renvoie le hash d'une chaine vide.
	static QByteArray *hashFile(QString path="");

	//Des méthodes pour savoir si le fichier a été supprimé ou modifié
	bool hasBeenRemoved();
	bool hasBeenUpdated();

	//Une méthode pour mettre à jour sa signature (par exemple après une modif)
	void updateHash();

	//Destructeur
	virtual ~File();
private:
	//Un constructeur privé pour qu'on ne puisse pas créer des fichiers non valides. (passons plutôt par createFile ou loadFile)
	File(QString localPath,QString realPath,Dir *parent,QByteArray *hash,State state,int revision,bool readOnly);

	//Contient la signature (le hash) du fichier
	QByteArray *hash;
};

#endif // FILE_H
