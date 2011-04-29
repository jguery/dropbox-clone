#include "messages.h"




//****************************************************************
//                 IMPLEMENTATIONS DE LA CLASSE REQUEST
//****************************************************************

Request::Request(): Message() {this->parameters=new QHash<QString,QByteArray>();}
QHash<QString,QByteArray> *Request::getParameters() {return parameters;}
RequestEnum Request::getType() {return type;}
void Request::setType(RequestEnum type) {this->type=type;}
bool Request::isRequest() {return true;}

QByteArray *Request::toXml()
{
	if(type==CREATE_FILE_INFO)
	{
		QString realPath=parameters->value("realPath","");
		bool isDirectory=(parameters->value("isDirectory","")=="true")?true:false;
		QString revision=parameters->value("revision","");
		return Messages::createMediaCreatedMessage(realPath,isDirectory,revision);
	}
	if(type==UPDATE_FILE_INFO)
	{
		QString realPath=parameters->value("realPath","");
		QByteArray content=parameters->value("content","");
		QString revision=parameters->value("revision","");
		return Messages::createMediaUpdatedMessage(realPath,content,revision);
	}
	if(type==REMOVE_FILE_INFO)
	{
		QString realPath=parameters->value("realPath","");
		QString revision=parameters->value("revision","");
		return Messages::createMediaRemovedMessage(realPath,revision);
	}
	if(type==REVISION_FILE_INFO)
	{
		QString realPath=parameters->value("realPath","");
		QString revision=parameters->value("revision","");
		return Messages::createDepotRevisionMessage(realPath,revision);
	}
	if(type==END_OLD_DETECTIONS)
	{
		return Messages::createEndOldDetections();
	}
	if(type==IDENTIFICATION)
	{
		QString pseudo=parameters->value("pseudo","");
		QString password=parameters->value("password","");
		return Messages::createIdentificationMessage(pseudo,password);
	}
	return NULL;
}



//****************************************************************
//                 IMPLEMENTATIONS DE LA CLASSE RESPONSE
//****************************************************************

Response::Response(): Message() {this->parameters=new QHash<QString,QByteArray>();}
QHash<QString,QByteArray> *Response::getParameters() {return parameters;}
ResponseEnum Response::getType() {return type;}
void Response::setType(ResponseEnum type) {this->type=type;}
bool Response::isRequest() {return false;}

QByteArray *Response::toXml()
{
	QString revision=parameters->value("revision","");
	return Messages::createResponseMessage(type,revision);
}









//****************************************************************
//                 IMPLEMENTATIONS DES METHODES STATIQUES
//****************************************************************


//Requete pour avertir de la modification du contenu d'un fichier
QByteArray *Messages::createMediaUpdatedMessage(QString realPath, QByteArray content, QString revision)
{
	QDomDocument document;

	//On crèe un noeud avec le nom FileInfo
	QDomElement element=document.createElement("FileInfo");
	element.setAttribute("type","UPDATED");

	//On écrit le realPath et le contenu du fichier au format base64 (nécéssaire pour les fichiers binaires)
	element.setAttribute("realPath",realPath);
	element.setAttribute("revision",revision);
	element.appendChild(document.createTextNode(content.toBase64()));
	document.appendChild(element);

	//On retourne le message dans un QByteArray
	return new QByteArray(document.toByteArray());
}



//Avertir de la création d'un nouveau Media
QByteArray *Messages::createMediaCreatedMessage(QString realPath, bool isDirectory, QString revision)
{
	QDomDocument document;

	//On crèe un noeud avec le nom FileInfo et le type CREATED
	QDomElement element=document.createElement("FileInfo");
	element.setAttribute("type","CREATED");

	//On écrit le realPath
	element.setAttribute("realPath",realPath);
	element.setAttribute("revision",revision);
	element.setAttribute("isDirectory",isDirectory?"true":"false");
	document.appendChild(element);

	//On retourne le message dans un QByteArray
	return new QByteArray(document.toByteArray());
}





//Avertir de la suppresion d'un média
QByteArray *Messages::createMediaRemovedMessage(QString realPath, QString revision)
{
	QDomDocument document;

	//On crèe un noeud avec le nom FileInfo et le type REMOVED
	QDomElement element=document.createElement("FileInfo");
	element.setAttribute("type","REMOVED");

	//On écrit le realPath
	element.setAttribute("realPath",realPath);
	element.setAttribute("revision",revision);
	document.appendChild(element);

	//On retourne le message dans un QByteArray
	return new QByteArray(document.toByteArray());
}




//Pour envoyer un numéro de révision
QByteArray *Messages::createDepotRevisionMessage(QString realPath,QString revision)
{
	QDomDocument document;

	//On crèe un noeud avec le nom FileInfo et le type REVISION_FILE_INFO
	QDomElement element=document.createElement("FileInfo");
	element.setAttribute("type","REVISION_FILE_INFO");

	//On écrit le realPath et la révision
	element.setAttribute("realPath",realPath);
	element.setAttribute("revision",revision);
	document.appendChild(element);

	//On retourne le message dans un QByteArray
	return new QByteArray(document.toByteArray());
}






//Pour s'identifier
QByteArray *Messages::createIdentificationMessage(QString pseudo, QString password)
{
	QDomDocument document;

	//On crèe un noeud avec le nom Identification
	QDomElement element=document.createElement("IDENTIFICATION");

	//On écrit le pseudo et le password
	element.setAttribute("pseudo",pseudo);
	element.setAttribute("password",password);
	document.appendChild(element);

	//On retourne le message dans un QByteArray
	return new QByteArray(document.toByteArray());
}






//Pour répondre
QByteArray *Messages::createResponseMessage(ResponseEnum type, QString revision)
{
	QDomDocument document;

	//On crèe un noeud avec le nom Identification
	QDomElement element=document.createElement("RESPONSE");

	//On écrit le code de réponse
	element.setAttribute("code",QString::number((int)type));
	element.setAttribute("revision",revision);
	document.appendChild(element);

	//On retourne le message dans un QByteArray
	return new QByteArray(document.toByteArray());
}


//Pour terminer un envoi d'anciennes détections
QByteArray *Messages::createEndOldDetections()
{
	QDomDocument document;

	//On crèe un noeud avec le nom END_OLD_DETECTIONS
	QDomElement element=document.createElement("END_OLD_DETECTIONS");
	document.appendChild(element);

	//On retourne le message dans un QByteArray
	return new QByteArray(document.toByteArray());
}





//Pour lire un message recu en xml
Message *Messages::parseMessage(QByteArray *message)
{
	//On ouvre un document xml avec le message comme contenu
	QDomDocument document;
	if(!document.setContent(*message)) return NULL;

	//On récupère les noeuds fils du document xml, et on les parcours
	QDomNodeList list=document.childNodes();
	for(unsigned int i=0;i<list.length();i++)
	{
		//Si c'est un noeud du type FileInfo
		if(list.at(i).toElement().tagName()=="FileInfo")
		{
			//On récupère le type de la requête
			QString type=list.at(i).toElement().attribute("type","");
			Request *request=new Request();

			//si c'est un message de modification de fichier ou repertoire
			if(type=="UPDATED")
			{
				request->setType(UPDATE_FILE_INFO);

				//On récupère le realPath et la revision
				QString realPath=list.at(i).toElement().attribute("realPath","");
				QString revision=list.at(i).toElement().attribute("revision","");

				//On récupère le contenu du fichier
				QByteArray fileContent;
				if(list.at(i).childNodes().count()>0 && list.at(i).firstChild().isText())
				{
					QDomText text=list.at(i).firstChild().toText();
					//attention il est en base64 : ancienne source de bug // corrigé
					fileContent=QByteArray::fromBase64(text.data().toAscii());
				}

				//On écrit un message à retourner
				request->getParameters()->insert("realPath",realPath.toAscii());
				request->getParameters()->insert("revision",revision.toAscii());
				request->getParameters()->insert("content",fileContent);
				return request;
			}

			//si c'est un message de création de fichier ou repertoire
			if(type=="CREATED")
			{
				request->setType(CREATE_FILE_INFO);

				//On récupère le realPath, la révision et isDirectory
				QString realPath=list.at(i).toElement().attribute("realPath","");
				QString revision=list.at(i).toElement().attribute("revision","");
				QString isDirectory=list.at(i).toElement().attribute("isDirectory");

				//On écrit un message à retourner
				request->getParameters()->insert("realPath",realPath.toAscii());
				request->getParameters()->insert("isDirectory",isDirectory.toAscii());
				request->getParameters()->insert("revision",revision.toAscii());
				return request;
			}

			//Sinon si c'est une requete de suppression de fichier ou repertoire
			else if(type=="REMOVED")
			{
				request->setType(REMOVE_FILE_INFO);

				//On récupère le realPath et la revision
				QString realPath=list.at(i).toElement().attribute("realPath","");
				QString revision=list.at(i).toElement().attribute("revision","");

				//On écrit un message à retourner
				request->getParameters()->insert("realPath",realPath.toAscii());
				request->getParameters()->insert("revision",revision.toAscii());
				return request;
			}

			//Sinon si c'est une requete de revision de dépot
			else if(type=="REVISION_FILE_INFO")
			{
				request->setType(REVISION_FILE_INFO);

				//On récupère le realPath et la revision
				QString realPath=list.at(i).toElement().attribute("realPath","");
				QString revision=list.at(i).toElement().attribute("revision","");

				//On écrit un message à retourner
				request->getParameters()->insert("realPath",realPath.toAscii());
				request->getParameters()->insert("revision",revision.toAscii());
				return request;
			}
			return NULL;
		}

		//Si c'est un message de fin d'anciennes détections
		if(list.at(i).toElement().tagName()=="END_OLD_DETECTIONS")
		{
			Request *request=new Request();
			request->setType(END_OLD_DETECTIONS);
			return request;
		}

		//Si c'est un message d'identification
		if(list.at(i).toElement().tagName()=="IDENTIFICATION")
		{
			Request *request=new Request();
			request->setType(IDENTIFICATION);

			//On récupère le pseudo et password
			QString pseudo=list.at(i).toElement().attribute("pseudo","");
			QString password=list.at(i).toElement().attribute("password","");

			//On écrit un message hash à retourner
			request->getParameters()->insert("pseudo",pseudo.toAscii());
			request->getParameters()->insert("password",password.toAscii());
			return request;
		}

		//Si c'est une réponse
		if(list.at(i).toElement().tagName()=="RESPONSE")
		{
			Response *response=new Response();

			//On récupère le code
			QString codeString=list.at(i).toElement().attribute("code","");
			int code=codeString.toInt();
			QString revision=list.at(i).toElement().attribute("revision","");

			response->setType((ResponseEnum)code);
			response->getParameters()->insert("revision",revision.toAscii());
			return response;
		}
	}
	//Si on ne trouve aucune requete connue, on retourne NULL
	//Dans les normes ca ne devrait jamais arrivé ;)
	return NULL;
}
