#include "widget.h"


extern QString APPLICATION_NAME;




//////////////////////////////////////////////////////////
/////////         POUR L'INTERFACE GENERALE
//////////////////////////////////////////////////////////

Widget::Widget(): QWidget()
{
	this->settings=new QSettings("Server-"+APPLICATION_NAME, "Server-"+APPLICATION_NAME);
	buildInterface();
	this->show();
	showLoadConfig();
}

void Widget::buildInterface()
{
	this->setWindowTitle("Serveur-"+APPLICATION_NAME);
	setMinimumSize(settings->value("window/width",840).toInt(),settings->value("window/height",480).toInt());
	onglets=new QTabWidget(this);
	QHBoxLayout *layoutPrinc=new QHBoxLayout();
	layoutPrinc->addWidget(onglets);this->setLayout(layoutPrinc);

	//On construit l'onglet général
	QWidget *generalOnglet=new QWidget();
	QTableView *tableView=new QTableView();
	tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	this->model=new QStandardItemModel(0,2,tableView);
	QStringList list;
	list<<trUtf8("Evenement")<<trUtf8("Heure");
	model->setHorizontalHeaderLabels(list);
	tableView->setModel(model);
	tableView->horizontalHeader()->setStretchLastSection(true);
	tableView->setColumnWidth(0,650);
	QHBoxLayout *layout = new QHBoxLayout;
	layout->addWidget(tableView);
	generalOnglet->setLayout(layout);
	onglets->addTab(generalOnglet,trUtf8("Général"));
	addRowToTable("Démarage de l'application",model,MSG_1);

	//On initialise les dialogues
	this->loadConfigDialog=NULL;
	this->createConfigDialog=NULL;
}






//////////////////////////////////////////////////////////
/////////         CHARGER UNE CONFIG
//////////////////////////////////////////////////////////

void Widget::showLoadConfig()
{
	if(createConfigDialog!=NULL)
	{
		delete createConfigDialog;
		createConfigDialog=NULL;
	}
	loadConfigDialog=new QDialog(this);
	loadConfigDialog->setModal(true);
	QVBoxLayout *layoutPrinc=new QVBoxLayout(loadConfigDialog);

	QGroupBox *boxDB=new QGroupBox(trUtf8("Charger une base de données"),loadConfigDialog);
	QFormLayout *layoutDB=new QFormLayout();
	dbNameLineEdit=new QLineEdit(settings->value("dbNameLineEdit","").toString(),loadConfigDialog);
	layoutDB->addRow(trUtf8("Nom de la base de données"),dbNameLineEdit);
	dbUserLineEdit=new QLineEdit(settings->value("dbUserLineEdit","").toString(),loadConfigDialog);
	layoutDB->addRow(trUtf8("User de la base de données"),dbUserLineEdit);
	dbPasswordLineEdit=new QLineEdit(settings->value("dbPasswordLineEdit","").toString(),loadConfigDialog);
	layoutDB->addRow(trUtf8("Password de la base de données"),dbPasswordLineEdit);
	boxDB->setLayout(layoutDB);
	layoutPrinc->addWidget(boxDB);

	QHBoxLayout *l6=new QHBoxLayout();
	QPushButton *load=new QPushButton(trUtf8("Charger cette BDD"),loadConfigDialog);l6->addWidget(load);
	QPushButton *create=new QPushButton(trUtf8("Créer une nouvelle BDD"),loadConfigDialog);l6->addWidget(create);
	QPushButton *quitter=new QPushButton(trUtf8("Quitter"),loadConfigDialog);l6->addWidget(quitter);
	QObject::connect(load,SIGNAL(clicked()),this,SLOT(validerLoadConfig()));
	QObject::connect(create,SIGNAL(clicked()),this,SLOT(showCreateConfig()));
	QObject::connect(quitter,SIGNAL(clicked()),loadConfigDialog,SLOT(close()));
	layoutPrinc->addLayout(l6);
	loadConfigDialog->setLayout(layoutPrinc);
	loadConfigDialog->adjustSize();
	loadConfigDialog->show();
}


void Widget::validerLoadConfig()
{
	settings->setValue("dbNameLineEdit",dbNameLineEdit->text());
	settings->setValue("dbUserLineEdit",dbUserLineEdit->text());
	settings->setValue("dbPasswordLineEdit",dbPasswordLineEdit->text());
	model->removeRows(0,model->rowCount());
	loadConfigDialog->close();

	//On créé la bdd
	DatabaseManager *db=DatabaseManager::loadDatabaseManager(dbNameLineEdit->text(),dbPasswordLineEdit->text(),dbUserLineEdit->text());
	if(db) addRowToTable("La base de données a bien été chargée",model,MSG_1);
	else
	{
		addRowToTable("Erreur au chargement de la base de données. Vérifiez le chemin",model,MSG_1);
		loadConfigDialog->show();
		return;
	}

	//On récupère les infos générales
	SqlGeneralInfos *infos=db->getGeneralInfos();
	if(!infos)
	{
		addRowToTable("Erreur: Pas d'infos générales trouvées dans la base de données",model,MSG_1);
		loadConfigDialog->show();
		delete db;
		return;
	}

	//On créé le svnManager
	SvnManager *svn=SvnManager::createSvnManager(infos->svnAddress,infos->svnUser,infos->svnPassword,"svn","svnadmin");
	if(svn) addRowToTable("Le gestionnaire SVN a été créé",model,MSG_1);
	else
	{
		addRowToTable("Erreur à la création du gestionnaire SVN",model,MSG_1);
		loadConfigDialog->show();
		delete db;delete infos;
		return;
	}

	//On créé le fileManager
	FileManager *f=FileManager::createFileManager(infos->filesPath,svn);
	if(f) addRowToTable("Le gestionnaire de fichiers a été créé",model,MSG_1);
	else
	{
		addRowToTable("Erreur à la création du gestionnaire de fichiers",model,MSG_1);
		loadConfigDialog->show();
		delete db;delete infos;delete svn;
		return;
	}

	QList<QString> depots=db->getDepotNames();
	int i;
	for(i=0;i<depots.length();i++)
	{
		if(f->addDepot(depots.at(i))) addRowToTable("Le dépot "+depots.at(i)+" a bien été chargé",model,MSG_1);
		else
		{
			addRowToTable("Erreur lors du chargement du dépot "+depots.at(i),model,MSG_1);
			loadConfigDialog->show();
			delete db;delete infos;delete f;
			return;
		}
	}
	if(i==0) addRowToTable("Aucun dépot n'a été trouvé dans la base de données",model,MSG_1);

	//On créé l'interface réseau
	this->server=Server::createServer(db,f,onglets,model);
	bool result=this->server->beginListenning(infos->serverPort);
	if(result) addRowToTable("Le serveur est démarré sur le port "+QString::number(infos->serverPort),model,MSG_1);
	else
	{
		addRowToTable("Echec de démarrage du serveur sur le port "+QString::number(infos->serverPort),model,MSG_1);
		loadConfigDialog->show();
		delete db;delete infos;delete svn;
		return;
	}
	delete loadConfigDialog;
	loadConfigDialog=NULL;
}






//////////////////////////////////////////////////////////
/////////         CREER UNE CONFIG
//////////////////////////////////////////////////////////


void Widget::showCreateConfig()
{
	if(loadConfigDialog!=NULL)
	{
		delete loadConfigDialog;
		loadConfigDialog=NULL;
	}
	createConfigDialog=new QDialog(this);
	createConfigDialog->setModal(true);
	QVBoxLayout *layoutPrinc=new QVBoxLayout(createConfigDialog);

	QGroupBox *boxGeneral=new QGroupBox(trUtf8("Infos Générales"),createConfigDialog);
	QFormLayout *layoutGeneral=new QFormLayout();
	portLineEdit=new QLineEdit(settings->value("portLineEdit","").toString(),createConfigDialog);
	layoutGeneral->addRow(trUtf8("Port d'écoute"),portLineEdit);
	QHBoxLayout *l1=new QHBoxLayout();
	filesPathLineEdit=new QLineEdit(settings->value("filesPathLineEdit","").toString(),createConfigDialog);QPushButton *parcourirFilesPath=new QPushButton("parcourir",createConfigDialog);
	QObject::connect(parcourirFilesPath,SIGNAL(clicked()),this,SLOT(parcourirFilesPath()));
	l1->addWidget(filesPathLineEdit);l1->addWidget(parcourirFilesPath);
	layoutGeneral->addRow(trUtf8("Chemin des fichiers temporaires"),l1);
	boxGeneral->setLayout(layoutGeneral);
	layoutPrinc->addWidget(boxGeneral);

	QGroupBox *boxSvn=new QGroupBox(trUtf8("Infos SVN"),createConfigDialog);
	QFormLayout *layoutSvn=new QFormLayout();
	svnAddressLineEdit=new QLineEdit(settings->value("svnAddressLineEdit","").toString(),createConfigDialog);
	layoutSvn->addRow(trUtf8("Adresse du serveur subversion"),svnAddressLineEdit);
	svnPortLineEdit=new QLineEdit(settings->value("svnPortLineEdit","").toString(),createConfigDialog);
	layoutSvn->addRow(trUtf8("Port d'écoute du subversion"),svnPortLineEdit);
	svnUserLineEdit=new QLineEdit(settings->value("svnUserLineEdit","").toString(),createConfigDialog);
	layoutSvn->addRow(trUtf8("Utilisateur svn"),svnUserLineEdit);
	svnPasswordLineEdit=new QLineEdit(settings->value("svnPasswordLineEdit","").toString(),createConfigDialog);
	layoutSvn->addRow(trUtf8("Mot de passe svn"),svnPasswordLineEdit);
	boxSvn->setLayout(layoutSvn);
	layoutPrinc->addWidget(boxSvn);

	QGroupBox *boxDB=new QGroupBox(trUtf8("Base de données"),createConfigDialog);
	QFormLayout *layoutDB=new QFormLayout();
	dbNameLineEdit=new QLineEdit(settings->value("dbNameLineEdit","").toString(),createConfigDialog);
	layoutDB->addRow(trUtf8("Nom de la base de données"),dbNameLineEdit);
	dbUserLineEdit=new QLineEdit(settings->value("dbUserLineEdit","").toString(),createConfigDialog);
	layoutDB->addRow(trUtf8("User de la base de données"),dbUserLineEdit);
	dbPasswordLineEdit=new QLineEdit(settings->value("dbPasswordLineEdit","").toString(),createConfigDialog);
	layoutDB->addRow(trUtf8("Password de la base de données"),dbPasswordLineEdit);
	boxDB->setLayout(layoutDB);
	layoutPrinc->addWidget(boxDB);

	QGroupBox *boxUsers=new QGroupBox(trUtf8("Utilisateurs"),createConfigDialog);
	QHBoxLayout *l4=new QHBoxLayout();
	tableUsers=new QTableWidget(0,3);
	tableUsers->setEditTriggers(QAbstractItemView::NoEditTriggers);
	tableUsers->horizontalHeader()->setStretchLastSection(true);
	QStringList list2;list2<<"Login"<<"Password"<<"Super-Admin?";
	tableUsers->setHorizontalHeaderLabels(list2);
	tableUsers->setColumnWidth(0,140);tableUsers->setColumnWidth(1,140);
	tableUsers->setMinimumWidth(410);tableUsers->setMinimumHeight(110);
	QObject::connect(tableUsers,SIGNAL(itemChanged(QTableWidgetItem*)),tableUsers,SLOT(scrollToBottom()));
	l4->addWidget(tableUsers);
	QVBoxLayout *l5=new QVBoxLayout();
	QPushButton *ajouterUser=new QPushButton("ajouter",createConfigDialog);l5->addWidget(ajouterUser);
	QPushButton *enleverUser=new QPushButton("enlever",createConfigDialog);l5->addWidget(enleverUser);
	QObject::connect(ajouterUser,SIGNAL(clicked()),this,SLOT(ajouterUser()));
	QObject::connect(enleverUser,SIGNAL(clicked()),this,SLOT(enleverUser()));
	l4->addLayout(l5);
	boxUsers->setLayout(l4);
	layoutPrinc->addWidget(boxUsers);

	QGroupBox *boxDepots=new QGroupBox(trUtf8("Dépots à synchroniser"),createConfigDialog);
	QHBoxLayout *l2=new QHBoxLayout();
	tableDepots=new QTableWidget(0,3,createConfigDialog);
	tableDepots->setEditTriggers(QAbstractItemView::NoEditTriggers);
	tableDepots->horizontalHeader()->setStretchLastSection(true);
	QStringList list1;list1<<trUtf8("Nom dépot")<<trUtf8("Logins admins")<<trUtf8("Logins users");
	tableDepots->setHorizontalHeaderLabels(list1);
	tableDepots->setColumnWidth(0,110);tableDepots->setColumnWidth(1,150);
	tableDepots->setMinimumWidth(410);tableDepots->setMinimumHeight(110);
	QObject::connect(tableDepots,SIGNAL(itemChanged(QTableWidgetItem*)),tableDepots,SLOT(scrollToBottom()));
	l2->addWidget(tableDepots);
	QVBoxLayout *l3=new QVBoxLayout();
	QPushButton *ajouterDepot=new QPushButton("ajouter",createConfigDialog);l3->addWidget(ajouterDepot);
	QPushButton *enleverDepot=new QPushButton("enlever",createConfigDialog);l3->addWidget(enleverDepot);
	QObject::connect(ajouterDepot,SIGNAL(clicked()),this,SLOT(ajouterDepot()));
	QObject::connect(enleverDepot,SIGNAL(clicked()),this,SLOT(enleverDepot()));
	l2->addLayout(l3);
	boxDepots->setLayout(l2);
	layoutPrinc->addWidget(boxDepots);

	QHBoxLayout *l6=new QHBoxLayout();
	QPushButton *create=new QPushButton(trUtf8("Créer cette BDD"),createConfigDialog);l6->addWidget(create);
	QPushButton *load=new QPushButton(trUtf8("Charger une autre BDD"),createConfigDialog);l6->addWidget(load);
	QPushButton *quitter=new QPushButton(trUtf8("Quitter"),createConfigDialog);l6->addWidget(quitter);
	QObject::connect(create,SIGNAL(clicked()),this,SLOT(validerCreateConfig()));
	QObject::connect(load,SIGNAL(clicked()),this,SLOT(showLoadConfig()));
	QObject::connect(quitter,SIGNAL(clicked()),createConfigDialog,SLOT(close()));
	layoutPrinc->addLayout(l6);
	createConfigDialog->setLayout(layoutPrinc);
	createConfigDialog->adjustSize();
	createConfigDialog->show();
}




void Widget::validerCreateConfig()
{
	model->removeRows(0,model->rowCount());
	settings->setValue("dbNameLineEdit",dbNameLineEdit->text());
	settings->setValue("dbUserLineEdit",dbUserLineEdit->text());
	settings->setValue("dbPasswordLineEdit",dbPasswordLineEdit->text());
	settings->setValue("portLineEdit",portLineEdit->text());
	settings->setValue("filesPathLineEdit",filesPathLineEdit->text());
	settings->setValue("svnAddressLineEdit",svnAddressLineEdit->text());
	settings->setValue("svnUserLineEdit",svnUserLineEdit->text());
	settings->setValue("svnPasswordLineEdit",svnPasswordLineEdit->text());
	settings->setValue("svnPortLineEdit",svnPortLineEdit->text());
	createConfigDialog->close();

	//On créé la bdd
	DatabaseManager *db=DatabaseManager::createDatabaseManager(dbNameLineEdit->text(),dbPasswordLineEdit->text(),dbUserLineEdit->text());
	if(db) addRowToTable("La base de données a bien été créé",model,MSG_1);
	else
	{
		addRowToTable("Erreur à la création de la base de données. Vérifiez le chemin",model,MSG_1);
		createConfigDialog->show();
		return;
	}

	if(!db->setGeneralInfos(portLineEdit->text().toInt(),filesPathLineEdit->text(),svnAddressLineEdit->text(),svnUserLineEdit->text(),svnPasswordLineEdit->text(),svnPortLineEdit->text().toInt()))
	{
		addRowToTable("Erreur lors d'une écriture dans la base de données", model,MSG_1);
		createConfigDialog->show();
		return;
	}

	//On créé le svnManager
	SvnManager *svn=SvnManager::createSvnManager(svnAddressLineEdit->text(),svnUserLineEdit->text(),svnPasswordLineEdit->text(),"svn","svnadmin");
	if(svn) addRowToTable("Le gestionnaire SVN a été créé",model,MSG_1);
	else
	{
		addRowToTable("Erreur à la création du gestionnaire SVN",model,MSG_1);
		createConfigDialog->show();
		delete db;
		return;
	}

	//On créé le fileManager
	FileManager *f=FileManager::createFileManager(filesPathLineEdit->text(),svn);
	if(f) addRowToTable("Le gestionnaire de fichiers a été créé",model,MSG_1);
	else
	{
		addRowToTable("Erreur à la création du gestionnaire de fichiers",model,MSG_1);
		addRowToTable("Vérifiez le chemin des fichiers temporaires",model,MSG_1);
		loadConfigDialog->show();
		delete db;delete svn;
		return;
	}

	int i;
	for(i=0;i<tableUsers->rowCount();i++)
	{
		if(!db->createUser(tableUsers->item(i,0)->text(),tableUsers->item(i,1)->text(),"",""))
		{
			addRowToTable("Erreur lors de la création de l'utilisateur "+tableUsers->item(i,0)->text(),model,MSG_1);
			continue;
		}
		else
		{
			if(tableUsers->item(i,2)->text().toLower()=="true")
				if(!db->setSuperAdmin(tableUsers->item(i,0)->text(),true))
					addRowToTable("Erreur lors de l'affectation des droits de super-administrateur à "+tableUsers->item(i,0)->text(),model,MSG_1);
		}
	}
	if(i==0) addRowToTable("Aucun utilisateur n'a été créé",model,MSG_1);

	for(i=0;i<tableDepots->rowCount();i++)
	{
		if(!db->createDepot(tableDepots->item(i,0)->text()))
		{
			addRowToTable("Erreur lors de la création du dépot "+tableDepots->item(i,0)->text(),model,MSG_1);
			continue;
		}
		QStringList admins=tableDepots->item(i,1)->text().split(";");
		for(int j=0;j<admins.size();j++)
		{
			if(!admins.at(j).isEmpty())
			{
				if(!db->setAdmin(admins.at(j),tableDepots->item(i,0)->text(),true))
					addRowToTable("Erreur lors de l'affectation des droits d'administrateur à "+admins.at(j),model,MSG_1);
			}
		}
		QStringList users=tableDepots->item(i,2)->text().split(";");
		for(int k=0;k<users.size();k++)
		{
			if(!users.at(k).isEmpty())
			{
				if(!db->setUtilisation(users.at(k),tableDepots->item(i,0)->text(),true))
					addRowToTable("Erreur lors de l'affectation des droits d'utilisateur à "+users.at(k),model,MSG_1);
			}
		}
		if(!f->addDepot(tableDepots->item(i,0)->text()))
		{
			addRowToTable("Erreur lors du chargement du dépot "+tableDepots->item(i,0)->text(),model,MSG_1);
		}
	}
	if(i==0) addRowToTable("Aucun dépot n'a été créé",model,MSG_1);

	//On créé l'interface réseau
	this->server=Server::createServer(db,f,onglets,model);
	bool result=this->server->beginListenning(portLineEdit->text().toInt());
	if(result) addRowToTable("Le serveur est démarré sur le port "+QString::number(portLineEdit->text().toInt()),model,MSG_1);
	else
	{
		addRowToTable("Echec de démarrage du serveur sur le port "+QString::number(portLineEdit->text().toInt()),model,MSG_1);
		addRowToTable("Vérifiez que le port n'est pas déjà utilisé ou simplement protégé",model,MSG_1);
		loadConfigDialog->show();
		delete db;delete svn;
		return;
	}
	delete createConfigDialog;
	createConfigDialog=NULL;
}







/////////////////////////////////////////////////////
/////      AUTRES METHODES
/////////////////////////////////////////////////////


//Juste une méthode statique qui écrit un évenement dans le model, et l'heure р laquelle il s'est réalisé
void Widget::addRowToTable(QString sentence, QStandardItemModel *model, QColor color)
{
	//On récupère la liste des 2 colonnes de la lignes
	QList<QStandardItem*> list;
	QStandardItem *i1=new QStandardItem(trUtf8(sentence.toAscii()));
	i1->setBackground(QBrush(color));
	QStandardItem *i2=new QStandardItem(QTime::currentTime().toString("hh:mm:ss"));
	i2->setBackground(QBrush(color));
	list << i1 << i2;
	//On ajoute la ligne
	model->appendRow(list);
}


void Widget::parcourirFilesPath()
{
	QDir dir;
	QString dirPath=Depot::extractParentPath(filesPathLineEdit->text());
	if(dirPath.isEmpty() || !dir.exists(dirPath)) dirPath=QDir::homePath();
	QString rep=QFileDialog::getExistingDirectory(createConfigDialog,tr("Files path"),dirPath);
	if(!rep.isEmpty()) filesPathLineEdit->setText(rep);
}


void Widget::ajouterDepot()
{
	QDialog dialog(createConfigDialog);
	dialog.setModal(true);
	QFormLayout layout;
	QLineEdit depotName;

	QHBoxLayout *layoutAdmins=new QHBoxLayout();
	QListWidget *listAdmins=new QListWidget();layoutAdmins->addWidget(listAdmins);listAdmins->setMaximumHeight(110);
	listAdmins->setSelectionMode(QAbstractItemView::MultiSelection);
	for(int i=0;i<tableUsers->rowCount();i++) listAdmins->addItem(new QListWidgetItem(tableUsers->item(i,0)->text()));
	QGroupBox admins(trUtf8("Sélectionnez les administrateurs"));admins.setLayout(layoutAdmins);

	QHBoxLayout *layoutUsers=new QHBoxLayout();
	QListWidget *listUsers=new QListWidget();layoutUsers->addWidget(listUsers);listUsers->setMaximumHeight(110);
	listUsers->setSelectionMode(QAbstractItemView::MultiSelection);
	for(int i=0;i<tableUsers->rowCount();i++) listUsers->addItem(new QListWidgetItem(tableUsers->item(i,0)->text()));
	QGroupBox users(trUtf8("Sélectionnez les utilisateurs"));users.setLayout(layoutUsers);

	QPushButton ok("Valider");QObject::connect(&ok,SIGNAL(clicked()),&dialog,SLOT(hide()));
	layout.addRow(trUtf8("Nom du dépot"),&depotName);
	layout.addRow(&admins);
	layout.addRow(&users);
	layout.addRow(&ok);
	dialog.setLayout(&layout);
	dialog.exec();
	if(depotName.text().isEmpty()) return ;
	QString strAdmins;
	for(int i=0;i<listAdmins->count();i++) if(listAdmins->item(i)->isSelected()) strAdmins=strAdmins+listAdmins->item(i)->text()+";";
	QString strUsers;
	for(int i=0;i<listUsers->count();i++) if(listUsers->item(i)->isSelected()) strUsers=strUsers+listUsers->item(i)->text()+";";
	int row=tableDepots->rowCount();
	tableDepots->insertRow(row);
	tableDepots->setItem(row,0,new QTableWidgetItem(depotName.text()));
	tableDepots->setItem(row,1,new QTableWidgetItem(strAdmins));
	tableDepots->setItem(row,2,new QTableWidgetItem(strUsers));
}


void Widget::enleverDepot()
{
	tableDepots->removeRow(tableDepots->currentRow());
}


void Widget::ajouterUser()
{
	QDialog dialog(createConfigDialog);
	dialog.setModal(true);
	QFormLayout layout;
	QLineEdit login;
	QLineEdit password1;password1.setEchoMode(QLineEdit::Password);
	QCheckBox superAdmin;
	QPushButton ok("Valider");QObject::connect(&ok,SIGNAL(clicked()),&dialog,SLOT(hide()));
	layout.addRow("Login user",&login);
	layout.addRow("Password user",&password1);
	layout.addRow("Super Admin?",&superAdmin);
	layout.addRow(&ok);
	dialog.setLayout(&layout);
	dialog.exec();
	if(login.text().isEmpty())
	{
		QMessageBox::critical(createConfigDialog,"Paramètres",trUtf8("Les paramètres entrés sont incorrects"));
		return;
	}
	for(int i=0;i<tableUsers->rowCount();i++)
	{
		if(tableUsers->item(i,0)->text()==login.text())
		{
			QMessageBox::critical(createConfigDialog,"Login existant",trUtf8("Ce login existe déjà dans le tableau"));
			return;
		}
	}
	int row=tableUsers->rowCount();
	tableUsers->insertRow(row);
	tableUsers->setItem(row,0,new QTableWidgetItem(login.text()));
	tableUsers->setItem(row,1,new QTableWidgetItem(password1.text()));
	tableUsers->setItem(row,2,new QTableWidgetItem(superAdmin.isChecked()?"True":"False"));
}


void Widget::enleverUser()
{
	tableUsers->removeRow(tableUsers->currentRow());
}
