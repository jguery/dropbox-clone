#include "messages.h"

#include <QtGui>
QByteArray *Messages::createFileContentMessage(int id, QString filePath)
{
	QFile file(filePath);
	if(!file.open(QIODevice::ReadOnly)) return NULL;
	QByteArray fileContent=file.readAll().toBase64();
	file.close();

	QDomDocument document;
	QDomElement element=document.createElement("FileContent");
	element.setAttribute("id",id);
	element.appendChild(document.createTextNode(fileContent));
	document.appendChild(element);
	return new QByteArray(document.toByteArray());
}


QByteArray *Messages::createFileCreatedMessage(int directoryId,QString newName)
{// lorsque newName se termine par un /, c'est un repertoire
	QDomDocument document;
	QDomElement element=document.createElement("FileInfo");
	element.setAttribute("type","CREATED");
	element.setAttribute("directoryId",directoryId);
	element.setAttribute("newName",newName);
	document.appendChild(element);
	return new QByteArray(document.toByteArray());
}


QByteArray *Messages::createFileRenamedMessage(int id,QString newName)
{
	QDomDocument document;
	QDomElement element=document.createElement("FileInfo");
	element.setAttribute("type","RENAMED");
	element.setAttribute("id",id);
	element.setAttribute("newName",newName);
	document.appendChild(element);
	return new QByteArray(document.toByteArray());
}

QByteArray *Messages::createFileDeletedMessage(int id)
{
	QDomDocument document;
	QDomElement element=document.createElement("FileInfo");
	element.setAttribute("type","DELETED");
	element.setAttribute("id",id);
	document.appendChild(element);
	return new QByteArray(document.toByteArray());
}


QByteArray *Messages::createFileMovedMessage(int directoryId,int id,QString newName)
{
	QDomDocument document;
	QDomElement element=document.createElement("FileInfo");
	element.setAttribute("type","MOVED");
	element.setAttribute("directoryId",directoryId);
	element.setAttribute("id",id);
	if(!newName.isEmpty()) element.setAttribute("newName",newName);
	document.appendChild(element);
	return new QByteArray(document.toByteArray());
}


QByteArray *Messages::createFileCopiedMessage(int directoryId,int id,QString newName)
{
	QDomDocument document;
	QDomElement element=document.createElement("FileInfo");
	element.setAttribute("type","COPIED");
	element.setAttribute("directoryId",directoryId);
	element.setAttribute("id",id);
	if(!newName.isEmpty()) element.setAttribute("newName",newName);
	document.appendChild(element);
	return new QByteArray(document.toByteArray());
}

