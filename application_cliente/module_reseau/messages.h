#ifndef MESSAGES_H
#define MESSAGES_H

#include <QtCore>
#include <QtXml>

class Messages
{
public:
	static QByteArray *createFileContentMessage(int id, QString filePath);
	static QByteArray *createFileCreatedMessage(int directoryId,QString newName);
	static QByteArray *createFileRenamedMessage(int id,QString newName);
	static QByteArray *createFileDeletedMessage(int id);
	static QByteArray *createFileMovedMessage(int directoryId,int id,QString newName="");
	static QByteArray *createFileCopiedMessage(int directoryId,int id,QString newName="");
};

#endif // MESSAGES_H


