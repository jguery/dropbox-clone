#include <QtGui/QApplication>
#include "widget.h"



QString APPLICATION_NAME="Clone-DropBox";

Server *loadConfig(QString dbPath);


//La méthode main pour tester le serveur
///////////////////////////////////////////



int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	//Si y'a pas de paramètre, on démarre une interface graphique
	if(argc==1)
	{
		//On créé la fenetre et on l'affiche
		Widget *w=new Widget();
		w->show();
	}
	//Si on a passé ue paramètre on démarre le serveur sans interface graphique
	else
	{
		Server *server=loadConfig(argv[1]);
	}

	//On exécute l'application
	return a.exec();
}






Server *loadConfig(QString dbPath)
{

	//On créé la bdd
	DatabaseManager *db=DatabaseManager::loadDatabaseManager(dbPath,"","");
	if(!db)
	{
		return NULL;
	}

	//On récupère les infos générales
	SqlGeneralInfos *infos=db->getGeneralInfos();
	if(!infos)
	{
		delete db;
		return NULL;
	}

	//On créé le svnManager
	SvnManager *svn=SvnManager::createSvnManager(infos->svnAddress,infos->svnUser,infos->svnPassword,"svn","svnadmin");
	if(!svn)
	{
		delete db;delete infos;
		return NULL;
	}

	//On créé le fileManager
	FileManager *f=FileManager::createFileManager(infos->filesPath,svn);
	if(!f)
	{
		delete db;delete infos;delete svn;
		return NULL;
	}

	QList<QString> depots=db->getDepotNames();
	int i;
	for(i=0;i<depots.length();i++)
	{
		f->addDepot(depots.at(i));
	}

	//On créé l'interface réseau
	Server *server=Server::createServer(db,f,new QTabWidget(),new QStandardItemModel());
	bool result=server->beginListenning(infos->serverPort);
	if(!result)
	{
		delete db;delete infos;delete svn;
		return NULL;
	}
	return server;
}
