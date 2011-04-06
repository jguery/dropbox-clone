#include "file.h"




//Permet de créer un objet fichier en passant son localPath et son realPath
//Le fichier doit exister sur le disque dur
//On crée enfaite une synchronisation, pas un vrai fichier
File *File::createFile(QString localPath,QString realPath,Dir *parent,State state,int revision,bool readOnly)
{
	if(localPath.isEmpty() || realPath.isEmpty())
		return NULL;

	//On vérifie que le fichier existe
	QFile f(localPath);
	if(!f.exists()) return NULL;

	//On considère que le contenu est vide
	QByteArray *hash=File::hashFile();

	//On peut créer le fichier
	return new File(localPath,realPath,parent,hash,state,revision,readOnly);
}







//Permet de charger le fichier depuis un noeud xml.
File *File::loadFile(QDomNode noeud,Dir *parent)
{
	//On vérifie que le nom du noeud xml est bien "file"
	if(noeud.toElement().tagName()!="file")
		return NULL;

	//On récupère les attributs localPath et realPath du noeud xml.
	QString localPath=noeud.toElement().attribute("localPath","");
	QString realPath=noeud.toElement().attribute("realPath","");

	if(localPath.isEmpty() || realPath.isEmpty())
		return NULL;

	//On récupère les attributs state, révision et readOnly du noeud xml.
	QString stateString=noeud.toElement().attribute("state","");
	QString revisionString=noeud.toElement().attribute("revision","");
	QString readOnlyString=noeud.toElement().attribute("readOnly","");

	//On convertit ces attributs en state, int et bool
	State state;int revision;bool readOnly;bool ok;
	state=Media::stateFromString(stateString);
	revision=revisionString.toInt(&ok); if(!ok) revision=0;
	readOnly=readOnlyString=="true"?true:false;

	//Récupère le hash du fichier, contenu dans le xml
	//C'est le premier et le seul fils du noeud représentant le fichier
	QByteArray *hash;
	if(noeud.childNodes().length()==0) //Le hash est vide ( = "" )
	{
		hash=new QByteArray();
	}
	else    //Récupère le texte contenu dans le fils, c'est le hash
	{
		hash=new QByteArray(noeud.firstChild().toText().data().toAscii());
	}

	 //On peut maintenant créer l'objet
	return new File(localPath,realPath,parent,hash,state,revision,readOnly);
}






//Fonction statique permetant de hasher un fichier pour récupérer sa signature.
//Ici on implemente l'algorithme de hash md5
QByteArray *File::hashFile(QString path)
{
	QByteArray content;
	//Si le chemin, est vide, on renvoi le hash d'une chaine vide
	if(path=="")
	{
		content="";
	}
	else
	{
		//Sinon on ouvre le fichier en lecture
		QFile f(path);
		if(!f.open(QIODevice::ReadOnly)) //Si l'ouverture échoue, on renvoie une signature NULL (non valide)
			return NULL;
		content=f.readAll(); //On récupère le contenu du fichier
		f.close();
	}

	//On déclare le hasher en MD5 (autres alternatives MD4 ou SHA1
	QCryptographicHash hasher(QCryptographicHash::Sha1);
	hasher.addData(content);  //On met les données à hasher

	return new QByteArray(hasher.result().toBase64()); //On hash, on met le hash en base64 et on retourne le resultat
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
File::File(QString localPath,QString realPath,Dir *parent,QByteArray *hash,State state,int revision,bool readOnly): Media(localPath,realPath,parent,state,revision,readOnly)
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

	//On écrit ses attributs state, revision et readOnly
	element.setAttribute("state",Media::stateToString(state));
	element.setAttribute("revision",QString::number(revision));
	element.setAttribute("readOnly",readOnly?"true":"false");

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
void File::updateHash()
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
