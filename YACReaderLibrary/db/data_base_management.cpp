#include "data_base_management.h"

#include <QtCore>
#include "library_creator.h"
#include "check_new_version.h"


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

        "comicVineID,"

		"hash"
		;

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
	QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE",dest);
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

    {
        //COMIC INFO (representa la información de un cómic, cada cómic tendrá un idéntificador único formado por un hash sha1'de los primeros 512kb' + su tamaño en bytes)
        QSqlQuery queryComicInfo(database);
        queryComicInfo.prepare("CREATE TABLE comic_info ("
                               "id INTEGER PRIMARY KEY,"
                               "title TEXT,"

                               "coverPage INTEGER DEFAULT 1,"
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
                               "read BOOLEAN DEFAULT 0,"
                               //new 7.0 fields

                               "hasBeenOpened BOOLEAN DEFAULT 0,"
                               "rating INTEGER DEFAULT 0,"
                               "currentPage INTEGER DEFAULT 1, "
                               "bookmark1 INTEGER DEFAULT -1, "
                               "bookmark2 INTEGER DEFAULT -1, "
                               "bookmark3 INTEGER DEFAULT -1, "
                               "brightness INTEGER DEFAULT -1, "
                               "contrast INTEGER DEFAULT -1, "
                               "gamma INTEGER DEFAULT -1, "
                               //new 7.1 fields
                               "comicVineID TEXT"

                               ")");
        success = success && queryComicInfo.exec();
        //queryComicInfo.finish();

        //FOLDER (representa una carpeta en disco)
        QSqlQuery queryFolder(database);
        queryFolder.prepare("CREATE TABLE folder ("
                            "id INTEGER PRIMARY KEY,"
                            "parentId INTEGER NOT NULL,"
                            "name TEXT NOT NULL,"
                            "path TEXT NOT NULL,"
                            //new 7.1 fields
                            "finished BOOLEAN DEFAULT 0,"   //reading
                            "completed BOOLEAN DEFAULT 1,"  //collecting
                            //new 8.6 fields
                            "numChildren INTEGER,"
                            "firstChildId INTEGER,"
                            "customImage TEXT,"
                            "FOREIGN KEY(parentId) REFERENCES folder(id) ON DELETE CASCADE, "
                            //8.6
                            "FOREIGN KEY(firstChildId) REFERENCES comic_info(id))");
        success = success && queryFolder.exec();

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
                        "VALUES ('" VERSION "')",database);
        //query.finish();

        //8.0> tables
        success = success && DataBaseManagement::createV8Tables(database);
    }

    return success;
}

bool DataBaseManagement::createV8Tables(QSqlDatabase &database)
{
    bool success = true;
    {
        //8.0> tables
        //LABEL
        QSqlQuery queryLabel(database);
        success = success && queryLabel.exec("CREATE TABLE label (id INTEGER PRIMARY KEY, "
                                             "name TEXT NOT NULL, "
                                             "color TEXT NOT NULL, "
                                             "ordering INTEGER NOT NULL); "); //order depends on the color

        QSqlQuery queryIndexLabel(database);
        success = success && queryIndexLabel.exec("CREATE INDEX label_ordering_index ON label (ordering)");

        //COMIC LABEL
        QSqlQuery queryComicLabel(database);
        success = success && queryComicLabel.exec("CREATE TABLE comic_label ("
                                                  "comic_id INTEGER, "
                                                  "label_id INTEGER, "
                                                  "ordering INTEGER, " //TODO order????
                                                  "FOREIGN KEY(label_id) REFERENCES label(id) ON DELETE CASCADE, "
                                                  "FOREIGN KEY(comic_id) REFERENCES comic(id) ON DELETE CASCADE, "
                                                  "PRIMARY KEY(label_id, comic_id))");

        QSqlQuery queryIndexComicLabel(database);
        success = success && queryIndexComicLabel.exec("CREATE INDEX comic_label_ordering_index ON label (ordering)");

        //READING LIST
        QSqlQuery queryReadingList(database);
        success = success && queryReadingList.exec("CREATE TABLE reading_list ("
                                                   "id INTEGER PRIMARY KEY, "
                                                   "parentId INTEGER, "
                                                   "ordering INTEGER DEFAULT 0, " //only use it if the parentId is NULL
                                                   "name TEXT NOT NULL, "
                                                   "finished BOOLEAN DEFAULT 0, "
                                                   "completed BOOLEAN DEFAULT 1, "
                                                   "FOREIGN KEY(parentId) REFERENCES reading_list(id) ON DELETE CASCADE)");

        QSqlQuery queryIndexReadingList(database);
        success = success && queryIndexReadingList.exec("CREATE INDEX reading_list_ordering_index ON label (ordering)");

        //COMIC READING LIST
        QSqlQuery queryComicReadingList(database);
        success = success && queryComicReadingList.exec("CREATE TABLE comic_reading_list ("
                                                        "reading_list_id INTEGER, "
                                                        "comic_id INTEGER, "
                                                        "ordering INTEGER, "
                                                        "FOREIGN KEY(reading_list_id) REFERENCES reading_list(id) ON DELETE CASCADE, "
                                                        "FOREIGN KEY(comic_id) REFERENCES comic(id) ON DELETE CASCADE, "
                                                        "PRIMARY KEY(reading_list_id, comic_id))");

        QSqlQuery queryIndexComicReadingList(database);
        success = success && queryIndexComicReadingList.exec("CREATE INDEX comic_reading_list_ordering_index ON label (ordering)");

        //DEFAULT READING LISTS
        QSqlQuery queryDefaultReadingList(database);
        success = success && queryDefaultReadingList.exec("CREATE TABLE default_reading_list ("
                                                          "id INTEGER PRIMARY KEY, "
                                                          "name TEXT NOT NULL"
                                                          //TODO icon????
                                                          ")");

        //COMIC DEFAULT READING LISTS
        QSqlQuery queryComicDefaultReadingList(database);
        success = success && queryComicDefaultReadingList.exec("CREATE TABLE comic_default_reading_list ("
                                                               "comic_id INTEGER, "
                                                               "default_reading_list_id INTEGER, "
                                                               "ordering INTEGER, " //order????
                                                               "FOREIGN KEY(default_reading_list_id) REFERENCES default_reading_list(id) ON DELETE CASCADE, "
                                                               "FOREIGN KEY(comic_id) REFERENCES comic(id) ON DELETE CASCADE,"
                                                               "PRIMARY KEY(default_reading_list_id, comic_id))");

        QSqlQuery queryIndexComicDefaultReadingList(database);
        success = success && queryIndexComicDefaultReadingList.exec("CREATE INDEX comic_default_reading_list_ordering_index ON label (ordering)");

        //INSERT DEFAULT READING LISTS
        QSqlQuery queryInsertDefaultReadingList(database);
        //if(!queryInsertDefaultReadingList.prepare())

        //1 Favorites
        //queryInsertDefaultReadingList.bindValue(":name", "Favorites");
        success = success && queryInsertDefaultReadingList.exec("INSERT INTO default_reading_list (name) VALUES (\"Favorites\")");

        //Reading doesn't need its onw list

    }
    return success;
}

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
        "VALUES ('" VERSION "')",destDB);
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
	QStringList hashes;

	bool b = false;

	QSqlDatabase sourceDB = loadDatabaseFromFile(source);
	QSqlDatabase destDB = loadDatabaseFromFile(dest);

	{
	QSqlQuery pragma("PRAGMA synchronous=OFF",destDB);
	

	QSqlQuery newInfo(sourceDB);
	newInfo.prepare("SELECT * FROM comic_info");
	newInfo.exec();
	destDB.transaction();
	int cp;
	while (newInfo.next()) //cada tupla deberá ser insertada o actualizada
	{
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

            "edited = :edited,"

            "comicVineID = :comicVineID"

			" WHERE hash = :hash ");

		QSqlQuery insert(destDB);
		insert.prepare("INSERT INTO comic_info "
			"(title,"
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
			"read,"
			"edited,"
            "comicVineID,"
			"hash)"

			"VALUES (:title,"
			":coverPage,"
			":numPages,"
			":number,"
			":isBis,"
			":count,"

			":volume,"
			":storyArc,"
			":arcNumber,"
			":arcCount,"

			":genere,"

			":writer,"
			":penciller,"
			":inker,"
			":colorist,"
			":letterer,"
			":coverArtist,"

			":date,"
			":publisher,"
			":format,"
			":color,"
			":ageRating,"

			":synopsis,"
			":characters,"
			":notes,"

			":read,"
			":edited,"
            ":comicVineID,"

			":hash )");

		QSqlRecord record = newInfo.record();
		cp = record.value("coverPage").toInt();
		if(cp>1)
		{
			QSqlQuery checkCoverPage(destDB);
			checkCoverPage.prepare("SELECT coverPage FROM comic_info where hash = :hash");
			checkCoverPage.bindValue(":hash",record.value("hash").toString());
			checkCoverPage.exec();
			bool extract = false;
			if(checkCoverPage.next())
			{
				extract = checkCoverPage.record().value("coverPage").toInt() != cp;
			}
			if(extract)
				hashes.append(record.value("hash").toString());
		}

		bindValuesFromRecord(record,update);

		update.bindValue(":edited",1);

		
		update.exec();

		if(update.numRowsAffected() == 0)
		{

			bindValuesFromRecord(record,insert);
			insert.bindValue(":edited",1);
			insert.bindValue(":read",0);

			insert.exec();

			QString error1 = insert.lastError().databaseText();
			QString error2 = insert.lastError().driverText();

			//QMessageBox::critical(NULL,"db",error1);
			//QMessageBox::critical(NULL,"driver",error2);
		}
		//update.finish();
		//insert.finish();
		}
	}

	destDB.commit();
	QString hash;
	foreach(hash, hashes)
	{
		QSqlQuery getComic(destDB);
		getComic.prepare("SELECT c.path,ci.coverPage FROM comic c INNER JOIN comic_info ci ON (c.comicInfoId = ci.id) where ci.hash = :hash");
		getComic.bindValue(":hash",hash);
		getComic.exec();
		if(getComic.next())
		{
			QString basePath = QString(dest).remove("/.yacreaderlibrary/library.ydb");
			QString path = basePath + getComic.record().value("path").toString();
			int coverPage =  getComic.record().value("coverPage").toInt();
			ThumbnailCreator tc(path,basePath+"/.yacreaderlibrary/covers/"+hash+".jpg",coverPage);
			tc.create();

		}
	}

	destDB.close();
	sourceDB.close();
	QSqlDatabase::removeDatabase(source);
	QSqlDatabase::removeDatabase(dest);
	return b;

}
//TODO fix these bindings
void DataBaseManagement::bindValuesFromRecord(const QSqlRecord & record, QSqlQuery & query)
{
	bindString("title",record,query);

	bindInt("coverPage",record,query);
	bindInt("numPages",record,query);

	bindInt("number",record,query);
	bindInt("isBis",record,query);
	bindInt("count",record,query);

	bindString("volume",record,query);
	bindString("storyArc",record,query);
	bindInt("arcNumber",record,query);
	bindInt("arcCount",record,query);

	bindString("genere",record,query);

	bindString("writer",record,query);
	bindString("penciller",record,query);
	bindString("inker",record,query);
	bindString("colorist",record,query);
	bindString("letterer",record,query);
	bindString("coverArtist",record,query);

	bindString("date",record,query);
	bindString("publisher",record,query);
	bindString("format",record,query);
	bindInt("color",record,query);
	bindString("ageRating",record,query);

	bindString("synopsis",record,query);
	bindString("characters",record,query);
	bindString("notes",record,query);

    bindString("comicVineID",record,query);

    bindString("hash",record,query);
}

bool DataBaseManagement::addColumns(const QString &tableName, const QStringList &columnDefs, const QSqlDatabase &db)
{
    QString sql = "ALTER TABLE %1 ADD COLUMN %2";
    bool returnValue = true;

    foreach(QString columnDef, columnDefs)
    {
        QSqlQuery alterTable(db);
        alterTable.prepare(sql.arg(tableName).arg(columnDef));
        //alterTableComicInfo.bindValue(":column_def",columnDef);
        alterTable.exec();
        returnValue = returnValue && (alterTable.numRowsAffected() > 0);
    }

    return returnValue;
}

bool DataBaseManagement::addConstraint(const QString &tableName, const QString &constraint, const QSqlDatabase &db)
{
    QString sql = "ALTER TABLE %1 ADD %2";
    bool returnValue = true;

    QSqlQuery alterTable(db);
    alterTable.prepare(sql.arg(tableName).arg(constraint));
    alterTable.exec();
    returnValue = returnValue && (alterTable.numRowsAffected() > 0);

    return returnValue;
}

void DataBaseManagement::bindString(const QString & name, const QSqlRecord & record, QSqlQuery & query)
{
	if(!record.value(name).isNull())
	{
		query.bindValue(":"+name,record.value(name).toString());
	}
}
void DataBaseManagement::bindInt(const QString & name, const QSqlRecord & record, QSqlQuery & query)
{
	if(!record.value(name).isNull())
	{
		query.bindValue(":"+name,record.value(name).toInt());
	}
}

QString DataBaseManagement::checkValidDB(const QString & fullPath)
{
	QSqlDatabase db = loadDatabaseFromFile(fullPath);
	QString versionString = "";
	if(db.isValid() && db.isOpen())
	{
		QSqlQuery version(db);
		version.prepare("SELECT * FROM db_info");
		version.exec();

		if(version.next())
			versionString = version.record().value("version").toString();
	}

	db.close();
	QSqlDatabase::removeDatabase(fullPath);
	return versionString;
}

int DataBaseManagement::compareVersions(const QString & v1, const QString v2)
{
	QStringList v1l = v1.split('.');
	QStringList v2l = v2.split('.');
	QList<int> v1il;
	QList<int> v2il;

	foreach(QString s, v1l)
		v1il.append(s.toInt());
	
	foreach(QString s,v2l)
		v2il.append(s.toInt());

	for(int i=0;i<qMin(v1il.length(),v2il.length());i++)
	{
		if(v1il[i]<v2il[i])
			return -1;
		if(v1il[i]>v2il[i])
			return 1;
	}

	if(v1il.length() < v2il.length())
		return -1;
	if(v1il.length() == v2il.length())
		return 0;
	if(v1il.length() > v2il.length())
		return 1;

	return 0;
}

bool DataBaseManagement::updateToCurrentVersion(const QString & fullPath)
{
    bool pre7 = false;
    bool pre7_1 = false;
    bool pre8 = false;
    bool pre8_6 = false;

    if(compareVersions(DataBaseManagement::checkValidDB(fullPath),"7.0.0")<0)
        pre7 = true;
    if(compareVersions(DataBaseManagement::checkValidDB(fullPath),"7.0.3")<0)
        pre7_1 = true;
    if(compareVersions(DataBaseManagement::checkValidDB(fullPath),"8.0.0")<0)
        pre8 = true;
    if(compareVersions(DataBaseManagement::checkValidDB(fullPath),"8.6.0")<0)
        pre8_6 = true;

	QSqlDatabase db = loadDatabaseFromFile(fullPath);
	bool returnValue = false;
    if(db.isValid() && db.isOpen())
	{
		QSqlQuery updateVersion(db);
		updateVersion.prepare("UPDATE db_info SET "
			"version = :version");
		updateVersion.bindValue(":version",VERSION);
		updateVersion.exec();

		if(updateVersion.numRowsAffected() > 0)
            returnValue = true;

        if(pre7) //TODO: execute only if previous version was < 7.0
		{
			//new 7.0 fields
			QStringList columnDefs;
			columnDefs << "hasBeenOpened BOOLEAN DEFAULT 0"
					   << "rating INTEGER DEFAULT 0"
					   << "currentPage INTEGER DEFAULT 1"
					   << "bookmark1 INTEGER DEFAULT -1"
					   << "bookmark2 INTEGER DEFAULT -1"
					   << "bookmark3 INTEGER DEFAULT -1"
					   << "brightness INTEGER DEFAULT -1"
					   << "contrast INTEGER DEFAULT -1"
					   << "gamma INTEGER DEFAULT -1";

            returnValue = returnValue && addColumns("comic_info", columnDefs, db);
		}
		//TODO update hasBeenOpened value

        if(pre7_1)
        {
            {
                QStringList columnDefs;
                columnDefs << "finished BOOLEAN DEFAULT 0"
                           << "completed BOOLEAN DEFAULT 1";
                returnValue = returnValue && addColumns("folder", columnDefs, db);
            }

            {//comic_info
                QStringList columnDefs;
                columnDefs << "comicVineID TEXT DEFAULT NULL";
                returnValue = returnValue && addColumns("comic_info", columnDefs, db);
            }
        }

        if(pre8)
        {
            returnValue = returnValue && createV8Tables(db);
        }

        if(pre8_6)
        {
            QStringList columnDefs;
            //TODO
            columnDefs << "numChildren INTEGER";
            columnDefs << "firstChildId INTEGER";
            columnDefs << "customImage TEXT";
            //returnValue = returnValue && addColumns("folder", columnDefs, db);
            //returnValue = returnValue && addConstraint("folder", FOREIGN KEY(firstChildId) REFERENCES comic_info(id), db);
        }
	}

	db.close();
	QSqlDatabase::removeDatabase(fullPath);
	return returnValue;
}

//COMICS_INFO_EXPORTER
ComicsInfoExporter::ComicsInfoExporter()
:QThread()
{
}

void ComicsInfoExporter::exportComicsInfo(QSqlDatabase & source, QSqlDatabase & dest)
{
	Q_UNUSED(source)
	Q_UNUSED(dest)
	//TODO check this method
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
	Q_UNUSED(source)
	Q_UNUSED(dest)
	//TODO check this method
}

void ComicsInfoImporter::run()
{

}
