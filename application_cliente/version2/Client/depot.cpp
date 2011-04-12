#include "depot.h"



//Pour créer un dépot à partir de son localPath et de son realPath
//Les paramètres localPath et realPath ne doivent pas être vides
//Le repertoire doit exister à l'adresse localPath
//On retourne un repertoire vide, même si celui d'origine contient des fichiers

Depot *Depot::createDepot(QString localPath,QString realPath,int revision,bool readOnly)
{
	//Les paramètres localPath et realPath ne doivent pas être vides
	if(localPath.isEmpty() || realPath.isEmpty()) return NULL;

	//Par convention les noms de repertoire ne doivent pas se terminer pas "/"
	if(localPath.endsWith("/")) localPath=localPath.left(localPath.length()-1);
	if(realPath.endsWith("/")) realPath=realPath.left(realPath.length()-1);

	//On vérifie si le repertoire existe sur le disque
	QDir localDir(localPath);
	if(!localDir.exists()) return NULL;

	//On crèe l'element
	Depot *depot=new Depot(localPath,realPath,revision,readOnly);

	//On fait alors l'allocation d'un objet Depot
	return depot;
}




//Permet de charger le dépot à partir d'un noeud xml.
Depot *Depot::loadDepot(QDomNode noeud)
{
	//On vérifie que le nom du noeud xml est bien "dir"
	if(noeud.toElement().tagName()!="dir")
		return NULL;

	//On récupère le localPath et realPath du noeud xml et on vérifie qu'ils ne sont pas vides
	QString localPath=noeud.toElement().attribute("localPath","");
	QString realPath=noeud.toElement().attribute("realPath","");
	if(localPath.isEmpty() || realPath.isEmpty())
		return NULL;

	//S'ils se terminent pas "/" on normalise en enlevant le slash (simple convention)
	if(localPath.endsWith("/")) localPath=localPath.left(localPath.length()-1);
	if(realPath.endsWith("/")) realPath=realPath.left(realPath.length()-1);

	//On récupère les attributs révision et readOnly du noeud xml.
	QString revisionString=noeud.toElement().attribute("revision","");
	QString readOnlyString=noeud.toElement().attribute("readOnly","");

	//On convertit ces attributs en int et bool
	int revision;bool readOnly;bool ok;
	revision=revisionString.toInt(&ok); if(!ok) revision=0;
	readOnly=readOnlyString=="true"?true:false;

	//On vérifie que le dépot existe bien
	QDir dir(localPath);
	if(!dir.exists()) return NULL;

	//On stocke notre dépot dans un objet Dépot qui contiendra
	//tous les sous répertoires de ce répertoire, sotckés dans "subMedias"
	Depot *depot=new Depot(localPath,realPath,revision,readOnly);

	//On récupère les fils du noeud qui sont en fait les sous médias du "dir"
	QDomNodeList list=noeud.childNodes();

	//On parcours tous les sous médias de notre repertoire
	for(unsigned int i=0;i<list.length();i++)
	{
		QDomNode n=list.at(i); QDomElement e=n.toElement();
		if(e.tagName()=="dir")          //Le sous-médias est un répertoire
		{
			Dir *d=Dir::loadDir(n,depot);  //On le charge
			if(d==NULL)
			{
				delete depot;
				//S'il n'est pas valide on retourne NULL
				return NULL;
			}
			//On ajoute ce répertoire à la liste des subMedias de notre répertoire
			depot->subMedias->append(d);
		}

		else if(e.tagName()=="file")    //Le sous-élément est un fichier
		{
			File *f=File::loadFile(n,depot); //On le charge
			if(f==NULL)
			{
				delete depot;
				//S'il n'est pas valide on retourne NULL
				return NULL;
			}
			//On ajoute ce fichier à la liste des subMedias de notre répertoire
			depot->subMedias->append(f);
		}
	}

	//Tout est bon, on retourne le dir
	return depot;
}




//Le constructeur fait les initialisations, puis alloue le watcher, et connecte son signal directoryChanged
Depot::Depot(QString localPath,QString realPath,int revision,bool readOnly): Dir(localPath,realPath,NULL,revision,readOnly)
{
}



