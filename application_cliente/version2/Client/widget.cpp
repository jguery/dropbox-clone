#include "widget.h"
#include "hddinterface.h"
#include <iostream>




//Le constructeur à 3 paramètres
//pour démarer le programme avec comme dépot le localPath et le realPath donnés
//La configuration générée sera stockée dans configSavePath
Widget::Widget(): QWidget()
{
	//On initialise la fenetre
	this->setWindowTitle("Logiciel client");
	this->settings=new QSettings("AppClientProjetS2", "AppClientProjetS2");
	buildInterface();
	networkInterface=NULL;
	configurationData=NULL;
	hddInterface=NULL;
}





void Widget::networkButtonSlot()
{
	if(networkInterface==NULL) return;
	if(this->networkInterface->checkIsConnected())
	{
		networkInterface->requestDisconnectFromServer();
	}
	else
	{
		networkInterface->requestConnectToServer();
	}
}



void Widget::detectionButtonSlot()
{
	if(configurationData==NULL || configurationData->getConfigurationFile()==NULL) return;
	if(this->configurationData->getConfigurationFile()->isListenning())
	{
		this->configurationData->getConfigurationFile()->setListenning(false);
	}
	else
	{
		this->configurationData->getConfigurationFile()->setListenning(true);
	}
}





void Widget::parcourirLoadConfigSlot()
{
	QDir dir;
	QString dirPath=Media::extractParentPath(loadConfigLineEdit->text());
	if(dirPath.isEmpty() || !dir.exists(dirPath)) dirPath=QDir::homePath();
	QString rep=QFileDialog::getOpenFileName(this,tr("Open config"),dirPath);
	if(!rep.isEmpty()) loadConfigLineEdit->setText(rep);
}


void Widget::parcourirCreateConfigSlot()
{
	QDir dir;
	QString dirPath=depotLocalPathLineEdit->text();
	if(dirPath.isEmpty() || !dir.exists(dirPath)) dirPath=QDir::homePath();
	QString rep=QFileDialog::getExistingDirectory(this,tr("Depot path"),dirPath);
	if(!rep.isEmpty()) depotLocalPathLineEdit->setText(rep);
}

void Widget::parcourirSaveConfigSlot()
{
	QDir dir;
	QString dirPath=Media::extractParentPath(saveConfigLineEdit->text());
	if(dirPath.isEmpty() || !dir.exists(dirPath)) dirPath=QDir::homePath();
	QString rep=QFileDialog::getSaveFileName(this,tr("Save config"),dirPath);
	if(!rep.isEmpty()) saveConfigLineEdit->setText(rep);
}






void Widget::loadConfigSlot()
{
	delete networkInterface;networkInterface=NULL;
	delete hddInterface;hddInterface=NULL;
	delete configurationData;configurationData=NULL;
	networkModel->removeRows(0,networkModel->rowCount());
	transfertModel->removeRows(0,transfertModel->rowCount());
	detectionModel->removeRows(0,detectionModel->rowCount());

	this->settings->setValue("loadConfigLineEdit",loadConfigLineEdit->text());

	//On charge la configuration
	this->configurationData=ConfigurationData::loadConfigurationData(loadConfigLineEdit->text(),detectionModel);
	if(configurationData)  addRowToTable("La configuration a bien été chargée",detectionModel,MSG_1);
	else addRowToTable("Echec du chargement de la configuration",detectionModel,MSG_1);

	//On créé l'interface réseau
	this->networkInterface=NetworkInterface::createNetworkInterface(configurationData?configurationData->getConfigurationNetwork():NULL,configurationData?configurationData->getConfigurationIdentification():NULL,networkModel);

	//On créé l'interface disque dur
	this->hddInterface=HddInterface::createHddInterface(configurationData,networkInterface,transfertModel);

	//On affiche l'onglet 2
	this->onglets->setCurrentIndex(1);

	if(networkInterface==NULL || configurationData==NULL || hddInterface==NULL)
	{
		QString msg=trUtf8("Les modules suivants n'ont pas été alloués correctement:");
		if(networkInterface==NULL) msg+="\n*NetworkInterface";
		if(configurationData==NULL) msg+="\n*ConfigurationData";
		if(hddInterface==NULL) msg+="\n*HddInterface";
		QMessageBox::critical(this,"Erreur",msg);
	}
	else
		QMessageBox::information(this,"Information",trUtf8("Tous les modules ont été alloués, et sont prêts à démarrer"));
}





void Widget::createConfigSlot()
{
	delete networkInterface;networkInterface=NULL;
	delete hddInterface;hddInterface=NULL;
	delete configurationData;configurationData=NULL;
	networkModel->removeRows(0,networkModel->rowCount());
	transfertModel->removeRows(0,transfertModel->rowCount());
	detectionModel->removeRows(0,detectionModel->rowCount());

	settings->setValue("serverAddressLineEdit",serverAddressLineEdit->text());
	settings->setValue("serverPortLineEdit",serverPortLineEdit->text());
	settings->setValue("loginLineEdit",loginLineEdit->text());
	settings->setValue("passwordLineEdit",passwordLineEdit->text());
	settings->setValue("depotLocalPathLineEdit",depotLocalPathLineEdit->text());
	settings->setValue("depotRealNameLineEdit",depotRealNameLineEdit->text());
	settings->setValue("saveConfigLineEdit",saveConfigLineEdit->text());
	settings->setValue("loadConfigLineEdit",saveConfigLineEdit->text());
	loadConfigLineEdit->setText(saveConfigLineEdit->text());

	//On créé la configuration réseau
	ConfigurationNetwork *configurationNetwork=ConfigurationNetwork::createConfigurationNetwork(serverAddressLineEdit->text(),serverPortLineEdit->text().toInt());
	if(configurationNetwork) addRowToTable("La configuration réseau a été crée",detectionModel,MSG_1);
	else addRowToTable("Echec de la création de la configuration réseau",detectionModel,MSG_1);

	//On créé la configuration d'identification
	ConfigurationIdentification *configurationIdentification=ConfigurationIdentification::createConfigurationIdentification(loginLineEdit->text(),passwordLineEdit->text());
	if(configurationIdentification) addRowToTable("La configuration d'identification a été crée",detectionModel,MSG_1);
	else addRowToTable("Echec de la création de la configuration d'indentification",detectionModel,MSG_1);

	//On créé la configuration de fichier
	QList<Depot*> *depots=new QList<Depot*>();
	Depot *d = Depot::createDepot(depotLocalPathLineEdit->text(),depotRealNameLineEdit->text(),0,false);depots->append(d);
	if(!d) addRowToTable("Echec de la création du repertoire 1",detectionModel,MSG_1);
	ConfigurationFile *configurationFile=ConfigurationFile::createConfigurationFile(depots,detectionModel);

	//On créé la configuration totale
	this->configurationData=ConfigurationData::createConfigurationData(configurationNetwork,configurationIdentification,configurationFile,saveConfigLineEdit->text());

	//On créé l'interface réseau
	this->networkInterface=NetworkInterface::createNetworkInterface(configurationNetwork,configurationIdentification,networkModel);

	//On créé l'interface disque dur
	this->hddInterface=HddInterface::createHddInterface(configurationData,networkInterface,transfertModel);

	//On affiche l'onglet 2
	this->onglets->setCurrentIndex(1);

	if(networkInterface==NULL || configurationData==NULL || hddInterface==NULL)
	{
		QString msg=trUtf8("Les modules suivants n'ont pas été alloués correctement:");
		if(networkInterface==NULL) msg+"\nNetworkInterface";
		if(configurationData==NULL) msg+"\nConfigurationData";
		if(hddInterface==NULL) msg+"\nHddInterface";
		QMessageBox::critical(this,"Erreur",msg);
	}
	else
		QMessageBox::information(this,"Information",trUtf8("Tous les modules ont été alloués, et sont prêts à démarrer"));
}








//Fonction qui crèe l'interface graphique
//avec les trois onglets, les boutons, les tableview
//et tout le bazard
void Widget::buildInterface()
{
	//Une taille minimale de 840*480 est nécéssaire pour une vue agréable
	setMinimumSize(settings->value("window/width",840).toInt(),settings->value("window/height",480).toInt());

	//L'entete des tableaux
	QStringList list;
	list<<trUtf8("Evenement")<<trUtf8("Heure");

	//On construit le QTabWidget, qui contiendra les onglets
	onglets=new QTabWidget(this);
	QHBoxLayout *layout=new QHBoxLayout();
	layout->addWidget(onglets);this->setLayout(layout);

	//On construit l'onglet des configuration
	configOnglet=new QWidget();

	//Le premier groupbox sert à charger une configuration
	QGroupBox *groupbox1 = new QGroupBox(trUtf8("Charger une configuration"), configOnglet);
	loadConfigLineEdit=new QLineEdit(settings->value("loadConfigLineEdit","/home/hky/test/config1.xml").toString());QPushButton *parcourirLoad=new QPushButton("Parcourir");
	QObject::connect(parcourirLoad,SIGNAL(clicked()),this,SLOT(parcourirLoadConfigSlot()));
	QHBoxLayout *l1=new QHBoxLayout();l1->addWidget(loadConfigLineEdit);l1->addWidget(parcourirLoad);
	QPushButton *loadButton=new QPushButton(trUtf8("Charger une configuration"));
	QObject::connect(loadButton,SIGNAL(clicked()),this,SLOT(loadConfigSlot()));
	QFormLayout *l2=new QFormLayout();l2->addRow("Chemin de la configuration",l1);l2->addWidget(loadButton);groupbox1->setLayout(l2);

	//Le deuxième groupbox sert à créer une configuration
	QGroupBox *groupbox2 = new QGroupBox(trUtf8("Créer une configuration"), configOnglet);
	QFormLayout *l3=new QFormLayout();
	serverAddressLineEdit=new QLineEdit(settings->value("serverAddressLineEdit","127.0.0.1").toString());l3->addRow(trUtf8("Adresse du serveur"),serverAddressLineEdit);
	serverPortLineEdit=new QLineEdit(settings->value("serverPortLineEdit","4321").toString());l3->addRow(trUtf8("Port du serveur"),serverPortLineEdit);
	loginLineEdit=new QLineEdit(settings->value("loginLineEdit","hky").toString());l3->addRow(trUtf8("Login d'identification"),loginLineEdit);
	passwordLineEdit=new QLineEdit(settings->value("passwordLineEdit","hky").toString());l3->addRow(trUtf8("Password d'identification"),passwordLineEdit);
	depotLocalPathLineEdit=new QLineEdit(settings->value("depotLocalPathLineEdit","/home/hky/test/A").toString());QPushButton *parcourirCreate=new QPushButton("Parcourir");
	QHBoxLayout *l4=new QHBoxLayout();l4->addWidget(depotLocalPathLineEdit);l4->addWidget(parcourirCreate);
	QObject::connect(parcourirCreate,SIGNAL(clicked()),this,SLOT(parcourirCreateConfigSlot()));
	l3->addRow(trUtf8("Chemin local du dépot"),l4);
	depotRealNameLineEdit=new QLineEdit(settings->value("depotRealNameLineEdit","projetdev/").toString());l3->addRow(trUtf8("Nom svn du dépot"),depotRealNameLineEdit);
	saveConfigLineEdit=new QLineEdit(settings->value("saveConfigLineEdit","/home/hky/test/config1.xml").toString());QPushButton *parcourirSave=new QPushButton("Parcourir");
	QHBoxLayout *l5=new QHBoxLayout();l5->addWidget(saveConfigLineEdit);l5->addWidget(parcourirSave);
	QObject::connect(parcourirSave,SIGNAL(clicked()),this,SLOT(parcourirSaveConfigSlot()));
	l3->addRow(trUtf8("Enregistrer la config sous..."),l5);
	QPushButton *createButton=new QPushButton(trUtf8("Créer une configuration"));
	QObject::connect(createButton,SIGNAL(clicked()),this,SLOT(createConfigSlot()));
	l3->addWidget(createButton);groupbox2->setLayout(l3);
	QVBoxLayout *layout0=new QVBoxLayout();layout0->addStretch();layout0->addWidget(groupbox1);layout0->addStretch();layout0->addWidget(groupbox2);
	layout0->addStretch();configOnglet->setLayout(layout0);
	onglets->addTab(configOnglet,trUtf8("Config"));

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
	QObject::connect(networkModel,SIGNAL(rowsInserted(QModelIndex,int,int)),networkView,SLOT(scrollToBottom()));
	layout1->addWidget(networkView);
	networkButton=new QPushButton(trUtf8("Se connecter/déconnecter au serveur"));
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
	QObject::connect(transfertModel,SIGNAL(rowsInserted(QModelIndex,int,int)),transfertView,SLOT(scrollToBottom()));
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
	QObject::connect(detectionModel,SIGNAL(rowsInserted(QModelIndex,int,int)),detectionView,SLOT(scrollToBottom()));
	layout3->addWidget(detectionView);
	detectionButton=new QPushButton(trUtf8("Démarer/Arreter les détections"));
	QObject::connect(detectionButton,SIGNAL(clicked()),this,SLOT(detectionButtonSlot()));
	layout3->addWidget(detectionButton);
	detectionOnglet->setLayout(layout3);
	onglets->addTab(detectionOnglet,trUtf8("Détections"));

}






//Juste une méthode statique qui écrit un évenement dans le model, et l'heure р laquelle il s'est réalisé
void Widget::addRowToTable(QString s, QStandardItemModel *model, QColor color)
{
	if(model==NULL) return;
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



