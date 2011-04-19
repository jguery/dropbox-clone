#include "databasemanager.h"




DatabaseManager *DatabaseManager::createDatabaseManager(QString name, QString login, QString password)
{
	QSqlDatabase *db=new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE"));
	db->setHostName("localhost");
	db->setDatabaseName(name);
	db->setUserName(login);
	db->setPassword(password);
	if(db->open())
	{
		QSqlQuery q(*db);
		if(!q.exec("drop table user;") || !q.exec("create table user (login varchar(50),password varchar(50),firstname varchar(50),lastname varchar(50),inscriptiondate datetime);") ||
		!q.exec("drop table admin;") || !q.exec("create table admin(login varchar(50),depotname varchar(50),inscriptiondate datetime);") ||
		!q.exec("drop table superadmin;") || !q.exec("create table superadmin(login varchar(50));") ||
		!q.exec("drop table depot;") || !q.exec("create table depot(depotname varchar(50),inscriptiondate datetime);") ||
		!q.exec("drop table utilisation;") || !q.exec("create table utilisation(login varchar(50),depotname varchar(50),access varchar(50),inscriptiondate datetime);"))
		return NULL;
		q.exec("insert into user(login,password) values('hky','hky');");
		q.exec("insert into user(login,password) values('jguery','jguery');");
		q.exec("insert into user(login,password) values('ymahe','ymahe');");
		return new DatabaseManager(db);
	}
	delete db;
	return NULL;
}





DatabaseManager *DatabaseManager::loadDatabaseManager(QString name, QString login, QString password)
{
	QSqlDatabase *db=new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE"));
	db->setHostName("localhost");
	db->setDatabaseName(name);
	db->setUserName(login);
	db->setPassword(password);
	if(db->open())
	{
		QSqlQuery q(*db);
		if(!q.exec("create table if not exists user (login varchar(50),password varchar(50),firstname varchar(50),lastname varchar(50),inscriptiondate datetime);") ||
		!q.exec("create table if not exists admin(login varchar(50),depotname varchar(50),inscriptiondate datetime);") ||
		!q.exec("create table if not exists superadmin(login varchar(50));") ||
		!q.exec("create table if not exists depot(depotname varchar(50),inscriptiondate datetime);") ||
		!q.exec("create table if not exists utilisation(login varchar(50),depotname varchar(50),access varchar(50),inscriptiondate datetime);"))
		return NULL;
		else
		return new DatabaseManager(db);
	}
	delete db;
	return NULL;
}





bool DatabaseManager::isUserExists(QString login)
{
	mutex.lock();
	QSqlQuery q(*db);
	q.exec("select * from user where login='"+login+"';");
	if(q.next())
	{
		mutex.unlock();
		return true;
	}
	mutex.unlock();
	return false;
}




bool DatabaseManager::isDepotExists(QString depotname)
{
	mutex.lock();
	QSqlQuery q(*db);
	q.exec("select * from depot where depotname='"+depotname+"';");
	if(q.next())
	{
		mutex.unlock();
		return true;
	}
	mutex.unlock();
	return false;
}



bool DatabaseManager::authentificateUser(QString login,QString password)
{
	mutex.lock();
	QSqlQuery q(*db);
	q.exec("select * from user where login='"+login+"' and password='"+password+"';");
	if(q.next())
	{
		mutex.unlock();
		return true;
	}
	mutex.unlock();
	return false;
}





bool DatabaseManager::userIsLinkDepot(QString login,QString depotname)
{
	mutex.lock();
	QSqlQuery q(*db);
	q.exec("select * from utilisation where login='"+login+"' and depotname='"+depotname+"';");
	if(q.next())
	{
		mutex.unlock();
		return true;
	}
	mutex.unlock();
	return false;
}




bool DatabaseManager::authentificateAdminDepot(QString login,QString password,QString depotname)
{
	if(!authentificateUser(login,password)) return false;
	mutex.lock();
	QSqlQuery q(*db);
	q.exec("select * from admin where depotname='"+depotname+"' and login='"+login+"'");
	if(q.next())
	{
		mutex.unlock();
		return true;
	}
	mutex.unlock();
	return false;
}




bool DatabaseManager::authentificateSuperAdmin(QString login,QString password)
{
	if(!authentificateUser(login,password)) return false;
	mutex.lock();
	QSqlQuery q(*db);
	q.exec("select * from superadmin where login='"+login+"'");
	if(q.next())
	{
		mutex.unlock();
		return true;
	}
	mutex.unlock();
	return false;
}




SqlUser *DatabaseManager::getUser(QString login)
{
	mutex.lock();
	QSqlQuery q(*db);
	q.exec("select login, password, firstname, lastname, inscriptiondate from user where login='"+login+"';");
	if(!q.next())
	{
		mutex.unlock();
		return NULL;
	}

	SqlUser *user=new SqlUser;
	user->login=q.value(0).toString();
	user->password=q.value(1).toString();
	user->firstName=q.value(2).toString();
	user->lastName=q.value(3).toString();
	user->inscriptionDate=q.value(4).toDateTime();

	mutex.unlock();
	return user;
}




SqlDepot *DatabaseManager::getDepot(QString depotname)
{
	mutex.lock();
	QSqlQuery q(*db);
	q.exec("select depotname, inscriptiondate from depot where depotname='"+depotname+"';");
	if(!q.next())
	{
		mutex.unlock();
		return NULL;
	}

	SqlDepot *depot=new SqlDepot;
	depot->depotname=q.value(0).toString();
	depot->inscriptionDate=q.value(1).toDateTime();

	mutex.unlock();
	return depot;
}




QList<SqlUser*> DatabaseManager::getUsers()
{
	mutex.lock();
	QSqlQuery q(*db);
	q.exec("select login, password, firstname, lastname, inscriptiondate from user;");
	QList<SqlUser*> users;
	while(q.next())
	{
		SqlUser *user=new SqlUser;
		user->login=q.value(0).toString();
		user->password=q.value(1).toString();
		user->firstName=q.value(2).toString();
		user->lastName=q.value(3).toString();
		user->inscriptionDate=q.value(4).toDateTime();
		users.append(user);
	}
	mutex.unlock();
	return users;
}




QList<SqlDepot*> DatabaseManager::getDepots()
{
	mutex.lock();
	QSqlQuery q(*db);
	q.exec("select depotname, inscriptiondate from depot;");
	QList<SqlDepot*> depots;
	while(q.next())
	{
		SqlDepot *depot=new SqlDepot;
		depot->depotname=q.value(0).toString();
		depot->inscriptionDate=q.value(1).toDateTime();
		depots.append(depot);
	}
	mutex.unlock();
	return depots;
}




QList<QString> DatabaseManager::getPseudoAdmins()
{
	mutex.lock();
	QSqlQuery q(*db);
	q.exec("select login from admin;");
	QList<QString> admins;
	while(q.next())
	{
		admins.append(q.value(0).toString());
	}
	mutex.unlock();
	return admins;
}





QList<QString> DatabaseManager::getPseudoSuperAdmins()
{
	mutex.lock();
	QSqlQuery q(*db);
	q.exec("select login from superadmin;");
	QList<QString> superadmins;
	while(q.next())
	{
		superadmins.append(q.value(0).toString());
	}
	mutex.unlock();
	return superadmins;
}




DatabaseManager::DatabaseManager(QSqlDatabase *db)
{
	this->db=db;
}
