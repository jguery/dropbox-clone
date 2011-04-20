#include "filemanager.h"



FileManager *FileManager::createFileManager(SvnManager *svnManager)
{
	if(!svnManager) return NULL;
	return new FileManager(svnManager);
}



FileManager::FileManager(SvnManager *svnManager)
{
	this->svnManager=svnManager;
	this->depots=new QList<Depot*>();
}




bool FileManager::addDepot(QString depotName)
{
	if(getDepot(depotName)!=NULL) return true;
	listDepotsMutex.lock();
	Depot *d=Depot::loadDepot(depotName,this->svnManager);
	if(!d) d=Depot::createDepot(depotName,this->svnManager);
	if(!d)
	{
		listDepotsMutex.unlock();
		return false;
	}
	depots->append(d);
	listDepotsMutex.unlock();
	return true;
}



Depot *FileManager::getDepot(QString depotName)
{
	listDepotsMutex.lock();
	Depot *d=NULL;
	for(int i=0;i<depots->length();i++)
	{
		if(depots->at(i)->getDepotName()==depotName) d=depots->at(i);
	}
	listDepotsMutex.unlock();
	return d;
}



bool FileManager::removeDepot(QString depotName)
{
	listDepotsMutex.lock();
	bool found=false;
	for(int i=0;i<depots->length();i++)
	{
		if(depots->at(i)->getDepotName()==depotName)
		{
			Depot *d=depots->at(i);
			depots->removeAt(i);
			QString path=Depot::GLOBAL_DEPOTS_PATH+d->getDepotName();
			delete d;
			Depot::removeNonEmptyDirectory(path);
			found=true;
		}
	}
	listDepotsMutex.unlock();
	return found;
}


