#include "hddinterface.h"
#include "widget.h"


//Pour allouer un objet de type hddInterface
HddInterface *HddInterface::createHddInterface(ConfigurationData *configurationData, NetworkInterface *networkInterface,QStandardItemModel *model)
{
        if(configurationData==NULL)
            return NULL;

        if(networkInterface==NULL)
            return NULL;

        if(model==NULL)
            return NULL;

	return new HddInterface(configurationData,networkInterface,model);
}


//Le constructeur
HddInterface::HddInterface(ConfigurationData *configurationData,
                           NetworkInterface *networkInterface,
                           QStandardItemModel *model) : QObject()
{
	this->networkInterface=networkInterface;
	this->model=model;
	this->configurationData=configurationData;

	this->configurationData->getConfigurationFile()->setSignalListener(this);


        //Connecte les signaux de réceptions de messages de l'interface réseau à des slots de cette classe
        //Le traitement des messages est donc fait ICI
	QObject::connect(networkInterface,SIGNAL(receiveCreatedMediaMessage(Dir*,QString)),this,SLOT(receiveCreatedMediaMessageAction(Dir*,QString)));
	QObject::connect(networkInterface,SIGNAL(receiveModifiedFileMessage(File*,QByteArray)),this,SLOT(receiveModifiedFileMessageAction(File*,QByteArray)));
	QObject::connect(networkInterface,SIGNAL(receiveRemovedMediaMessage(Media*)),this,SLOT(receiveRemovedMediaMessageAction(Media*)));
	QObject::connect(networkInterface,SIGNAL(receiveErrorMessage(QString)),this,SLOT(receiveErrorMessageAction(QString)));
}


//Lorsqu'un média est créé...
//Principalement appelée par Dir::directoryChangedAction (car le watcher est installé dans cette classe)
void HddInterface::mediaHasBeenCreated(Media *m)
{
	QString realPath=m->getRealPath();
	if(m->isDirectory()) realPath=realPath+"/";
	networkInterface->sendMediaCreated(realPath);

	if(m->isDirectory())
	{
		Dir *d=(Dir*)m;
		d->setSignalListener(this);
	}
	Widget::addRowToTable("Le media "+m->getLocalPath()+" a été créé",model);
        configurationData->save();
}


//Lorsqu'un média est supprimé
void HddInterface::mediaHasBeenRemoved(Media *m)
{
	QString realPath=m->getRealPath();
	networkInterface->sendMediaRemoved(realPath);
	if(m->isDirectory())
	{
		Dir *d=(Dir*)m;
		d->setSignalListener(NULL);
	}
	Widget::addRowToTable("Le media "+m->getLocalPath()+" a été supprimé",model);
	delete m;
	configurationData->save();
}


//Lorsqu'un fichier est modifié
void HddInterface::fileHasBeenUpdated(File *f)
{
	QString realPath=f->getRealPath();
	QFile file(f->getLocalPath());
	file.open(QIODevice::ReadOnly);
	networkInterface->sendFileModified(realPath,file.readAll());
	file.close();
	Widget::addRowToTable("Le fichier "+f->getLocalPath()+" a été modifié",model);
	configurationData->save();
}















//on a recu la modif d'un fichier
void HddInterface::receiveModifiedFileMessageAction(File *f,QByteArray content)
{
	Dir *dir=configurationData->getConfigurationFile()->findMediaParentByLocalPath(f->getLocalPath());
	if(!dir) return;
	dir->setSignalListener(NULL);
	QFile file(f->getLocalPath());
	if(!file.open(QIODevice::WriteOnly)) return;
	file.write(content);
	file.close();
	f->updateContent();
	dir->setSignalListener(this);
	Widget::addRowToTable("Message du serveur: le fichier "+f->getLocalPath()+" a été modifié",model);
	configurationData->save();
}



void HddInterface::receiveCreatedMediaMessageAction(Dir *parent,QString realName)
{
	parent->setSignalListener(NULL);
	if(realName.endsWith("/"))
	{
		realName=realName.left(realName.length()-1);
		QDir dir(parent->getLocalPath());
		if(!dir.mkdir(realName)) return;
		Dir *d=Dir::createDir(parent->getLocalPath()+"/"+realName,parent->getRealPath()+"/"+realName);
		if(!d) return ;
		parent->getSubMedias()->append(d);
		d->setSignalListener(this);
		Widget::addRowToTable("Message du serveur: le repertoire "+d->getLocalPath()+" a été créé",model);
	}
	else
	{
		QFile file(parent->getLocalPath()+"/"+realName);
		if(!file.open(QIODevice::WriteOnly)) return;
		file.write("");
		file.close();
		File *f=File::createFile(parent->getLocalPath()+"/"+realName,parent->getRealPath()+"/"+realName);
		if(!f) return;
		parent->getSubMedias()->append(f);
		Widget::addRowToTable("Message du serveur: le fichier "+f->getLocalPath()+" a été créé",model);
	}
	parent->setSignalListener(this);
	configurationData->save();
}


void rmNonEmptyDir( QString name )
{
    QDir dir( name );
    QFileInfoList fileList = dir.entryInfoList( QDir::Files | QDir::Hidden );
    foreach(QFileInfo file, fileList) dir.remove( file.absoluteFilePath());
    QFileInfoList dirList = dir.entryInfoList( QDir::AllDirs | QDir::Hidden | QDir::NoDotAndDotDot );
    foreach(QFileInfo dir, dirList) rmNonEmptyDir(dir.absoluteFilePath());
    dir.rmdir(dir.absolutePath());
}


void HddInterface::receiveRemovedMediaMessageAction(Media *m)
{
	Dir *parent=configurationData->getConfigurationFile()->findMediaParentByLocalPath(m->getLocalPath());
	if(!parent) return;
	parent->setSignalListener(NULL);
	for(int i=0;i<parent->getSubMedias()->size();i++)
	{
		if(m==parent->getSubMedias()->at(i))
		{
			parent->getSubMedias()->remove(i);
			break;
		}
	}
	if(m->isDirectory())
	{
		Dir *d=(Dir*)m;
		d->setSignalListener(NULL);
		rmNonEmptyDir(d->getLocalPath());
		Widget::addRowToTable("Message du serveur: le repertoire "+d->getLocalPath()+" a été supprimé",model);
		delete d;
	}
	else
	{
		File *f=(File*)m;
		QFile::remove(f->getLocalPath());
		Widget::addRowToTable("Message du serveur: le fichier "+f->getLocalPath()+" a été supprimé",model);
		delete f;
	}
	parent->setSignalListener(this);
	configurationData->save();
}





void HddInterface::receiveErrorMessageAction(QString s)
{
	QMessageBox::information(NULL,"",s);
}




