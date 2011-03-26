#include "messages.h"


//Requete pour avertir de la modification du contenu d'un fichier
QByteArray *Messages::createFileContentMessage(QString realPath, QByteArray content)
{
	QDomDocument document;
	QDomElement element=document.createElement("FileContent");
	element.setAttribute("realPath",realPath);
	element.appendChild(document.createTextNode(content));
	document.appendChild(element);
	return new QByteArray(document.toByteArray());
}

//Averir de la création d'un nouveau Media
QByteArray *Messages::createMediaCreatedMessage(QString realPath)
{
	QDomDocument document;
	QDomElement element=document.createElement("FileInfo");
	element.setAttribute("type","CREATED");
	element.setAttribute("realPath",realPath);
	document.appendChild(element);
	return new QByteArray(document.toByteArray());
}

//Avertir de la suppresion d'un média
QByteArray *Messages::createMediaRemovedMessage(QString realPath)
{
	QDomDocument document;
	QDomElement element=document.createElement("FileInfo");
	element.setAttribute("type","REMOVED");
	element.setAttribute("realPath",realPath);
	document.appendChild(element);
	return new QByteArray(document.toByteArray());
}

//Pour s'identifier
QByteArray *Messages::createIdentificationMessage(QString pseudo, QString password)
{
	QDomDocument document;
	QDomElement element=document.createElement("Identification");
	element.setAttribute("pseudo",pseudo);
	element.setAttribute("password",password);
	document.appendChild(element);
	return new QByteArray(document.toByteArray());
}


//Message de validation (OK)
QByteArray *Messages::createValidationMessage()
{
	QDomDocument document;
	QDomElement element=document.createElement("Validation");
	document.appendChild(element);
	return new QByteArray(document.toByteArray());
}


//Message d'annulation (ANNULER)
QByteArray *Messages::createAnnulationMessage()
{
	QDomDocument document;
	QDomElement element=document.createElement("Annulation");
	document.appendChild(element);
	return new QByteArray(document.toByteArray());
}


//Pour lire un message recu en xml
QHash<QString,QByteArray> *Messages::parseMessage(QByteArray *message)
{
	if(message==NULL) return NULL;
	QDomDocument document;
	if(!document.setContent(*message)) return NULL;
	QDomNodeList list=document.childNodes();
	for(unsigned int i=0;i<list.length();i++)
	{
		if(list.at(i).toElement().tagName()=="FileContent")
		{
			QString realPath=list.at(i).toElement().attribute("realPath","");
			if(realPath=="") continue;
			QByteArray fileContent;
			if(list.at(i).childNodes().count()>0 && list.at(i).firstChild().isText())
			{
				QDomText text=list.at(i).firstChild().toText();
				fileContent=QByteArray::fromBase64(text.data().toAscii());
			}
			QHash<QString,QByteArray> *hash=new QHash<QString,QByteArray>();
			hash->insert("action","MODIFIED");
			hash->insert("realPath",realPath.toAscii());
			hash->insert("content",fileContent);
			return hash;
		}
		if(list.at(i).toElement().tagName()=="FileInfo")
		{
			QString type=list.at(i).toElement().attribute("type","");
			if(type=="CREATED")
			{
				QString realPath=list.at(i).toElement().attribute("realPath","");
				if(realPath=="") continue;
				QHash<QString,QByteArray> *hash=new QHash<QString,QByteArray>();
				hash->insert("action","CREATED");
				hash->insert("realPath",realPath.toAscii());
				return hash;
			}
			else if(type=="REMOVED")
			{
				QString realPath=list.at(i).toElement().attribute("realPath","");
				if(realPath=="") continue;
				QHash<QString,QByteArray> *hash=new QHash<QString,QByteArray>();
				hash->insert("action","REMOVED");
				hash->insert("realPath",realPath.toAscii());
				return hash;
			}
			else continue;
		}
		if(list.at(i).toElement().tagName()=="Identification")
		{
			QString pseudo=list.at(i).toElement().attribute("pseudo","");
			if(pseudo=="") continue;
			QString password=list.at(i).toElement().attribute("password","");
			if(password=="") continue;
			QHash<QString,QByteArray> *hash=new QHash<QString,QByteArray>();
			hash->insert("action","IDENTIFICATION");
			hash->insert("pseudo",pseudo.toAscii());
			hash->insert("password",password.toAscii());
			return hash;
		}
		if(list.at(i).toElement().tagName()=="Validation")
		{
			QHash<QString,QByteArray> *hash=new QHash<QString,QByteArray>();
			hash->insert("action","VALIDATION");
			return hash;
		}
		if(list.at(i).toElement().tagName()=="Annulation")
		{
			QHash<QString,QByteArray> *hash=new QHash<QString,QByteArray>();
			hash->insert("action","ANNULATION");
			return hash;
		}
	}
	return NULL;
}
