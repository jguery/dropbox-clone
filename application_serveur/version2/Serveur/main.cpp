#include <QtGui/QApplication>
#include "widget.h"
#include "depot.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	Widget w;
	w.show();
	/*DatabaseManager *d=DatabaseManager::createDatabaseManager("dropbox","hky","hky");
	if(d)
	{
		qDebug("db chargee");
		FileManager *f=FileManager::createFileManager(SvnManager::createSvnManager("svn://pchky/","hky","hky"));
		QList<SqlDepot*> l=d->getDepots();
		for(int i=0;i<l.size();i++) f->addDepot(l.at(i)->depotname);
		f->getDepot("projetdev/")->createDir("projetdev/test234","hky","hky");
		f->getDepot("projetdev/")->createFile("projetdev/test234/a","hky","hky");
		f->getDepot("projetdev/")->updateFileContent("projetdev/test234/a","Mon troisieme test","hky","hky");
		f->getDepot("projetdev/")->deleteMedia("projetdev/test1","hky","hky");
		f->getDepot("projetdev/")->createDir("projetdev/test4","hky","hky");
	}*/
	return a.exec();
	//return 0;
}
