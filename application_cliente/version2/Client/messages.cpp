#include "messages.h"


//Requete pour avertir de la modification du contenu d'un fichier
QByteArray *Messages::createFileContentMessage(QString realPath, QByteArray content)
{
	QDomDocument document;

	//On crèe un noeud avec le nom FileContent
	QDomElement element=document.createElement("FileContent");

	//On écrit le realPath et le contenu du fichier au format base64 (nécéssaire pour les fichiers binaires)
	element.setAttribute("realPath",realPath);
	element.appendChild(document.createTextNode(content.toBase64()));
	document.appendChild(element);

	//On retourne le message dans un QByteArray
	return new QByteArray(document.toByteArray());
}

//Avertir de la création d'un nouveau Media
QByteArray *Messages::createMediaCreatedMessage(QString realPath)
{
	QDomDocument document;

	//On crèe un noeud avec le nom FileContent et le type CREATED
	QDomElement element=document.createElement("FileInfo");
	element.setAttribute("type","CREATED");

	//On écrit le realPath
	element.setAttribute("realPath",realPath);
	document.appendChild(element);

	//On retourne le message dans un QByteArray
	return new QByteArray(document.toByteArray());
}



//Avertir de la suppresion d'un média
QByteArray *Messages::createMediaRemovedMessage(QString realPath)
{
	QDomDocument document;

	//On crèe un noeud avec le nom FileContent et le type REMOVED
	QDomElement element=document.createElement("FileInfo");
	element.setAttribute("type","REMOVED");

	//On écrit le realPath
	element.setAttribute("realPath",realPath);
	document.appendChild(element);

	//On retourne le message dans un QByteArray
	return new QByteArray(document.toByteArray());
}

//Pour s'identifier
QByteArray *Messages::createIdentificationMessage(QString pseudo, QString password)
{
	QDomDocument document;

	//On crèe un noeud avec le nom Identification
	QDomElement element=document.createElement("Identification");

	//On écrit le pseudo et le password
	element.setAttribute("pseudo",pseudo);
	element.setAttribute("password",password);
	document.appendChild(element);

	//On retourne le message dans un QByteArray
	return new QByteArray(document.toByteArray());
}


//Message de validation (OK)
QByteArray *Messages::createValidationMessage()
{
	QDomDocument document;

	//On crèe un noeud avec le nom Validation
	QDomElement element=document.createElement("Validation");
	document.appendChild(element);

	//On retourne le message dans un QByteArray
	return new QByteArray(document.toByteArray());
}


//Message d'annulation (ANNULER)
QByteArray *Messages::createAnnulationMessage()
{
	QDomDocument document;

	//On crèe un noeud avec le nom Annulation
	QDomElement element=document.createElement("Annulation");
	document.appendChild(element);

	//On retourne le message dans un QByteArray
	return new QByteArray(document.toByteArray());
}



//Pour lire un message recu en xml
QHash<QString,QByteArray> *Messages::parseMessage(QByteArray *message)
{
	//Si le message est NULL pas la peine de continuer
	if(message==NULL) return NULL;

	//On ouvre un document xml avec le message comme contenu
	QDomDocument document;
	if(!document.setContent(*message)) return NULL;

	//On récupère les noeuds fils du document xml, et on les parcours
	QDomNodeList list=document.childNodes();
	for(unsigned int i=0;i<list.length();i++)
	{

		//Si c'est un noeud du type FileContent (requete de modification d'un fichier)
		if(list.at(i).toElement().tagName()=="FileContent")
		{
			//On récupère le realPath
			QString realPath=list.at(i).toElement().attribute("realPath","");
			if(realPath.isEmpty()) continue;

			//On récupère le contenu du fichier
			QByteArray fileContent;
			if(list.at(i).childNodes().count()>0 && list.at(i).firstChild().isText())
			{
				QDomText text=list.at(i).firstChild().toText();
				//attention il est en base64 : ancienne source de bug // corrigé
				fileContent=QByteArray::fromBase64(text.data().toAscii());
			}

			//On écrit un message hash à retourner
			QHash<QString,QByteArray> *hash=new QHash<QString,QByteArray>();
			hash->insert("action","MODIFIED");
			hash->insert("realPath",realPath.toAscii());
			hash->insert("content",fileContent);
			return hash;
		}

		//Si c'est un noeud du type FileInfo
		if(list.at(i).toElement().tagName()=="FileInfo")
		{
			//On récupère le type de la requête
			QString type=list.at(i).toElement().attribute("type","");

			//si c'est un message de création de fichier ou repertoire
			if(type=="CREATED")
			{
				//On récupère le realPath
				QString realPath=list.at(i).toElement().attribute("realPath","");
				if(realPath.isEmpty()) continue;

				//On écrit un message hash à retourner
				QHash<QString,QByteArray> *hash=new QHash<QString,QByteArray>();
				hash->insert("action","CREATED");
				hash->insert("realPath",realPath.toAscii());
				return hash;
			}

			//Sinon si c'est une requete de suppression de fichier ou repertoire
			else if(type=="REMOVED")
			{
				//On récupère le realPath
				QString realPath=list.at(i).toElement().attribute("realPath","");
				if(realPath.isEmpty()) continue;

				//On écrit un message hash à retourner
				QHash<QString,QByteArray> *hash=new QHash<QString,QByteArray>();
				hash->insert("action","REMOVED");
				hash->insert("realPath",realPath.toAscii());
				return hash;
			}
			else continue;
		}

		//Si c'est un message d'identification
		if(list.at(i).toElement().tagName()=="Identification")
		{
			//On récupère le pseudo et password
			QString pseudo=list.at(i).toElement().attribute("pseudo","");
			if(pseudo=="") continue;

			QString password=list.at(i).toElement().attribute("password","");
			if(password=="") continue;

			//On écrit un message hash à retourner
			QHash<QString,QByteArray> *hash=new QHash<QString,QByteArray>();
			hash->insert("action","IDENTIFICATION");
			hash->insert("pseudo",pseudo.toAscii());
			hash->insert("password",password.toAscii());
			return hash;
		}
		if(list.at(i).toElement().tagName()=="Validation")
		{			
			//On écrit un message hash à retourner
			QHash<QString,QByteArray> *hash=new QHash<QString,QByteArray>();
			hash->insert("action","VALIDATION");
			return hash;
		}
		if(list.at(i).toElement().tagName()=="Annulation")
		{
			//On écrit un message hash à retourner
			QHash<QString,QByteArray> *hash=new QHash<QString,QByteArray>();
			hash->insert("action","ANNULATION");
			return hash;
		}
	}
	//Si on ne trouve aucune requete connue, on retourne NULL
	//Dans les normes ca ne devrait jamais arrivé ;)
	return NULL;
}
