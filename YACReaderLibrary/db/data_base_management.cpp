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
	//selectQuery.finish();
	return new TreeModel(selectQuery);
}

QSqlDatabase DataBaseManagement::createDatabase(QString name, QString path)
{
	return createDatabase(QDir::cleanPath(path) + "/" + name + ".ydb");
}

QSqlDatabase DataBaseManagement::createDatabase(QString dest)
{
	QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
	db.setDatabaseName(dest);
	if (!db.open())
		qDebug() << db.lastError();
	else {
		qDebug() << db.tables();
	}
	QSqlQuery pragma("PRAGMA foreign_keys = ON",db);
	//pragma.finish();
	DataBaseManagement::createTables(db);
	
	QSqlQuery query("INSERT INTO folder (parentId, name, path) "
                   "VALUES (1,'root', '/')",db);
	//query.finish();
	//db.close();

	return db;
}

QSqlDatabase DataBaseManagement::loadDatabase(QString path)
{
	//TODO check path
	QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE",path);
	db.setDatabaseName(path+"/library.ydb");
	if (!db.open()) {
		//se devuelve una base de datos vacía e inválida
		
		return QSqlDatabase();
	}
	QSqlQuery pragma("PRAGMA foreign_keys = ON",db);
	//pragma.finish();
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
	//queryFolder.finish();
		//COMIC INFO (representa la información de un cómic, cada cómic tendrá un idéntificador único formado por un hash sha1'de los primeros 512kb' + su tamaño en bytes)
	QSqlQuery queryComicInfo(database);
	queryComicInfo.prepare("CREATE TABLE comic_info ("
		"id INTEGER PRIMARY KEY,"
		"title TEXT,"

		"coverPage INTEGER,"
		"numPages INTEGER,"

		"number INTEGER,"
		"isBis BOOLEAN,"
		"count INTEGER,"

		"volume TEXT,"
		"storyArc TEXT,"
		"arcNumber INTEGER,"
		"arcCount INTEGER,"

		"genere TEXT,"

		"writer TEXT,"
		"penciller TEXT,"
		"inker TEXT,"
		"colorist TEXT,"
		"letterer TEXT,"
		"coverArtist TEXT,"

		"date TEXT," //dd/mm/yyyy --> se mostrará en 3 campos diferentes
		"publisher TEXT,"
		"format TEXT,"
		"color BOOLEAN,"
		"ageRating BOOLEAN,"

		"synopsis TEXT,"
		"characters TEXT,"
		"notes TEXT,"

		"hash TEXT NOT NULL,"
		"edited BOOLEAN DEFAULT 0,"
		"read BOOLEAN)");
	success = success && queryComicInfo.exec();
	//queryComicInfo.finish();

	//COMIC (representa un cómic en disco, contiene el nombre de fichero)
	QSqlQuery queryComic(database);
	queryComic.prepare("CREATE TABLE comic (id INTEGER PRIMARY KEY, parentId INTEGER NOT NULL, comicInfoId INTEGER NOT NULL,  fileName TEXT NOT NULL, path TEXT, FOREIGN KEY(parentId) REFERENCES folder(id) ON DELETE CASCADE, FOREIGN KEY(comicInfoId) REFERENCES comic_info(id))");
	success = success && queryComic.exec();
	//queryComic.finish();
	//DB INFO
	QSqlQuery queryDBInfo(database);
	queryDBInfo.prepare("CREATE TABLE db_info (version TEXT NOT NULL)");
	success = success && queryDBInfo.exec();
	//queryDBInfo.finish();

	QSqlQuery query("INSERT INTO db_info (version) "
                   "VALUES ('5.0.0')",database);
	//query.finish();


	return success;
}

void DataBaseManagement::exportComicsInfo(QString source, QString dest)
{
	QSqlDatabase sourceDB = loadDatabase(source);
	QSqlDatabase destDB = QSqlDatabase::addDatabase("QSQLITE");
	destDB.setDatabaseName(dest);
	destDB.open();
	sourceDB.open();

	QSqlQuery attach(sourceDB);
	attach.prepare("ATTACH DATABASE '"+QDir().toNativeSeparators(dest) +"' AS dest;");
	//attach.bindValue(":dest",QDir().toNativeSeparators(dest));
	attach.exec();
	//attach.finish();

	QSqlQuery attach2(sourceDB);
	attach2.prepare("ATTACH DATABASE '"+QDir().toNativeSeparators(source) +"' AS source;");
	attach2.exec();
	//attach2.finish();

	//sourceDB.close();
	QSqlQuery queryDBInfo(sourceDB);
	queryDBInfo.prepare("CREATE TABLE dest.db_info (version TEXT NOT NULL)");
	queryDBInfo.exec();
	//queryDBInfo.finish();

	/*QSqlQuery queryComicsInfo(sourceDB);
	queryComicsInfo.prepare("CREATE TABLE dest.comic_info (id INTEGER PRIMARY KEY, hash TEXT NOT NULL, edited BOOLEAN DEFAULT FALSE, title TEXT, read BOOLEAN)");
	queryComicsInfo.exec();*/

	QSqlQuery query("INSERT INTO dest.db_info (version) "
		"VALUES ('5.0.0')",sourceDB);
	//query.finish();

	QSqlQuery exportData(sourceDB);
	exportData.prepare("create table dest.comic_info as select * from source.comic_info where source.comic_info.edited = 1");
	exportData.exec();
	//exportData.finish();

	QString error = exportData.lastError().databaseText();
	QString error2 = exportData.lastError().text();

	sourceDB.close();
	destDB.close();

}
#include <qmessagebox.h>
bool DataBaseManagement::importComicsInfo(QString source, QString dest)
{
	QString error;
	QString driver;

	bool b = false;

	QSqlDatabase destDB = QSqlDatabase::addDatabase("QSQLITE",dest);
	destDB.setDatabaseName(dest);
	if(destDB.open())
	{
	QSqlQuery pragma("PRAGMA foreign_keys = ON",destDB);

	QSqlQuery attach(destDB);
	attach.prepare("ATTACH DATABASE '"+QDir().toNativeSeparators(dest) +"' AS dest;");
	attach.exec();
	
	error = attach.lastError().databaseText();
	driver = attach.lastError().driverText();

	QMessageBox::critical(NULL,tr("db error"),error);
	QMessageBox::critical(NULL,tr("db error"),driver);

	QSqlQuery attach2(destDB);
	attach2.prepare("ATTACH DATABASE '"+QDir().toNativeSeparators(source) +"' AS source;");
	attach2.exec();
	
	error = attach2.lastError().databaseText();
	driver = attach2.lastError().driverText();

	QMessageBox::critical(NULL,tr("db error"),error);
	QMessageBox::critical(NULL,tr("db error"),driver);
	//TODO check versions...

	QSqlQuery import(destDB);
	import.prepare("insert or replace into dest.comic_info select * from source.comic_info;");
	bool b = import.exec();
	error = import.lastError().databaseText();
	driver = import.lastError().driverText();

	QMessageBox::critical(NULL,tr("db error"),error);
	QMessageBox::critical(NULL,tr("db error"),driver);

	destDB.close();
	}
	return b;

}

//COMICS_INFO_EXPORTER
ComicsInfoExporter::ComicsInfoExporter()
:QThread()
{
}

void ComicsInfoExporter::exportComicsInfo(QSqlDatabase & source, QSqlDatabase & dest)
{

}

void ComicsInfoExporter::run()
{

}


//COMICS_INFO_IMPORTER
ComicsInfoImporter::ComicsInfoImporter()
:QThread()
{
}

void ComicsInfoImporter::importComicsInfo(QSqlDatabase & source, QSqlDatabase & dest)
{

}

void ComicsInfoImporter::run()
{

}