#include "data_base_management.h"

#include <QtCore>

static QString fields = 		"title ,"

		"coverPage,"
		"numPages,"

		"number,"
		"isBis,"
		"count,"

		"volume,"
		"storyArc,"
		"arcNumber,"
		"arcCount,"

		"genere,"

		"writer,"
		"penciller,"
		"inker,"
		"colorist,"
		"letterer,"
		"coverArtist,"

		"date," 
		"publisher,"
		"format,"
		"color,"
		"ageRating,"

		"synopsis,"
		"characters,"
		"notes,"

		"hash";

DataBaseManagement::DataBaseManagement()
	:QObject(),dataBasesList()
{

}

/*TreeModel * DataBaseManagement::newTreeModel(QString path)
{
	//la consulta se ejecuta...
	QSqlQuery selectQuery(loadDatabase(path));
	selectQuery.setForwardOnly(true);
	selectQuery.exec("select * from folder order by parentId,name");
	//selectQuery.finish();
	return new TreeModel(selectQuery);
}*/

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

	{
	QSqlQuery pragma("PRAGMA foreign_keys = ON",db);
	//pragma.finish();
	DataBaseManagement::createTables(db);
	
	QSqlQuery query("INSERT INTO folder (parentId, name, path) "
                   "VALUES (1,'root', '/')",db);
	}
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

QSqlDatabase DataBaseManagement::loadDatabaseFromFile(QString filePath)
{
	//TODO check path
	QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE",filePath);
	db.setDatabaseName(filePath);
	if (!db.open()) {
		//se devuelve una base de datos vacía e inválida
		
		return QSqlDatabase();
	}
	{
	QSqlQuery pragma("PRAGMA foreign_keys = ON",db);
	}
	//pragma.finish();
	//devuelve la base de datos
	return db;
}

bool DataBaseManagement::createTables(QSqlDatabase & database)
{
	bool success = true;

	//FOLDER (representa una carpeta en disco)
	{
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

		"hash TEXT UNIQUE NOT NULL,"
		"edited BOOLEAN DEFAULT 0,"
		"read BOOLEAN DEFAULT 0)");
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
	}

	return success;
}
#include <qmessagebox.h>
void DataBaseManagement::exportComicsInfo(QString source, QString dest)
{
	//QSqlDatabase sourceDB = loadDatabase(source);
	QSqlDatabase destDB = loadDatabaseFromFile(dest);
	//sourceDB.open();
	{
	QSqlQuery attach(destDB);
	attach.prepare("ATTACH DATABASE '"+QDir().toNativeSeparators(dest) +"' AS dest;");
	//attach.bindValue(":dest",QDir().toNativeSeparators(dest));
	attach.exec();
	//attach.finish();

	QSqlQuery attach2(destDB);
	attach2.prepare("ATTACH DATABASE '"+QDir().toNativeSeparators(source) +"' AS source;");
	attach2.exec();
	//attach2.finish();

	//sourceDB.close();
	QSqlQuery queryDBInfo(destDB);
	queryDBInfo.prepare("CREATE TABLE dest.db_info (version TEXT NOT NULL)");
	queryDBInfo.exec();
	//queryDBInfo.finish();

	/*QSqlQuery queryComicsInfo(sourceDB);
	queryComicsInfo.prepare("CREATE TABLE dest.comic_info (id INTEGER PRIMARY KEY, hash TEXT NOT NULL, edited BOOLEAN DEFAULT FALSE, title TEXT, read BOOLEAN)");
	queryComicsInfo.exec();*/

	QSqlQuery query("INSERT INTO dest.db_info (version) "
		"VALUES ('5.0.0')",destDB);
	//query.finish();

	QSqlQuery exportData(destDB);
	exportData.prepare("create table dest.comic_info as select " + fields +
		" from source.comic_info where source.comic_info.edited = 1");
	exportData.exec();
	//exportData.finish();
	}

	//sourceDB.close();
	destDB.close();
	QSqlDatabase::removeDatabase(dest);

}

bool DataBaseManagement::importComicsInfo(QString source, QString dest)
{
	QString error;
	QString driver;

	bool b = false;

	QSqlDatabase sourceDB = loadDatabaseFromFile(source);
	QSqlDatabase destDB = loadDatabaseFromFile(dest);

	QSqlQuery pragma("PRAGMA synchronous=OFF",destDB);
	
	
	//QSqlQuery attach(destDB);
	//attach.prepare("ATTACH DATABASE '"+QDir().toNativeSeparators(dest) +"' AS dest;");
	//attach.exec();
	
	//error = attach.lastError().databaseText();
	//driver = attach.lastError().driverText();

	//QMessageBox::critical(NULL,tr("db error"),error);
	//QMessageBox::critical(NULL,tr("db error"),driver);

	//QSqlQuery attach2(destDB);
	//attach2.prepare("ATTACH DATABASE '"+QDir().toNativeSeparators(source) +"' AS source;");
	//attach2.exec();
	//
	//error = attach2.lastError().databaseText();
	//driver = attach2.lastError().driverText();

	//QMessageBox::critical(NULL,tr("db error"),error);
	//QMessageBox::critical(NULL,tr("db error"),driver);
	//TODO check versions...
	//QSqlQuery update(destDB);
	//update.prepare("UPDATE dest.comic_info"
 //   "SET"
	//"title = coalesce(title , (select source.comic_info.title from source.comic_info where dest.comic_info.hash = source.comic_info.hash)),"
	//"coverPage = coalesce(coverPage , (select source.comic_info.coverPage from source.comic_info where dest.comic_info.hash = source.comic_info.hash)),"
	//"numPages = coalesce(numPages , (select source.comic_info.numPages from source.comic_info where dest.comic_info.hash = source.comic_info.hash)),"
	//"number = coalesce(number , (select source.comic_info.number from source.comic_info where dest.comic_info.hash = source.comic_info.hash)),"
	//"isBis = coalesce(isBis , (select source.comic_info.isBis from source.comic_info where dest.comic_info.hash = source.comic_info.hash)),"
	//"count = coalesce(count , (select source.comic_info.count from source.comic_info where dest.comic_info.hash = source.comic_info.hash)),"
	//"volume = coalesce(volume , (select source.comic_info.volume from source.comic_info where dest.comic_info.hash = source.comic_info.hash)),"
	//"storyArc = coalesce(storyArc , (select source.comic_info.storyArc from source.comic_info where dest.comic_info.hash = source.comic_info.hash)),"
	//"arcNumber = coalesce(arcNumber , (select source.comic_info.arcNumber from source.comic_info where dest.comic_info.hash = source.comic_info.hash)),"
	//"arcCount = coalesce(arcCount , (select source.comic_info.arcCount from source.comic_info where dest.comic_info.hash = source.comic_info.hash)),"
	//"genere = coalesce(genere , (select source.comic_info.genere from source.comic_info where dest.comic_info.hash = source.comic_info.hash)),"
	//"writer = coalesce(writer , (select source.comic_info.writer from source.comic_info where dest.comic_info.hash = source.comic_info.hash)),"
	//"penciller = coalesce(penciller , (select source.comic_info.penciller from source.comic_info where dest.comic_info.hash = source.comic_info.hash)),"
	//"inker = coalesce(inker , (select source.comic_info.inker from source.comic_info where dest.comic_info.hash = source.comic_info.hash)),"
	//"colorist = coalesce(colorist , (select source.comic_info.colorist from source.comic_info where dest.comic_info.hash = source.comic_info.hash)),"
	//"letterer = coalesce(letterer , (select source.comic_info.letterer from source.comic_info where dest.comic_info.hash = source.comic_info.hash)),"
	//"coverArtist = coalesce(coverArtist , (select source.comic_info.coverArtist from source.comic_info where dest.comic_info.hash = source.comic_info.hash)),"
	//"date = coalesce(date , (select source.comic_info.date from source.comic_info where dest.comic_info.hash = source.comic_info.hash)),"
	//"publisher = coalesce(publisher , (select source.comic_info.publisher from source.comic_info where dest.comic_info.hash = source.comic_info.hash)),"
	//"format = coalesce(format , (select source.comic_info.format from source.comic_info where dest.comic_info.hash = source.comic_info.hash)),"
	//"color = coalesce(color , (select source.comic_info.color from source.comic_info where dest.comic_info.hash = source.comic_info.hash)),"
	//"ageRating = coalesce(ageRating , (select source.comic_info.ageRating from source.comic_info where dest.comic_info.hash = source.comic_info.hash)),"
	//"synopsis = coalesce(synopsis , (select source.comic_info.synopsis from source.comic_info where dest.comic_info.hash = source.comic_info.hash)),"
	//"characters = coalesce(characters , (select source.comic_info.characters from source.comic_info where dest.comic_info.hash = source.comic_info.hash)),"
	//"notes = coalesce(notes , (select source.comic_info.notes from source.comic_info where dest.comic_info.hash = source.comic_info.hash)),"
	//"edited = 1"
	//	);
	//b = b || update.exec();

	//error = update.lastError().databaseText();
	//driver = update.lastError().driverText();

	//QMessageBox::critical(NULL,tr("db error"),error);
	//QMessageBox::critical(NULL,tr("db error"),driver);

	//QSqlQuery import(destDB);
	//import.prepare("insert or ignore into dest.comic_info (" +fields + ",edited,read) select " + fields + ",1 as edited, 0 as read from source.comic_info;");
	////import.prepare("insert into dest.comic_info (" +fields + ",edited,read) select " + fields + ",1 as edited, 0 as read from source.comic_info where sourc.comic_info.hash not in (select dest.comic_info.hash from dest.comic_info);");
	//b = b || import.exec();
	//error = import.lastError().databaseText();
	//driver = import.lastError().driverText();

	//QMessageBox::critical(NULL,tr("db error"),error);
	//QMessageBox::critical(NULL,tr("db error"),driver);

	QSqlQuery update(destDB);
	update.prepare("UPDATE comic_info SET "
		"title = :title,"
		
		"coverPage = :coverPage,"
		"numPages = :numPages,"

		"number = :number,"
		"isBis = :isBis,"
		"count = :count,"

		"volume = :volume,"
		"storyArc = :storyArc,"
		"arcNumber = :arcNumber,"
		"arcCount = :arcCount,"

		"genere = :genere,"
		
		"writer = :writer,"
		"penciller = :penciller,"
		"inker = :inker,"
		"colorist = :colorist,"
		"letterer = :letterer,"
		"coverArtist = :coverArtist,"

		"date = :date,"
		"publisher = :publisher,"
		"format = :format,"
		"color = :color,"
		"ageRating = :ageRating,"

		"synopsis = :synopsis,"
		"characters = :characters,"
		"notes = :notes,"
		
		"edited = :edited"
		
		" WHERE hash = :hash ");

		QSqlQuery insert(destDB);
	insert.prepare("INSERT INTO comic_info SET "
		"title = :title,"
		
		"coverPage = :coverPage,"
		"numPages = :numPages,"

		"number = :number,"
		"isBis = :isBis,"
		"count = :count,"

		"volume = :volume,"
		"storyArc = :storyArc,"
		"arcNumber = :arcNumber,"
		"arcCount = :arcCount,"

		"genere = :genere,"
		
		"writer = :writer,"
		"penciller = :penciller,"
		"inker = :inker,"
		"colorist = :colorist,"
		"letterer = :letterer,"
		"coverArtist = :coverArtist,"

		"date = :date,"
		"publisher = :publisher,"
		"format = :format,"
		"color = :color,"
		"ageRating = :ageRating,"

		"synopsis = :synopsis,"
		"characters = :characters,"
		"notes = :notes,"
		
		"read = :read,"
		"edited = :edited,"
		
		"hash = :hash ");
	QSqlQuery newInfo(sourceDB);
	newInfo.prepare("SELECT * FROM comic_info");
	newInfo.exec();
	destDB.transaction();
	while (newInfo.next()) //cada tupla deberá ser insertada o actualizada
	{
		for(int i = 0; i<10000; i++)
		{
		QSqlRecord record = newInfo.record();
		
		update.bindValue(":title",record.value("title").toString());

		update.bindValue(":coverPage",record.value("coverPage").toInt());
		update.bindValue(":numPages",record.value("numPages").toInt());

		update.bindValue(":number",record.value("number").toInt());
		update.bindValue(":isBis",record.value("isBis").toInt());
		update.bindValue(":count",record.value("count").toInt());

		update.bindValue(":volume",record.value("volume").toString());
		update.bindValue(":storyArc",record.value("storyArc").toString());
		update.bindValue(":arcNumber",record.value("arcNumber").toString());
		update.bindValue(":arcCount",record.value("arcCount").toString());

		update.bindValue(":genere",record.value("genere").toString());

		update.bindValue(":writer",record.value("writer").toString());
		update.bindValue(":penciller",record.value("penciller").toString());
		update.bindValue(":inker",record.value("inker").toString());
		update.bindValue(":colorist",record.value("colorist").toString());
		update.bindValue(":letterer",record.value("letterer").toString());
		update.bindValue(":coverArtist",record.value("coverArtist").toString());

		update.bindValue(":date",record.value("date").toString());
		update.bindValue(":publisher",record.value("publisher").toString());
		update.bindValue(":format",record.value("format").toString());
		update.bindValue(":color",record.value("color").toInt());
		update.bindValue(":ageRating",record.value("ageRating").toString());
		
		update.bindValue(":synopsis",record.value("synopsis").toString());
		update.bindValue(":characters",record.value("characters").toString());
		update.bindValue(":notes",record.value("notes").toString());

		update.bindValue(":edited",1);

		update.bindValue(":hash",record.value("hash").toString());
		update.exec();

		if(update.numRowsAffected() == 0)
		{
			insert.bindValue(":title",record.value("title").toString());

			insert.bindValue(":coverPage",record.value("coverPage").toInt());
			insert.bindValue(":numPages",record.value("numPages").toInt());

			insert.bindValue(":number",record.value("number").toInt());
			insert.bindValue(":isBis",record.value("isBis").toInt());
			insert.bindValue(":count",record.value("count").toInt());

			insert.bindValue(":volume",record.value("volume").toString());
			insert.bindValue(":storyArc",record.value("storyArc").toString());
			insert.bindValue(":arcNumber",record.value("arcNumber").toString());
			insert.bindValue(":arcCount",record.value("arcCount").toString());

			insert.bindValue(":genere",record.value("genere").toString());

			insert.bindValue(":writer",record.value("writer").toString());
			insert.bindValue(":penciller",record.value("penciller").toString());
			insert.bindValue(":inker",record.value("inker").toString());
			insert.bindValue(":colorist",record.value("colorist").toString());
			insert.bindValue(":letterer",record.value("letterer").toString());
			insert.bindValue(":coverArtist",record.value("coverArtist").toString());

			insert.bindValue(":date",record.value("date").toString());
			insert.bindValue(":publisher",record.value("publisher").toString());
			insert.bindValue(":format",record.value("format").toString());
			insert.bindValue(":color",record.value("color").toInt());
			insert.bindValue(":ageRating",record.value("ageRating").toString());

			insert.bindValue(":synopsis",record.value("synopsis").toString());
			insert.bindValue(":characters",record.value("characters").toString());
			insert.bindValue(":notes",record.value("notes").toString());

			insert.bindValue(":edited",1);
			insert.bindValue(":read",0);

			insert.bindValue(":hash",record.value("hash").toString());
			insert.exec();
		}
		update.finish();
		insert.finish();
		}
	}
	
	destDB.commit();

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