#include "widget.h"


//Le constructeur
Widget::Widget(): QWidget()
{
	//On initialise la fenetre et ses composants
	this->setWindowTitle("Client");
	setFixedSize(640,480);
	QTableView *tableView=new QTableView();
	tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	this->model=new QStandardItemModel(0,2,tableView);
	QStringList list;
	list<<"Evenement"<<"Heure";
	model->setHorizontalHeaderLabels(list);
	tableView->setModel(model);
	tableView->horizontalHeader()->setStretchLastSection(true);
	tableView->setColumnWidth(0,500);
	QHBoxLayout *layout = new QHBoxLayout;
	layout->addWidget(tableView);
	this->setLayout(layout);
	addRowToTable("D�marage de l'application",model);

	//On cr�� l'interface r�seau
	this->server=new Server(model);
	bool result=this->server->beginListenning(4321);
	if(result) addRowToTable("Le serveur est d�marr� sur le port 4321",model);
	else {addRowToTable("Echec de d�marrage du serveur sur le port 4321",model);return;}
}


//Juste une m�thode statique qui �crit un �venement dans le model, et l'heure � laquelle il s'est r�alis�
void Widget::addRowToTable(QString s,QStandardItemModel *model)
{
	static int r=255;
	static int g=125;
	static int b=0;
	r=abs(40-r);g=abs(130-g);b=abs(100-b);
	QList<QStandardItem*> list;
	QStandardItem *i1=new QStandardItem(s);
	i1->setBackground(QBrush(QColor::fromRgb(r,g,b)));
	QStandardItem *i2=new QStandardItem(QTime::currentTime().toString("hh:mm:ss"));
	i2->setBackground(QBrush(QColor::fromRgb(r,g,b)));
	list << i1 << i2;
	model->appendRow(list);
}
