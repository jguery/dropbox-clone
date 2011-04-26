#include "widget.h"


//Le constructeur
Widget::Widget(): QWidget()
{
	//On initialise la fenetre et ses composants
	this->setWindowTitle("Serveur");
	setMinimumSize(640,480);
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
	addRowToTable("Démarage de l'application",model,MSG_1);

	//On créé la bdd
	DatabaseManager *db=DatabaseManager::createDatabaseManager("dropbox","hky","hky");
	if(db) addRowToTable("La base de données a été chargée",model,MSG_1);
	else {addRowToTable("Erreur au chargement de la base de données",model,MSG_1);return;}

	//On créé le fileManager
	SvnManager *svn=SvnManager::createSvnManager("svn://pchky.maisel.enst-bretagne.fr/","hky","hky","svn","svnadmin");
	if(svn) addRowToTable("Le gestionnaire SVN a été créé",model,MSG_1);
	else {addRowToTable("Erreur à la création du gestionnaire SVN",model,MSG_1);return;}

	FileManager *f=FileManager::createFileManager(svn);
	if(f) addRowToTable("Le gestionnaire de fichiers a été créé",model,MSG_1);
	else {addRowToTable("Erreur à la création du gestionnaire de fichiers",model,MSG_1);return;}
	QList<SqlDepot*> depots=db->getDepots();
	for(int i=0;i<depots.length();i++) f->addDepot(depots.at(i)->depotname);

	//On créé l'interface réseau
	this->server=Server::createServer(db,f,model);
	bool result=this->server->beginListenning(4321);
	if(result) addRowToTable("Le serveur est démarré sur le port 4321",model,MSG_1);
	else {addRowToTable("Echec de démarrage du serveur sur le port 4321",model,MSG_1);return;}
}





//Juste une méthode statique qui écrit un évenement dans le model, et l'heure р laquelle il s'est réalisé
void Widget::addRowToTable(QString s, QStandardItemModel *model, QColor color)
{
	//On récupère la liste des 2 colonnes de la lignes
	QList<QStandardItem*> list;
	QStandardItem *i1=new QStandardItem(trUtf8(s.toAscii()));
	i1->setBackground(QBrush(color));
	QStandardItem *i2=new QStandardItem(QTime::currentTime().toString("hh:mm:ss"));
	i2->setBackground(QBrush(color));
	list << i1 << i2;
	//On ajoute la ligne
	model->appendRow(list);
}



