#include "filemanager.h"



FileManager *FileManager::createFileManager(QString filesPath,SvnManager *svnManager)
{
	if(filesPath.isEmpty() || !svnManager) return NULL;
	QDir dir(filesPath);
	if(!dir.exists()) return NULL;
	if(!filesPath.endsWith("/")) filesPath=filesPath+"/";
	Depot::GLOBAL_DEPOTS_PATH=filesPath;
	return new FileManager(svnManager);
}




FileManager::FileManager(SvnManager *svnManager)
{
	this->svnManager=svnManager;
	this->depots=new QList<Depot*>();
}




bool FileManager::addDepot(QString depotName)
{
	if(!depotName.endsWith("/")) depotName=depotName+"/";
	if(getDepot(depotName)!=NULL) return true;
	mutex.lock();
	Depot *d=Depot::loadDepot(depotName,this->svnManager);
	if(!d)
	{
		mutex.unlock();
		return false;
	}
	depots->append(d);
	mutex.unlock();
	return true;
}



Depot *FileManager::getDepot(QString depotName)
{
	if(!depotName.endsWith("/")) depotName=depotName+"/";
	mutex.lock();
	Depot *d=NULL;
	for(int i=0;i<depots->length();i++)
	{
		if(depots->at(i)->getDepotName()==depotName) d=depots->at(i);
	}
	mutex.unlock();
	return d;
}




Depot *FileManager::depotContainer(QString mediaPath)
{
	mutex.lock();
	Depot *d=NULL;
	for(int i=0;i<depots->length();i++)
	{
		if(mediaPath.startsWith(depots->at(i)->getDepotName())) d=depots->at(i);
	}
	mutex.unlock();
	return d;
}





bool FileManager::removeDepot(QString depotName)
{
	if(!depotName.endsWith("/")) depotName=depotName+"/";
	mutex.lock();
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
	mutex.unlock();
	return found;
}


