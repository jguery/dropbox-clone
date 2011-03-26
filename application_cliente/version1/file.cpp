#include "file.h"

//Permet de créer un objet fichier en passant sont localPath et son realPath
//Le fichier doit exister sur le disque dur
File *File::createFile(QString localPath,QString realPath)
{
	if(localPath.isEmpty() || realPath.isEmpty()) return NULL;
	QByteArray *hash=File::hashFile(localPath);
	if(hash==NULL) return NULL;
	return new File(localPath,realPath,hash);
}

//Fonction statique permetant de hasher un fichier pour récupérer sa signature.
//Ici on peut implémenter l'algorithme de hash (md5 ou ...)
//Pour l'instant on retourne seulement le contenu du fichier comme hash, mais c'est bien sur à changer.
QByteArray *File::hashFile(QString path)
{
	QFile f(path);
	if(!f.open(QIODevice::ReadOnly)) return NULL;
	QByteArray *hash=new QByteArray(f.readAll());
	f.close();
	return hash;
}


//Permet de charger le fichier depuis un noeud xml. Attention il faut que la signature du fichier qui se trouve dans le xml
//soit égale à sa signature actuelle. Autrement il faut que le fichier n'est pas été modifié à l'insu du programme
File *File::loadFile(QDomNode noeud)
{
	if(noeud.toElement().tagName()!="file") return NULL;
	QString localPath=noeud.toElement().attribute("localPath","");
	QString realPath=noeud.toElement().attribute("realPath","");
	if(localPath.isEmpty() || realPath.isEmpty()) return NULL;
	QByteArray *hash=hashFile(localPath);
	if(hash==NULL) return NULL;
	QByteArray *h;
	if(noeud.childNodes().length()==0)
	{
		h=new QByteArray();
	}
	else if(!noeud.firstChild().isText())
	{
		delete h;delete hash;
		return NULL;
	}
	else
	{
		h=new QByteArray(noeud.firstChild().toText().data().toAscii());
	}
	if((*hash)!=(*h))
	{
		delete hash;delete h;
		return NULL;
	}
	delete h;
	return new File(localPath,realPath,hash);
}


//Détecte si oui ou non le fichier a été supprimé
bool File::hasBeenRemoved()
{
	QFile file(localPath);
	if(!file.exists()) return true;
	return false;
}

//Détecte si oui ou non le fichier a été modifié
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
	QDomElement element=document->createElement("file");
	element.setAttribute("localPath",localPath);
	element.setAttribute("realPath",realPath);
	element.appendChild(document->createTextNode(QString(*hash)));
	return element;
}


//Retourne si oui ou non le chemin passé en parametre correspond au fichier
Media *File::findMediaByLocalPath(QString localPath)
{
	if(this->localPath==localPath) return this;
	return NULL;
}


//Retourne si oui ou non le chemin realPath passé correspond au realPath du fichier
Media *File::findMediaByRealPath(QString realPath)
{
	if(this->realPath==realPath) return this;
	return NULL;
}


//Met à jour le hash du fichier
void File::updateContent()
{
	delete this->hash;
	this->hash=hashFile(localPath);
}


File::~File()
{
	delete hash;
}
