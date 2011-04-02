#include "widget.h"
#include "hddinterface.h"


//Le constructeur à 3 paramètres
//pour démarer le programme avec comme dépot le localPath et le realPath donnés
//La configuration générée sera stockée dans configSavePath
Widget::Widget(QString localPath, QString realPath, QString configSavePath): QWidget()
{
	//On initialise la fenetre
	this->setWindowTitle("Client B");
	setMinimumSize(840,480);

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
	tableView->setColumnWidth(0,700);

	//On range la qtableview dans la fenetre avec une layout
	QHBoxLayout *layout = new QHBoxLayout;
	layout->addWidget(tableView);
	this->setLayout(layout);
	addRowToTable("Démarage de l'application",model,MSG_INIT);

	//On créé la configuration réseau
	ConfigurationNetwork *configurationNetwork=ConfigurationNetwork::createConfigurationNetwork("127.0.0.1",4321);
	if(configurationNetwork) addRowToTable("La configuration réseau a été crée",model,MSG_INIT);
	else {addRowToTable("Echec de la création de la configuration réseau",model,MSG_INIT);return;}

	//On créé la configuration d'identification
	ConfigurationIdentification *configurationIdentification=ConfigurationIdentification::createConfigurationIdentification("hky","hky");
	if(configurationIdentification) addRowToTable("La configuration d'identification' a été crée",model,MSG_INIT);
	else {addRowToTable("Echec de la création de la configuration d'indentification",model,MSG_INIT);return;}

	//On créé la configuration de fichier
	QList<Dir*> *depots=new QList<Dir*>();
	Dir *d = Dir::createDir(localPath,realPath,0,false);depots->append(d);
	if(!d){addRowToTable("Echec de la création du repertoire 1",model,MSG_INIT);return;}
	ConfigurationFile *configurationFile=ConfigurationFile::createConfigurationFile(depots);
	if(configurationFile) addRowToTable("Les configurations des repertoires surveillés ont été créés",model,MSG_INIT);
	else {addRowToTable("Echec de la création des configurations de repertoires surveillés",model,MSG_INIT);return;}

	//On créé la configuration totale
	this->configurationData=ConfigurationData::createConfigurationData(configurationNetwork,configurationIdentification,configurationFile,configSavePath);

	//On créé l'interface réseau
	this->networkInterface=NetworkInterface::createNetworkInterface(configurationData,model);
	if(networkInterface) addRowToTable("L'interface réseau a été crée",model,MSG_INIT);
	else {addRowToTable("Echec de la création de l'interface réseau",model,MSG_INIT);return;}

	//On tente de se connecter au serveur
	addRowToTable("Tentative de connexion au serveur",model,MSG_NETWORK);
	bool a=networkInterface->connect();
	if(a) addRowToTable("Success: Connexion réuissie",model,MSG_NETWORK);
	else addRowToTable("Echec: Connexion échouée",model,MSG_NETWORK);

	//On créé l'interface disque dur
	this->hddInterface=HddInterface::createHddInterface(configurationData,networkInterface,model);
	if(hddInterface) addRowToTable("L'interface disque a été crée",model,MSG_INIT);
	else {addRowToTable("Echec de la création de l'interface disque",model,MSG_INIT);return;}

}



//Le constructeur à 1 paramètre
//pour démarer le programme avec la configuration stockée dans configPath
Widget::Widget(QString configPath): QWidget()
{
	//On initialise la fenetre
	this->setWindowTitle("Client B");
	setMinimumSize(840,480);

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
	tableView->setColumnWidth(0,700);

	//On range la qtableview dans la fenetre avec une layout
	QHBoxLayout *layout = new QHBoxLayout;
	layout->addWidget(tableView);
	this->setLayout(layout);
	addRowToTable("Démarage de l'application",model,MSG_INIT);

	//On charge la configuration
	this->configurationData=ConfigurationData::loadConfigurationData(configPath);
	if(configurationData)  addRowToTable("La configuration a bien été chargée",model,MSG_INIT);
	else {addRowToTable("Echec du chargement de la configuration",model,MSG_INIT);return;}

	//On créé l'interface réseau
	this->networkInterface=NetworkInterface::createNetworkInterface(configurationData,model);
	if(networkInterface) addRowToTable("L'interface réseau a été crée",model,MSG_INIT);
	else {addRowToTable("Echec de la création de l'interface réseau",model,MSG_INIT);return;}

	//On tente de se connecter au serveur
	addRowToTable("Tentative de connexion au serveur",model,MSG_NETWORK);
	bool a=networkInterface->connect();
	if(a) addRowToTable("Success: Connexion réuissie",model,MSG_NETWORK);
	else addRowToTable("Echec: Connexion échouée",model,MSG_NETWORK);

	//On créé l'interface disque dur
	this->hddInterface=HddInterface::createHddInterface(configurationData,networkInterface,model);
	if(hddInterface) addRowToTable("L'interface disque a été crée",model,MSG_INIT);
	else {addRowToTable("Echec de la création de l'interface disque",model,MSG_INIT);return;}

}




//Juste une méthode statique qui écrit un évenement dans le model, et l'heure р laquelle il s'est réalisé
void Widget::addRowToTable(QString s,QStandardItemModel *model, QColor color)
{
	//Les attributs statiques permettent de faire varier la couleur de la ligne

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



