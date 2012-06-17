#include "data_base_management.h"

#include <QtCore>

DataBaseManagement::DataBaseManagement()
	:QObject(),dataBasesList()
{

}

TreeModel * DataBaseManagement::newTreeModel(QString path)
{
	//la consulta se ejecuta...
	QSqlQuery selectQuery(loadDatabase(path));
	selectQuery.setForwardOnly(true);
	selectQuery.exec("select * from folder order by parentId,name");
	return new TreeModel(selectQuery);
}

QSqlDatabase DataBaseManagement::createDatabase(QString name, QString path)
{
	QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
	db.setDatabaseName(QDir::cleanPath(path) + "/" + name + ".ydb");
	if (!db.open())
		qDebug() << db.lastError();
	else {
		qDebug() << db.tables();
	}
	QSqlQuery pragma("PRAGMA foreign_keys = ON",db);
	DataBaseManagement::createTables(db);
	
	QSqlQuery query("INSERT INTO folder (parentId, name, path) "
                   "VALUES (1,'root', '/')",db);

	//db.close();

	return db;
}

QSqlDatabase DataBaseManagement::loadDatabase(QString path)
{
	//TODO check path
	QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
	db.setDatabaseName(path+"/library.ydb");
	if (!db.open()) {
		//se devuelve una base de datos vacía e inválida
		
		return QSqlDatabase();
	}
	QSqlQuery pragma("PRAGMA foreign_keys = ON",db);
	//devuelve la base de datos
	return db;
}

bool DataBaseManagement::createTables(QSqlDatabase & database)
{
	bool success = true;

	//FOLDER (representa una carpeta en disco)
	QSqlQuery queryFolder(database);
	queryFolder.prepare("CREATE TABLE folder (id INTEGER PRIMARY KEY, parentId INTEGER NOT NULL, name TEXT NOT NULL, path TEXT NOT NULL, FOREIGN KEY(parentId) REFERENCES folder(id) ON DELETE CASCADE)");
	success = success && queryFolder.exec();

		//COMIC INFO (representa la información de un cómic, cada cómic tendrá un idéntificador único formado por un hash sha1'de los primeros 512kb' + su tamaño en bytes)
	QSqlQuery queryComicInfo(database);
	queryComicInfo.prepare("CREATE TABLE comic_info (id INTEGER PRIMARY KEY, hash TEXT NOT NULL, name TEXT, read BOOLEAN)");
	success = success && queryComicInfo.exec();

	//COMIC (representa un cómic en disco, contiene el nombre de fichero)
	QSqlQuery queryComic(database);
	queryComic.prepare("CREATE TABLE comic (id INTEGER PRIMARY KEY, parentId INTEGER NOT NULL, comicInfoId INTEGER NOT NULL,  fileName TEXT NOT NULL, path TEXT, FOREIGN KEY(parentId) REFERENCES folder(id) ON DELETE CASCADE, FOREIGN KEY(comicInfoId) REFERENCES comic_info(id))");
	success = success && queryComic.exec();

	//DB INFO
	QSqlQuery queryDBInfo(database);
	queryDBInfo.prepare("CREATE TABLE db_info (version TEXT NOT NULL)");
	success = success && queryDBInfo.exec();

	QSqlQuery query("INSERT INTO db_info (version) "
                   "VALUES ('5.0.0')",database);


	return success;
}