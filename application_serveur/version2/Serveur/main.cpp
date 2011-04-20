#include <QtGui/QApplication>
#include "widget.h"
#include "depot.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	//Widget w;
	//w.show();
	//return a.exec();
	SvnManager *s=SvnManager::createSvnManager("hky","hky","svn","svnadmin");
	if(s) qDebug("yes");
	else qDebug("no");
	qDebug(QString::number(s->getRevision(Depot::GLOBAL_DEPOTS_PATH+"projetdev")).toAscii());
	return 0;
}
