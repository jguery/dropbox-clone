#include "widget.h"
#include "hddinterface.h"
#include<iostream>

//Le constructeur à 3 paramètres
//pour démarer le programme avec comme dépot le localPath et le realPath donnés
//La configuration générée sera stockée dans configSavePath
Widget::Widget(QString localPath, QString realPath, QString configSavePath): QWidget()
{
	//On initialise la fenetre
	this->setWindowTitle("Client A");
	buildInterface();

	//On créé la configuration réseau
	ConfigurationNetwork *configurationNetwork=ConfigurationNetwork::createConfigurationNetwork("127.0.0.1",4321);
	if(configurationNetwork) addRowToTable("La configuration réseau a été crée",detectionModel,MSG_1);
	else {addRowToTable("Echec de la création de la configuration réseau",detectionModel,MSG_1);return;}

	//On créé la configuration d'identification
	ConfigurationIdentification *configurationIdentification=ConfigurationIdentification::createConfigurationIdentification("hky","hky");
	if(configurationIdentification) addRowToTable("La configuration d'identification a été crée",detectionModel,MSG_1);
	else {addRowToTable("Echec de la création de la configuration d'indentification",detectionModel,MSG_1);return;}

	//On créé la configuration de fichier
	QList<Depot*> *depots=new QList<Depot*>();
	Depot *d = Depot::createDepot(localPath,realPath,0,false);depots->append(d);
	if(!d){addRowToTable("Echec de la création du repertoire 1",detectionModel,MSG_1);return;}
	ConfigurationFile *configurationFile=ConfigurationFile::createConfigurationFile(depots,detectionModel);

	//On créé la configuration totale
	this->configurationData=ConfigurationData::createConfigurationData(configurationNetwork,configurationIdentification,configurationFile,configSavePath);

	//On créé l'interface réseau
	this->networkInterface=NetworkInterface::createNetworkInterface(configurationNetwork,configurationIdentification,networkModel);

	//On créé l'interface disque dur
	this->hddInterface=HddInterface::createHddInterface(configurationData,networkInterface,transfertModel);

}






//Le constructeur à 1 paramètre
//pour démarer le programme avec la configuration stockée dans configPath
Widget::Widget(QString configPath): QWidget()
{
	//On initialise la fenetre
	this->setWindowTitle("Client A");
	buildInterface();

	//On charge la configuration
	this->configurationData=ConfigurationData::loadConfigurationData(configPath,detectionModel);
	if(configurationData)  addRowToTable("La configuration a bien été chargée",detectionModel,MSG_1);
	else {addRowToTable("Echec du chargement de la configuration",detectionModel,MSG_1);return;}

	//On créé l'interface réseau
	this->networkInterface=NetworkInterface::createNetworkInterface(configurationData->getConfigurationNetwork(),configurationData->getConfigurationIdentification(),networkModel);

	//On créé l'interface disque dur
	this->hddInterface=HddInterface::createHddInterface(configurationData,networkInterface,transfertModel);

}




void Widget::networkButtonSlot()
{
	if(networkInterface==NULL) return;
	if(this->networkInterface->checkIsConnected())
	{
		this->networkInterface->disconnectFromServer();
		this->networkButton->setText(trUtf8("Se connecter au serveur"));
	}
	else
	{
		this->networkInterface->connectToServer();
		this->networkButton->setText(trUtf8("Se déconnecter du serveur"));
	}
}



void Widget::detectionButtonSlot()
{
	if(configurationData==NULL || configurationData->getConfigurationFile()==NULL) return;
	if(this->configurationData->getConfigurationFile()->isListenning())
	{
		this->configurationData->getConfigurationFile()->setListenning(false);;
		this->detectionButton->setText(trUtf8("Démarer les détections"));
	}
	else
	{
		this->configurationData->getConfigurationFile()->setListenning(true);
		this->detectionButton->setText(trUtf8("Arreter les détections"));
	}
}





void Widget::buildInterface()
{
	setMinimumSize(840,480);

	QStringList list;
	list<<trUtf8("Evenement")<<trUtf8("Heure");

	//On construit le QTabWidget
	onglets=new QTabWidget(this);
	QHBoxLayout *layout=new QHBoxLayout();
	layout->addWidget(onglets);this->setLayout(layout);

	//On construit l'onglet du network
	networkOnglet=new QWidget();
	QVBoxLayout *layout1=new QVBoxLayout();
	QTableView *networkView=new QTableView();
	networkView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	networkModel=new QStandardItemModel(0,2,networkView);
	networkModel->setHorizontalHeaderLabels(list);
	networkView->setModel(networkModel);
	networkView->horizontalHeader()->setStretchLastSection(true);
	networkView->setColumnWidth(0,650);
	layout1->addWidget(networkView);
	networkButton=new QPushButton(trUtf8("Se connecter au serveur"));
	QObject::connect(networkButton,SIGNAL(clicked()),this,SLOT(networkButtonSlot()));
	layout1->addWidget(networkButton);
	networkOnglet->setLayout(layout1);
	onglets->addTab(networkOnglet,trUtf8("Réseau"));

	//On construit l'onglet des transferts
	transfertOnglet=new QWidget();
	QVBoxLayout *layout2=new QVBoxLayout();
	QTableView *transfertView=new QTableView();
	transfertView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	transfertModel=new QStandardItemModel(0,2,transfertView);
	transfertModel->setHorizontalHeaderLabels(list);
	transfertView->setModel(transfertModel);
	transfertView->horizontalHeader()->setStretchLastSection(true);
	transfertView->setColumnWidth(0,650);
	layout2->addWidget(transfertView);
	transfertOnglet->setLayout(layout2);
	onglets->addTab(transfertOnglet,trUtf8("Transferts"));

	//On construit l'onglet des détections
	detectionOnglet=new QWidget();
	QVBoxLayout *layout3=new QVBoxLayout();
	QTableView *detectionView=new QTableView();
	detectionView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	detectionModel=new QStandardItemModel(0,2,detectionView);
	detectionModel->setHorizontalHeaderLabels(list);
	detectionView->setModel(detectionModel);
	detectionView->horizontalHeader()->setStretchLastSection(true);
	detectionView->setColumnWidth(0,650);
	layout3->addWidget(detectionView);
	detectionButton=new QPushButton(trUtf8("Démarer les détections"));
	QObject::connect(detectionButton,SIGNAL(clicked()),this,SLOT(detectionButtonSlot()));
	layout3->addWidget(detectionButton);
	detectionOnglet->setLayout(layout3);
	onglets->addTab(detectionOnglet,trUtf8("Détections"));

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



