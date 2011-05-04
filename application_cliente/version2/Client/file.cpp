#include "file.h"
#include "dir.h"



//Permet de créer un objet fichier en passant son localPath et son realPath
//Le fichier doit exister sur le disque dur
//On crée enfaite une synchronisation, pas un vrai fichier
File *File::createFile(QString localPath,QString realPath,Dir *parent)
{
	if(localPath.isEmpty() || realPath.isEmpty())
		return NULL;

	//On vérifie que le fichier existe
	QFile f(localPath);
	if(!f.exists()) return NULL;

	//On considère que le contenu est vide
	QByteArray *hash=File::hashFile();

	//On peut créer le fichier
	return new File(localPath,realPath,parent,hash);
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

	//On récupère les attributs detectionState
	QString detectionStateString=noeud.toElement().attribute("detectionState","");
	QStringList listDetectionState=detectionStateString.split(",");

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
	File *f=new File(localPath,realPath,parent,hash);

	//On ajoute les états qui ont étés chargés depuis le noeud xml
	for(int i=0;i<listDetectionState.size();i++)
	{
		if(detectionStateString!="" && listDetectionState.at(i)!=Media::stateToString(MediaDefaultState))
		{
			f->getDetectionState()->append(Media::stateFromString(listDetectionState.at(i)));
			f->getParent()->getOldDetections()->append(f);
		}
	}
	return f;
}






//Fonction statique permetant de hasher un fichier pour récupérer sa signature.
//Ici on implemente l'algorithme de hash md5
QByteArray *File::hashFile(QString path)
{
	QByteArray content;
        //Si le chemin, est vide, on renvoi le hash d'une chaine vide (convention)
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

        //On déclare le hasher en Sha1 (autres alternatives MD4 ou MD5)
	QCryptographicHash hasher(QCryptographicHash::Sha1);
	hasher.addData(content);  //On met les données à hasher

	return new QByteArray(hasher.result()); //On hash et on retourne le resultat
}








//Détecte si oui ou non le fichier a été supprimé
bool File::hasBeenRemoved()
{
	QFile file(this->getLocalPath());
	if(!file.exists()) return true; //S'il n'existe pas, c'est qu'il a été supprimé
	return false;
}





//Détecte si oui ou non le fichier a été modifié
//Pour cela, on compare les signatures actuelle et dans le xml
bool File::hasBeenUpdated()
{
	QByteArray *h=File::hashFile(this->getLocalPath());
	if(h==NULL) return false;
	if(*h!=*hash)
	{
		delete h;
		return true;
	}
	delete h;
	return false;
}



//Pour récupérer le contenu
QByteArray File::getFileContent()
{
	QByteArray content;
	//on ouvre le fichier en lecture
	QFile f(this->getLocalPath());
	if(!f.open(QIODevice::ReadOnly))
		return content;
	content=f.readAll(); //On récupère le contenu du fichier
	f.close();
	return content;
}



//Pour écrire le contenu du fichier
bool File::putFileContent(QByteArray content)
{
	this->lock();

	//Récupère l'état d'écoute et stope l'écoute du dossier parent
	bool listenState=getParent()->isListenning();
	getParent()->setListenning(false);

	QFile f(this->getLocalPath());
	//on ouvre le fichier en écriture
	if(!f.open(QIODevice::WriteOnly))
	{
		this->unlock();
		this->getParent()->setListenning(listenState);
		return false;
	}
	f.write(content);
	f.close();
	updateHash();
	this->unlock();
	this->getParent()->setListenning(listenState);
	return true;
}




//Retourne false car ce n'est pas un repertoire ;)
bool File::isDirectory()
{
	return false;
}





//Constructeur qui ne fait qu'initialiser
File::File(QString localPath,QString realPath,Dir *parent,QByteArray *hash): Media(localPath,realPath,parent)
{
	this->hash=hash;
}






//Retourne le code xml correspondant au fichier pour sauvegarde ultérieure.
QDomElement File::toXml(QDomDocument *document)
{
	//On crèe le noeud xml du fichier avec le nom "file"
	QDomElement element=document->createElement("file");

	//On initialise ses attributs localPath et realPath
	element.setAttribute("localPath",this->getLocalPath());
	element.setAttribute("realPath",this->getRealPath());

	//On écrit ses attributs detectionState, revision et readOnly
	QStringList listDetectionState;
	for(int i=0;i<this->getDetectionState()->length();i++)
	{
		if(this->getDetectionState()->at(i)!=MediaDefaultState)
		listDetectionState.append(Media::stateToString(this->getDetectionState()->at(i)));
	}
	element.setAttribute("detectionState",listDetectionState.join(","));

	//On ajoute sa signature dans le noeud xml
	element.appendChild(document->createTextNode(QString(*hash)));

	//On retourne le noeud xml
	return element;
}






//Retourne this si le chemin localPath passé correspond au localPath du fichier
//Sinon retourne NULL
Media *File::findMediaByLocalPath(QString localPath)
{
	if(this->getLocalPath()==localPath)
		return this;
	return NULL;
}





//Retourne this si le chemin realPath passé correspond au realPath du fichier
//Sinon retourne NULL
Media *File::findMediaByRealPath(QString realPath)
{
	if(this->getRealPath()==realPath)
		return this;
	return NULL;
}





//Met à jour le hash du fichier
void File::updateHash()
{
	this->lock();
	//On supprime l'ancien hash
	delete this->hash;
	//On récalcule le nouveau hash
	this->hash=hashFile(this->getLocalPath());
	if(this->hash==NULL) this->hash=new QByteArray();
	this->unlock();
}






//Le destructeur
File::~File()
{
	delete hash;
}

