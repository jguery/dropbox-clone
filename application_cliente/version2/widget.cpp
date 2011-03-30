#include "widget.h"
#include "hddinterface.h"


//Le constructeur
Widget::Widget(): QWidget()
{
	//On initialise la fenetre
	this->setWindowTitle("Client A");
	setMinimumSize(640,480);

	//On crèe une table view non éditable
	QTableView *tableView=new QTableView();
	tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

	//On crèe le model à 2 colonnes de la qtableview
	this->model=new QStandardItemModel(0,2,tableView);

	//On écrit l'entete du model
	QStringList list;
	list<<"Evenement"<<"Heure";
	model->setHorizontalHeaderLabels(list);

	//On affecte le model à sa qtableview, et on configure cette dernière
	tableView->setModel(model);
	tableView->horizontalHeader()->setStretchLastSection(true);
	tableView->setColumnWidth(0,500);

	//On range la qtableview dans la fenetre avec une layout
	QHBoxLayout *layout = new QHBoxLayout;
	layout->addWidget(tableView);
	this->setLayout(layout);
	addRowToTable("Démarage de l'application",model);

	//On créé la configuration réseau
	ConfigurationNetwork *configurationNetwork=ConfigurationNetwork::createConfigurationNetwork("localhost",4321);
	if(configurationNetwork) addRowToTable("La configuration réseau a été crée",model);
	else {addRowToTable("Echec de la création de la configuration réseau",model);return;}

	//On créé la configuration d'identification
	ConfigurationIdentification *configurationIdentification=ConfigurationIdentification::createConfigurationIdentification("hky","hky");
	if(configurationIdentification) addRowToTable("La configuration d'identification' a été crée",model);
	else {addRowToTable("Echec de la création de la configuration d'indentification",model);return;}

	//On créé la configuration de fichier
	QList<Dir*> *depots=new QList<Dir*>();
	Dir *d1=Dir::createDir("/home/hky/test/A","/sd/1");depots->append(d1);
	if(!d1){addRowToTable("Echec de la création du repertoire 1",model);return;}
	//Dir *d2=Dir::createDir("/home/hky/test/B","/sd/1");depots->append(d2);
	//if(!d2){addRowToTable("Echec de la création du repertoire 2",model);return;}
	ConfigurationFile *configurationFile=ConfigurationFile::createConfigurationFile(depots);
	if(configurationFile) addRowToTable("Les configurations des repertoires surveillés ont été créés",model);
	else {addRowToTable("Echec de la création des configurations de repertoires surveillés",model);return;}

	//On créé la configuration totale
	this->configurationData=ConfigurationData::createConfigurationData(configurationNetwork,configurationIdentification,configurationFile,"/home/hky/test/config1.xml");

	//On créé l'interface réseau
	this->networkInterface=NetworkInterface::createNetworkInterface(configurationData);
	if(networkInterface) addRowToTable("L'interface réseau a été crée",model);
	else {addRowToTable("Echec de la création de l'interface réseau",model);return;}

	//On créé l'interface disque dur
	this->hddInterface=HddInterface::createHddInterface(configurationData,networkInterface,model);
	if(hddInterface) addRowToTable("L'interface disque a été crée",model);
	else {addRowToTable("Echec de la création de l'interface disque",model);return;}

	//On tente de se connecter au serveur
	addRowToTable("Tentative de connexion au serveur",model);
	bool a=networkInterface->connect();
	if(a) addRowToTable("Success: Connexion réuissie",model);
	else addRowToTable("Echec: Connexion échouée",model);
}




//Juste une méthode statique qui écrit un évenement dans le model, et l'heure р laquelle il s'est réalisé
void Widget::addRowToTable(QString s,QStandardItemModel *model)
{
	//Les attributs statiques permettent de faire varier la couleur de la ligne
	static int r=255;
	static int g=125;
	static int b=0;

	//On fait varier la couleur de la ligne
	r=abs(40-r);g=abs(130-g);b=abs(100-b);

	//On récupère la liste des 2 colonnes de la lignes
	QList<QStandardItem*> list;
	QStandardItem *i1=new QStandardItem(s);
	i1->setBackground(QBrush(QColor::fromRgb(r,g,b)));
	QStandardItem *i2=new QStandardItem(QTime::currentTime().toString("hh:mm:ss"));
	i2->setBackground(QBrush(QColor::fromRgb(r,g,b)));
	list << i1 << i2;

	//On ajoute la ligne
	model->appendRow(list);
}



