#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H


#include<QtCore>
#include<QtSql>



typedef struct SqlUtilisation
{
	QString depotname;
	QString access;
	QDateTime inscriptionDate;
} SqlUtilisation;


typedef struct SqlUser
{
	QString login;
	QString password;
	QString firstName;
	QString lastName;
	QDateTime inscriptionDate;
	QList<SqlUtilisation*> *utilisations;
} SqlUser;


typedef struct SqlDepot
{
	QString depotname;
	QDateTime inscriptionDate;
} SqlDepot;




class DatabaseManager
{
public:
	static DatabaseManager *createDatabaseManager(QString name,QString login,QString password);
	static DatabaseManager *loadDatabaseManager(QString name,QString login,QString password);

	bool isUserExists(QString login);
	bool isDepotExists(QString depotname);
	bool authentificateUser(QString login,QString password);
	bool userIsLinkDepot(QString login,QString depotname);
	bool authentificateAdminDepot(QString login,QString password,QString depotname);
	bool authentificateSuperAdmin(QString login,QString password);

	SqlUser *getUser(QString login);
	SqlDepot *getDepot(QString depotname);

	QList<SqlUser*> getUsers();
	QList<SqlDepot*> getDepots();
	QList<QString> getPseudoAdmins();
	QList<QString> getPseudoSuperAdmins();

private:
	DatabaseManager(QSqlDatabase *db);
	QSqlDatabase *db;
	QMutex mutex;
};



#endif // DATABASEMANAGER_H
