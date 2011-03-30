#include "file.h"

//Permet de créer un objet fichier en passant son localPath et son realPath
//Le fichier doit exister sur le disque dur
//On crée enfaite une synchronisation, pas un vrai fichier
File *File::createFile(QString localPath,QString realPath)
{
        if(localPath.isEmpty() || realPath.isEmpty())
            return NULL;

        //Récupère la signature (le hash) du fichier
	QByteArray *hash=File::hashFile(localPath);
	  if(hash==NULL) //Si la signature n'est pas valide on ne crèe pas le fichier
            return NULL;

	//On peut créer le fichier
	return new File(localPath,realPath,hash);
}

//Fonction statique permetant de hasher un fichier pour récupérer sa signature.
//Ici on peut implémenter l'algorithme de hash (md5 ou ...)
//Pour l'instant on retourne seulement le contenu du fichier comme hash, mais c'est bien sur à changer.
QByteArray *File::hashFile(QString path)
{
	QFile f(path);
	  if(!f.open(QIODevice::ReadOnly)) //Si l'ouverture échoue, on renvoie une signature NULL (non valide)
            return NULL;

	QByteArray *hash=new QByteArray(f.readAll());
	f.close();

	return hash;
}


//Permet de charger le fichier depuis un noeud xml. Attention il faut que la signature du fichier qui se trouve dans le xml
//soit égale à sa signature actuelle. Autrement il faut que le fichier n'ait pas été modifié à l'insu du programme
File *File::loadFile(QDomNode noeud)
{
	//On vérifie que le nom du noeud xml est bien "file"
        if(noeud.toElement().tagName()!="file")
            return NULL;

	  //On récupère les attributs localPath et realPath du noeud xml.
	QString localPath=noeud.toElement().attribute("localPath","");
	QString realPath=noeud.toElement().attribute("realPath","");
        if(localPath.isEmpty() || realPath.isEmpty())
            return NULL;

        //Récupère le hash du fichier (sa valeur actuelle)
	QByteArray *hash=hashFile(localPath);
        if(hash==NULL)
            return NULL;

	QByteArray *h;
        //Récupère le hash du fichier, contenu dans le xml
        //C'est le premier et le seul fils du noeud représentant le fichier
        if(noeud.childNodes().length()==0) //Le hash est vide ( = "" )
	{
		h=new QByteArray();
	}
        else if(!noeud.firstChild().isText())   //Si ce fils n'est pas du texte...
	{
                delete h;  delete hash;
                return NULL;    //On annule tout
	}
        else    //Récupère le texte contenu dans le fils, c'est le hash
	{
		h=new QByteArray(noeud.firstChild().toText().data().toAscii());
	}

        //Vérifie que le hash actuel et celui contenu dans le xml sont égaux
	if((*hash)!=(*h))
	{
		delete hash;delete h;
		return NULL;
	}
	delete h;

        //C'est bien égal, on crée l'objet
	return new File(localPath,realPath,hash);
}


//Détecte si oui ou non le fichier a été supprimé
bool File::hasBeenRemoved()
{
	QFile file(localPath);
	if(!file.exists()) return true; //S'il n'existe pas, c'est qu'il a été supprimé
	return false;
}

//Détecte si oui ou non le fichier a été modifié
//Pour cela, on compare les signatures actuelle et dans le xml
bool File::hasBeenUpdated()
{
	QByteArray *h=File::hashFile(localPath);
	if(*h!=*hash)
	{
		delete h;
		return true;
	}
	delete h;
	return false;
}

//Retourne false car ce n'est pas un repertoire ;)
bool File::isDirectory()
{
	return false;
}


//Constructeur qui ne fait qu'initialiser
File::File(QString localPath,QString realPath,QByteArray *hash): Media(localPath,realPath)
{
	this->hash=hash;
}


//Retourne le code xml correspondant au fichier pour sauvegarde ultérieure.
QDomElement File::toXml(QDomDocument *document)
{
	//On crèe le noeud xml du fichier avec le nom "file"
	QDomElement element=document->createElement("file");

	//On initialise ses attributs localPath et realPath
	element.setAttribute("localPath",localPath);
	element.setAttribute("realPath",realPath);

	//On ajoute sa signature dans le noeud xml
	element.appendChild(document->createTextNode(QString(*hash)));

	//On retourne le noeud xml
	return element;
}


//Retourne this si le chemin localPath passé correspond au localPath du fichier
//Sinon retourne NULL
Media *File::findMediaByLocalPath(QString localPath)
{
        if(this->localPath==localPath)
            return this;

	return NULL;
}


//Retourne this si le chemin realPath passé correspond au realPath du fichier
//Sinon retourne NULL
Media *File::findMediaByRealPath(QString realPath)
{
        if(this->realPath==realPath)
            return this;
	return NULL;
}


//Met à jour le hash du fichier
void File::updateContent()
{
	//On supprime l'ancien hash
	delete this->hash;

	//On récalcule le nouveau hash
	this->hash=hashFile(localPath);
}


//Le destructeur
File::~File()
{
	delete hash;
}
