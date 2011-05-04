#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H


#include<QtCore>
#include<QtSql>



typedef struct SqlGeneralInfos
{
	int serverPort;
	QString filesPath;
	QString svnAddress;
	QString svnUser;
	QString svnPassword;
	int svnPort;
} SqlGeneralInfos;


typedef struct SqlUtilisation
{
	QString login;
	QString depotname;
	bool readonly;
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
	QList<SqlUtilisation*> *utilisations;
	QList<QString> *pseudoAdmins;
} SqlDepot;




class DatabaseManager
{
public:
	static DatabaseManager *createDatabaseManager(QString name,QString login,QString password);
	static DatabaseManager *loadDatabaseManager(QString name,QString login,QString password);

	bool setGeneralInfos(int serverPort,QString filesPath,QString svnAddress,QString svnUser,QString svnPassword,int svnPort);
	bool createUser(QString login,QString password,QString firstName,QString lastName);
	bool removeUser(QString login);
	bool setAdmin(QString login,QString depotname,bool admin);
	bool setSuperAdmin(QString login,bool superadmin);

	bool createDepot(QString depotname);
	bool removeDepot(QString depotname);
	bool setUtilisation(QString login,QString depotname,bool utilise);
	bool setUtilisationReadOnly(QString login,QString depotname,bool readOnly);

	bool isUserExists(QString login);
	bool isAdmin(QString login,QString depotname);
	bool isSuperAdmin(QString login);
	bool isDepotExists(QString depotname);
	bool isUserLinkDepot(QString login,QString depotname);

	bool authentificateUser(QString login,QString password);
	bool authentificateAdminDepot(QString login,QString password,QString depotname);
	bool authentificateSuperAdmin(QString login,QString password);

	SqlGeneralInfos *getGeneralInfos();
	SqlUser *getUser(QString login);
	SqlDepot *getDepot(QString depotname);
	QList<QString> getUserLogins();
	QList<SqlUser*> getUsers();

	QList<QString> getDepotNames();
	QList<SqlDepot*> getDepots();

	QList<QString> getPseudoAdmins();
	QList<QString> getPseudoSuperAdmins();

	~DatabaseManager();

private:
	static QString hashPassword(QString password);

	DatabaseManager(QSqlDatabase *db);
	QSqlDatabase *db;
	QMutex *mutex;

	void lock();
	void unlock();
};



#endif // DATABASEMANAGER_H
