#include "databasemanager.h"




DatabaseManager *DatabaseManager::createDatabaseManager(QString name, QString login, QString password)
{
	if(name.isEmpty()) return NULL;
	QSqlDatabase *db=new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE"));
	db->setHostName("localhost");
	db->setDatabaseName(name);
	db->setUserName(login);
	db->setPassword(password);
	if(db->open())
	{
		QSqlQuery q(*db);
		q.exec("drop table infos;");q.exec("drop table user;");q.exec("drop table admin;");
		q.exec("drop table superadmin;");q.exec("drop table depot;");q.exec("drop table utilisation;");
		bool error=(
		!q.exec("create table infos(serverPort integer,filesPath varchar(200),svnAddress varchar(200),svnUser varchar(200),svnPassword varchar(200),svnPort integer);") ||
		!q.exec("create table user (login varchar(200),password varchar(200),firstname varchar(200),lastname varchar(200),inscriptiondate datetime);") ||
		!q.exec("create table admin(login varchar(200),depotname varchar(200),inscriptiondate datetime);") ||
		!q.exec("create table superadmin(login varchar(200),inscriptiondate datetime);") ||
		!q.exec("create table depot(depotname varchar(200),inscriptiondate datetime);") ||
		!q.exec("create table utilisation(login varchar(200),depotname varchar(200),readonly varchar(200),inscriptiondate datetime);"));
		if(error) return NULL;
		return new DatabaseManager(db);
	}
	delete db;
	return NULL;
}





DatabaseManager *DatabaseManager::loadDatabaseManager(QString name, QString login, QString password)
{
	if(name.isEmpty()) return NULL;
	QFile file(name); if(!file.exists()) return NULL;
	QSqlDatabase *db=new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE"));
	db->setHostName("localhost");
	db->setDatabaseName(name);
	db->setUserName(login);
	db->setPassword(password);
	if(db->open())
	{
		QSqlQuery q(*db);
		q.exec("create table if not exists infos(serverPort integer,filesPath varchar(200),svnAddress varchar(200),svnUser varchar(200),svnPassword varchar(200),svnPort integer);") ||
		q.exec("create table if not exists user (login varchar(200),password varchar(200),firstname varchar(200),lastname varchar(200),inscriptiondate datetime);") ||
		q.exec("create table if not exists admin(login varchar(200),depotname varchar(200),inscriptiondate datetime);") ||
		q.exec("create table if not exists superadmin(login varchar(200),inscriptiondate datetime);") ||
		q.exec("create table if not exists depot(depotname varchar(200),inscriptiondate datetime);") ||
		q.exec("create table if not exists utilisation(login varchar(200),depotname varchar(200),readonly varchar(200),inscriptiondate datetime);");
		return new DatabaseManager(db);
	}
	delete db;
	return NULL;
}





///////////////////////////////////////////////////////////
/////       FONCTIONS D'ADMINISTRATION DES INFOS
///////////////////////////////////////////////////////////

bool DatabaseManager::setGeneralInfos(int serverPort,QString filesPath,QString svnAddress,QString svnUser,QString svnPassword,int svnPort)
{
	mutex->lock();
	QSqlQuery q(*db);
	q.exec("delete from infos;");
	bool result=q.exec("insert into infos(serverPort,filesPath,svnAddress,svnUser,svnPassword,svnPort) values("+QString::number(serverPort)+",'"+filesPath+"','"+svnAddress+"','"+svnUser+"','"+svnPassword+"',"+QString::number(svnPort)+");");
	mutex->unlock();
	return result;
}




///////////////////////////////////////////////////////////
/////       FONCTIONS D'ADMINISTRATION DES USERS
///////////////////////////////////////////////////////////

bool DatabaseManager::createUser(QString login,QString password,QString firstName,QString lastName)
{
	if(login.isEmpty() || password.isEmpty()) return false;
	password=DatabaseManager::hashPassword(password);
	if(isUserExists(login)) return false;
	mutex->lock();
	QSqlQuery q(*db);
	bool result=q.exec("insert into user(login,password,firstName,lastName,inscriptionDate) values('"+login+"','"+password+"','"+firstName+"','"+lastName+"','"+QDateTime::currentDateTime().toString("yyyy-MM-dd")+"');");
	mutex->unlock();
	return result;
}
bool DatabaseManager::removeUser(QString login)
{
	if(login.isEmpty()) return false;
	if(!isUserExists(login)) return false;
	mutex->lock();
	QSqlQuery q(*db);
	q.exec("delete from utilisation where login='"+login+"';");
	q.exec("delete from admin where login='"+login+"';");
	q.exec("delete from superadmin where login='"+login+"';");
	bool result=q.exec("delete from user where login='"+login+"';");
	mutex->unlock();
	return result;
}
bool DatabaseManager::setAdmin(QString login,QString depotname,bool admin)
{
	if(login.isEmpty() || depotname.isEmpty()) return false;
	if(!isUserExists(login)) return false;
	if(!isDepotExists(depotname)) return false;
	if((admin && this->isAdmin(login,depotname)) || (!admin && !this->isAdmin(login,depotname))) return true;
	mutex->lock();
	QSqlQuery q(*db);
	bool result;
	if(admin) result=q.exec("insert into admin(depotname,login,inscriptionDate) values('"+depotname+"','"+login+"','"+QDateTime::currentDateTime().toString("yyyy-MM-dd")+"');");
	else      result=q.exec("delete from admin where login='"+login+"' and depotname='"+depotname+"';");
	mutex->unlock();
	return result;
}
bool DatabaseManager::setSuperAdmin(QString login,bool superadmin)
{
	if(login.isEmpty()) return false;
	if(!isUserExists(login)) return false;
	if((superadmin && this->isSuperAdmin(login)) || (!superadmin && !this->isSuperAdmin(login))) return true;
	mutex->lock();
	QSqlQuery q(*db);
	bool result;
	if(superadmin) result=q.exec("insert into superadmin(login,inscriptionDate) values('"+login+"','"+QDateTime::currentDateTime().toString("yyyy-MM-dd")+"');");
	else           result=q.exec("delete from superadmin where login='"+login+"';");
	mutex->unlock();
	return result;
}




///////////////////////////////////////////////////////////
/////       FONCTIONS D'ADMINISTRATION DES DEPOTS
///////////////////////////////////////////////////////////

bool DatabaseManager::createDepot(QString depotname)
{
	if(depotname.isEmpty()) return false;
	if(isDepotExists(depotname)) return false;
	mutex->lock();
	QSqlQuery q(*db);
	bool result=q.exec("insert into depot(depotname,inscriptiondate) values('"+depotname+"','"+QDateTime::currentDateTime().toString("yyyy-MM-dd")+"');");
	mutex->unlock();
	return result;
}
bool DatabaseManager::removeDepot(QString depotname)
{
	if(depotname.isEmpty()) return false;
	if(!isDepotExists(depotname)) return false;
	mutex->lock();
	QSqlQuery q(*db);
	q.exec("delete from utilisation where depotname='"+depotname+"';");
	q.exec("delete from admin where depotname='"+depotname+"';");
	bool result=q.exec("delete from depot where depotname='"+depotname+"';");
	mutex->unlock();
	return result;
}
bool DatabaseManager::setUtilisation(QString login,QString depotname,bool utilise)
{
	if(login.isEmpty() || depotname.isEmpty()) return false;
	if(!isUserExists(login)) return false;
	if(!isDepotExists(depotname)) return false;
	if((utilise && this->isUserLinkDepot(login,depotname)) || (!utilise && !this->isUserLinkDepot(login,depotname))) return true;
	mutex->lock();
	QSqlQuery q(*db);
	bool result;
	if(utilise) result=q.exec("insert into utilisation(depotname,login,readonly,inscriptionDate) values('"+depotname+"','"+login+"','false','"+QDateTime::currentDateTime().toString("yyyy-MM-dd")+"');");
	else        result=q.exec("delete from utilisation where login='"+login+"' and depotname='"+depotname+"';");
	mutex->unlock();
	return result;
}
bool DatabaseManager::setUtilisationReadOnly(QString login,QString depotname,bool readOnly)
{
	if(login.isEmpty() || depotname.isEmpty()) return false;
	if(!this->isUserLinkDepot(login,depotname)) return false;
	mutex->lock();
	QSqlQuery q(*db);
	bool result=q.exec("update utilisation set readonly='"+QString(readOnly?"true":"false")+"' where login='"+login+"' and depotname='"+depotname+"';");
	mutex->unlock();
	return result;
}







///////////////////////////////////////////////////////////
/////       FONCTIONS DE TYPE "IS EXISTS"
///////////////////////////////////////////////////////////

bool DatabaseManager::isUserExists(QString login)
{
	mutex->lock();
	QSqlQuery q(*db);
	q.exec("select * from user where login='"+login+"';");
	if(q.next())
	{
		mutex->unlock();
		return true;
	}
	mutex->unlock();
	return false;
}
bool DatabaseManager::isAdmin(QString login,QString depotname)
{
	if(!isUserExists(login)) return false;
	mutex->lock();
	QSqlQuery q(*db);
	q.exec("select * from admin where login='"+login+"' and depotname='"+depotname+"';");
	if(q.next())
	{
		mutex->unlock();
		return true;
	}
	mutex->unlock();
	return false;
}
bool DatabaseManager::isSuperAdmin(QString login)
{
	if(!isUserExists(login)) return false;
	mutex->lock();
	QSqlQuery q(*db);
	q.exec("select * from superadmin where login='"+login+"';");
	if(q.next())
	{
		mutex->unlock();
		return true;
	}
	mutex->unlock();
	return false;
}
bool DatabaseManager::isDepotExists(QString depotname)
{
	mutex->lock();
	QSqlQuery q(*db);
	q.exec("select * from depot where depotname='"+depotname+"';");
	if(q.next())
	{
		mutex->unlock();
		return true;
	}
	mutex->unlock();
	return false;
}
bool DatabaseManager::isUserLinkDepot(QString login,QString depotname)
{
	mutex->lock();
	QSqlQuery q(*db);
	q.exec("select * from utilisation where login='"+login+"' and depotname='"+depotname+"';");
	if(q.next())
	{
		mutex->unlock();
		return true;
	}
	mutex->unlock();
	return false;
}








///////////////////////////////////////////////////////////
/////       FONCTIONS D'AUTHENTIFICATION
///////////////////////////////////////////////////////////

bool DatabaseManager::authentificateUser(QString login,QString password)
{
	password=DatabaseManager::hashPassword(password);
	mutex->lock();
	QSqlQuery q(*db);
	q.exec("select * from user where login='"+login+"' and password='"+password+"';");
	if(q.next())
	{
		mutex->unlock();
		return true;
	}
	mutex->unlock();
	return false;
}
bool DatabaseManager::authentificateAdminDepot(QString login,QString password,QString depotname)
{
	if(!authentificateUser(login,password)) return false;
	if(!isAdmin(login,depotname)) return false;
	return true;
}
bool DatabaseManager::authentificateSuperAdmin(QString login,QString password)
{
	if(!authentificateUser(login,password)) return false;
	if(!isSuperAdmin(login)) return false;
	return true;
}






///////////////////////////////////////////////////////////
/////       FONCTIONS DE RECUPERATION D'INFOS
///////////////////////////////////////////////////////////

SqlGeneralInfos *DatabaseManager::getGeneralInfos()
{
	mutex->lock();
	QSqlQuery q(*db);
	q.exec("select serverPort, filesPath, svnAddress, svnUser, svnPassword, svnPort from infos;");
	if(!q.next())
	{
		mutex->unlock();
		return NULL;
	}

	SqlGeneralInfos *infos=new SqlGeneralInfos;
	infos->serverPort=q.value(0).toString().toInt();
	infos->filesPath=q.value(1).toString();
	infos->svnAddress=q.value(2).toString();
	infos->svnUser=q.value(3).toString();
	infos->svnPassword=q.value(4).toString();
	infos->svnPort=q.value(5).toString().toInt();

	mutex->unlock();
	return infos;
}
SqlUser *DatabaseManager::getUser(QString login)
{
	mutex->lock();
	QSqlQuery q(*db);
	q.exec("select login, password, firstname, lastname, inscriptiondate from user where login='"+login+"';");
	if(!q.next())
	{
		mutex->unlock();
		return NULL;
	}

	SqlUser *user=new SqlUser;
	user->login=q.value(0).toString();
	user->password=q.value(1).toString();
	user->firstName=q.value(2).toString();
	user->lastName=q.value(3).toString();
	user->inscriptionDate=q.value(4).toDateTime();
	user->utilisations=new QList<SqlUtilisation*>();

	q.exec("select depotname,readonly,inscriptiondate from utilisation where login='"+login+"';");
	while(q.next())
	{
		SqlUtilisation *u=new SqlUtilisation();
		u->login=user->login;
		u->depotname=q.value(0).toString();
		u->readonly=(q.value(1).toString()=="true")?true:false;
		u->inscriptionDate=q.value(2).toDateTime();
		user->utilisations->append(u);
	}

	mutex->unlock();
	return user;
}
SqlDepot *DatabaseManager::getDepot(QString depotname)
{
	mutex->lock();
	QSqlQuery q(*db);
	q.exec("select depotname, inscriptiondate from depot where depotname='"+depotname+"';");
	if(!q.next())
	{
		mutex->unlock();
		return NULL;
	}

	SqlDepot *depot=new SqlDepot;
	depot->depotname=q.value(0).toString();
	depot->inscriptionDate=q.value(1).toDateTime();
	depot->utilisations=new QList<SqlUtilisation*>();

	q.exec("select login,readonly,inscriptiondate from utilisation where depotname='"+depotname+"';");
	while(q.next())
	{
		SqlUtilisation *u=new SqlUtilisation();
		u->login=q.value(0).toString();
		u->depotname=depot->depotname;
		u->readonly=(q.value(1).toString()=="true")?true:false;
		u->inscriptionDate=q.value(2).toDateTime();
		depot->utilisations->append(u);
	}

	mutex->unlock();
	return depot;
}
QList<QString> DatabaseManager::getUserLogins()
{
	mutex->lock();
	QSqlQuery q(*db);
	q.exec("select login from user;");
	QList<QString> users;
	while(q.next())
	{
		users.append(q.value(0).toString());
	}
	mutex->unlock();
	return users;
}
QList<SqlUser*> DatabaseManager::getUsers()
{
	mutex->lock();
	QSqlQuery q(*db);
	q.exec("select login from user;");
	QList<SqlUser*> users;
	while(q.next())
	{
		SqlUser *user=this->getUser(q.value(0).toString());
		if(user) users.append(user);
	}
	mutex->unlock();
	return users;
}
QList<QString> DatabaseManager::getDepotNames()
{
	mutex->lock();
	QSqlQuery q(*db);
	q.exec("select depotname from depot;");
	QList<QString> depots;
	while(q.next())
	{
		depots.append(q.value(0).toString());
	}
	mutex->unlock();
	return depots;
}
QList<SqlDepot*> DatabaseManager::getDepots()
{
	mutex->lock();
	QSqlQuery q(*db);
	q.exec("select depotname, inscriptiondate from depot;");
	QList<SqlDepot*> depots;
	while(q.next())
	{
		SqlDepot *depot=this->getDepot(q.value(0).toString());
		if(depot) depots.append(depot);
	}
	mutex->unlock();
	return depots;
}
QList<QString> DatabaseManager::getPseudoAdmins()
{
	mutex->lock();
	QSqlQuery q(*db);
	q.exec("select login from admin;");
	QList<QString> admins;
	while(q.next())
	{
		admins.append(q.value(0).toString());
	}
	mutex->unlock();
	return admins;
}
QList<QString> DatabaseManager::getPseudoSuperAdmins()
{
	mutex->lock();
	QSqlQuery q(*db);
	q.exec("select login from superadmin;");
	QList<QString> superadmins;
	while(q.next())
	{
		superadmins.append(q.value(0).toString());
	}
	mutex->unlock();
	return superadmins;
}





QString DatabaseManager::hashPassword(QString password)
{
	QCryptographicHash crypto(QCryptographicHash::Md5);
	crypto.addData(password.toAscii());
	return crypto.result();
}




DatabaseManager::DatabaseManager(QSqlDatabase *db)
{
	this->db=db;
	this->mutex=new QMutex(QMutex::Recursive);
}


DatabaseManager::~DatabaseManager()
{
	mutex->lock();
	mutex->unlock();
	delete db;
}


