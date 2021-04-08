#include "data_base_management.h"

#include "library_creator.h"
#include "check_new_version.h"
#include "db_helper.h"

#include "QsLog.h"

#include <QDir>
#include <QImageReader>
#include <QList>

static QString fields = "title ,"

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

                        "hash";

QSqlDatabase DataBaseManagement::createDatabase(const QString &name, const QString &path)
{
    return createDatabase(QDir::cleanPath(path) + "/" + name + ".ydb");
}

QSqlDatabase DataBaseManagement::createDatabase(const QString &dest)
{
    QString threadId = QString::number((long long)QThread::currentThreadId(), 16);
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", dest + threadId);
    db.setDatabaseName(dest);
    if (!db.open())
        qDebug() << db.lastError();
    else {
        qDebug() << db.tables();
    }

    {
        QSqlQuery pragma("PRAGMA foreign_keys = ON", db);
        //pragma.finish();
        DataBaseManagement::createTables(db);

        QSqlQuery query("INSERT INTO folder (parentId, name, path) "
                        "VALUES (1,'root', '/')",
                        db);
    }
    //query.finish();
    //db.close();

    return db;
}

QSqlDatabase DataBaseManagement::loadDatabase(const QString &path)
{
    //TODO check path
    QString threadId = QString::number((long long)QThread::currentThreadId(), 16);
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", path + threadId);
    db.setDatabaseName(path + "/library.ydb");
    if (!db.open()) {
        return QSqlDatabase();
    }
    QSqlQuery pragma("PRAGMA foreign_keys = ON", db);

    return db;
}

QSqlDatabase DataBaseManagement::loadDatabaseFromFile(const QString &filePath)
{
    //TODO check path
    QString threadId = QString::number((long long)QThread::currentThreadId(), 16);
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", filePath + threadId);
    db.setDatabaseName(filePath);
    if (!db.open()) {
        //se devuelve una base de datos vacía e inválida

        return QSqlDatabase();
    }
    QSqlQuery pragma("PRAGMA foreign_keys = ON", db);

    return db;
}

bool DataBaseManagement::createTables(QSqlDatabase &database)
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
                               "comicVineID TEXT,"
                               //new 9.5 fields
                               "lastTimeOpened INTEGER,"
                               "coverSizeRatio REAL,"
                               "originalCoverSize STRING," //h/w
                               //new 9.8 fields
                               "manga BOOLEAN DEFAULT 0"

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
                            "finished BOOLEAN DEFAULT 0," //reading
                            "completed BOOLEAN DEFAULT 1," //collecting
                            //new 9.5 fields
                            "numChildren INTEGER,"
                            "firstChildHash TEXT,"
                            "customImage TEXT,"
                            "FOREIGN KEY(parentId) REFERENCES folder(id) ON DELETE CASCADE)");
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
                        "VALUES ('" VERSION "')",
                        database);
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
                                                   "manga BOOLEAN DEFAULT 0, "
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

void DataBaseManagement::exportComicsInfo(const QString &source, const QString &dest)
{
    QString connectionName = "";
    {
        QSqlDatabase destDB = loadDatabaseFromFile(dest);

        QSqlQuery attach(destDB);
        attach.prepare("ATTACH DATABASE '" + QDir().toNativeSeparators(dest) + "' AS dest;");
        attach.exec();

        QSqlQuery attach2(destDB);
        attach2.prepare("ATTACH DATABASE '" + QDir().toNativeSeparators(source) + "' AS source;");
        attach2.exec();

        QSqlQuery queryDBInfo(destDB);
        queryDBInfo.prepare("CREATE TABLE dest.db_info (version TEXT NOT NULL)");
        queryDBInfo.exec();

        QSqlQuery query("INSERT INTO dest.db_info (version) "
                        "VALUES ('" VERSION "')",
                        destDB);

        QSqlQuery exportData(destDB);
        exportData.prepare("create table dest.comic_info as select " + fields +
                           " from source.comic_info where source.comic_info.edited = 1");
        exportData.exec();
        connectionName = destDB.connectionName();
    }

    QSqlDatabase::removeDatabase(connectionName);
}

void DataBaseManagement::importComicsInfo(const QString &source, const QString &dest)
{
    QStringList hashes;

    QString sourceDBconnection = "";
    QString destDBconnection = "";

    {
        QSqlDatabase sourceDB = loadDatabaseFromFile(source);
        QSqlDatabase destDB = loadDatabaseFromFile(dest);

        QSqlQuery pragma("PRAGMA synchronous=OFF", destDB);

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
                           "manga = :manga"

                           "synopsis = :synopsis,"
                           "characters = :characters,"
                           "notes = :notes,"

                           "edited = :edited,"

                           "comicVineID = :comicVineID,"

                           "lastTimeOpened = :lastTimeOpened,"

                           "coverSizeRatio = :coverSizeRatio,"
                           "originalCoverSize = :originalCoverSize"

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
                           "lastTimeOpened,"
                           "coverSizeRatio,"
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
                           ":manga,"

                           ":synopsis,"
                           ":characters,"
                           ":notes,"

                           ":read,"
                           ":edited,"
                           ":comicVineID,"

                           ":lastTimeOpened,"

                           ":coverSizeRatio,"
                           ":originalCoverSize,"

                           ":hash )");

            QSqlRecord record = newInfo.record();
            cp = record.value("coverPage").toInt();
            if (cp > 1) {
                QSqlQuery checkCoverPage(destDB);
                checkCoverPage.prepare("SELECT coverPage FROM comic_info where hash = :hash");
                checkCoverPage.bindValue(":hash", record.value("hash").toString());
                checkCoverPage.exec();
                bool extract = false;
                if (checkCoverPage.next()) {
                    extract = checkCoverPage.record().value("coverPage").toInt() != cp;
                }
                if (extract)
                    hashes.append(record.value("hash").toString());
            }

            bindValuesFromRecord(record, update);

            update.bindValue(":edited", 1);

            update.exec();

            if (update.numRowsAffected() == 0) {

                bindValuesFromRecord(record, insert);
                insert.bindValue(":edited", 1);
                insert.bindValue(":read", 0);

                insert.exec();
            }
        }

        destDB.commit();
        QString hash;
        foreach (hash, hashes) {
            QSqlQuery getComic(destDB);
            getComic.prepare("SELECT c.path,ci.coverPage FROM comic c INNER JOIN comic_info ci ON (c.comicInfoId = ci.id) where ci.hash = :hash");
            getComic.bindValue(":hash", hash);
            getComic.exec();
            if (getComic.next()) {
                QString basePath = QString(dest).remove("/.yacreaderlibrary/library.ydb");
                QString path = basePath + getComic.record().value("path").toString();
                int coverPage = getComic.record().value("coverPage").toInt();
                ThumbnailCreator tc(path, basePath + "/.yacreaderlibrary/covers/" + hash + ".jpg", coverPage);
                tc.create();
            }
        }
        sourceDBconnection = sourceDB.connectionName();
        destDBconnection = sourceDB.connectionName();
    }

    QSqlDatabase::removeDatabase(sourceDBconnection);
    QSqlDatabase::removeDatabase(destDBconnection);
}
//TODO fix these bindings
void DataBaseManagement::bindValuesFromRecord(const QSqlRecord &record, QSqlQuery &query)
{
    bindString("title", record, query);

    bindInt("coverPage", record, query);
    bindInt("numPages", record, query);

    bindInt("number", record, query);
    bindInt("isBis", record, query);
    bindInt("count", record, query);

    bindString("volume", record, query);
    bindString("storyArc", record, query);
    bindInt("arcNumber", record, query);
    bindInt("arcCount", record, query);

    bindString("genere", record, query);

    bindString("writer", record, query);
    bindString("penciller", record, query);
    bindString("inker", record, query);
    bindString("colorist", record, query);
    bindString("letterer", record, query);
    bindString("coverArtist", record, query);

    bindString("date", record, query);
    bindString("publisher", record, query);
    bindString("format", record, query);
    bindInt("color", record, query);
    bindString("ageRating", record, query);
    bindInt("manga", record, query);

    bindString("synopsis", record, query);
    bindString("characters", record, query);
    bindString("notes", record, query);

    bindString("comicVineID", record, query);

    bindString("lastTimeOpened", record, query);

    bindDouble("coverSizeRatio", record, query);
    bindString("originalCoverSize", record, query);

    bindString("hash", record, query);
}

bool DataBaseManagement::addColumns(const QString &tableName, const QStringList &columnDefs, const QSqlDatabase &db)
{
    QString sql = "ALTER TABLE %1 ADD COLUMN %2";
    bool returnValue = true;

    foreach (QString columnDef, columnDefs) {
        QSqlQuery alterTable(db);
        alterTable.prepare(sql.arg(tableName).arg(columnDef));
        //alterTableComicInfo.bindValue(":column_def",columnDef);
        bool exec = alterTable.exec();
        returnValue = returnValue && exec;
        if (!exec) {
            QLOG_ERROR() << alterTable.lastError().text();
        }
        //returnValue = returnValue && (alterTable.numRowsAffected() > 0);
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

void DataBaseManagement::bindString(const QString &name, const QSqlRecord &record, QSqlQuery &query)
{
    if (!record.value(name).isNull()) {
        query.bindValue(":" + name, record.value(name).toString());
    }
}
void DataBaseManagement::bindInt(const QString &name, const QSqlRecord &record, QSqlQuery &query)
{
    if (!record.value(name).isNull()) {
        query.bindValue(":" + name, record.value(name).toInt());
    }
}

void DataBaseManagement::bindDouble(const QString &name, const QSqlRecord &record, QSqlQuery &query)
{
    if (!record.value(name).isNull()) {
        query.bindValue(":" + name, record.value(name).toDouble());
    }
}

QString DataBaseManagement::checkValidDB(const QString &fullPath)
{
    QString versionString = "";
    QString connectionName = "";
    {
        QSqlDatabase db = loadDatabaseFromFile(fullPath);

        if (db.isValid() && db.isOpen()) {
            QSqlQuery version(db);
            version.prepare("SELECT * FROM db_info");
            version.exec();

            if (version.next())
                versionString = version.record().value("version").toString();
        }
        connectionName = db.connectionName();
    }

    QSqlDatabase::removeDatabase(connectionName);

    return versionString;
}

int DataBaseManagement::compareVersions(const QString &v1, const QString v2)
{
    QStringList v1l = v1.split('.');
    QStringList v2l = v2.split('.');
    QList<int> v1il;
    QList<int> v2il;

    foreach (QString s, v1l)
        v1il.append(s.toInt());

    foreach (QString s, v2l)
        v2il.append(s.toInt());

    for (int i = 0; i < qMin(v1il.length(), v2il.length()); i++) {
        if (v1il[i] < v2il[i])
            return -1;
        if (v1il[i] > v2il[i])
            return 1;
    }

    if (v1il.length() < v2il.length())
        return -1;
    if (v1il.length() == v2il.length())
        return 0;
    if (v1il.length() > v2il.length())
        return 1;

    return 0;
}

bool DataBaseManagement::updateToCurrentVersion(const QString &path)
{
    bool pre7 = false;
    bool pre7_1 = false;
    bool pre8 = false;
    bool pre9_5 = false;
    bool pre9_8 = false;

    QString fullPath = path + "/library.ydb";

    if (compareVersions(DataBaseManagement::checkValidDB(fullPath), "7.0.0") < 0)
        pre7 = true;
    if (compareVersions(DataBaseManagement::checkValidDB(fullPath), "7.0.3") < 0)
        pre7_1 = true;
    if (compareVersions(DataBaseManagement::checkValidDB(fullPath), "8.0.0") < 0)
        pre8 = true;
    if (compareVersions(DataBaseManagement::checkValidDB(fullPath), "9.5.0") < 0)
        pre9_5 = true;
    if (compareVersions(DataBaseManagement::checkValidDB(fullPath), "9.8.0") < 0)
        pre9_8 = true;

    QString connectionName = "";
    bool returnValue = false;

    {
        QSqlDatabase db = loadDatabaseFromFile(fullPath);
        if (db.isValid() && db.isOpen()) {
            QSqlQuery updateVersion(db);
            updateVersion.prepare("UPDATE db_info SET "
                                  "version = :version");
            updateVersion.bindValue(":version", VERSION);
            updateVersion.exec();

            if (updateVersion.numRowsAffected() > 0)
                returnValue = true;

            if (pre7) //TODO: execute only if previous version was < 7.0
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

                bool successAddingColumns = addColumns("comic_info", columnDefs, db);
                returnValue = returnValue && successAddingColumns;
            }
            //TODO update hasBeenOpened value

            if (pre7_1) {
                {
                    QStringList columnDefs;
                    columnDefs << "finished BOOLEAN DEFAULT 0"
                               << "completed BOOLEAN DEFAULT 1";
                    bool successAddingColumns = addColumns("folder", columnDefs, db);
                    returnValue = returnValue && successAddingColumns;
                }

                { //comic_info
                    QStringList columnDefs;
                    columnDefs << "comicVineID TEXT DEFAULT NULL";
                    bool successAddingColumns = addColumns("comic_info", columnDefs, db);
                    returnValue = returnValue && successAddingColumns;
                }
            }

            if (pre8) {
                bool successCreatingNewTables = createV8Tables(db);
                returnValue = returnValue && successCreatingNewTables;
            }

            if (pre9_5) {
                { //folder
                    QStringList columnDefs;
                    //a full library update is needed after updating the table
                    columnDefs << "numChildren INTEGER";
                    columnDefs << "firstChildHash TEXT";
                    columnDefs << "customImage TEXT";
                    bool successAddingColumns = addColumns("folder", columnDefs, db);
                    returnValue = returnValue && successAddingColumns;
                }

                { //comic_info
                    QStringList columnDefs;
                    columnDefs << "lastTimeOpened INTEGER";
                    columnDefs << "coverSizeRatio REAL";
                    columnDefs << "originalCoverSize TEXT";
                    bool successAddingColumns = addColumns("comic_info", columnDefs, db);
                    returnValue = returnValue && successAddingColumns;

                    QSqlQuery queryIndexLastTimeOpened(db);
                    bool successCreatingIndex = queryIndexLastTimeOpened.exec("CREATE INDEX last_time_opened_index ON comic_info (lastTimeOpened)");
                    returnValue = returnValue && successCreatingIndex;
                }

                //update folders info
                {
                    DBHelper::updateChildrenInfo(db);
                }

                {
                    QSqlQuery selectQuery(db);
                    selectQuery.prepare("SELECT id, hash FROM comic_info");
                    selectQuery.exec();

                    db.transaction();

                    QSqlQuery updateCoverInfo(db);
                    updateCoverInfo.prepare("UPDATE comic_info SET coverSizeRatio = :coverSizeRatio WHERE id = :id");

                    QImageReader thumbnail;
                    while (selectQuery.next()) {
                        thumbnail.setFileName(path % "/covers/" % selectQuery.value(1).toString() % ".jpg");

                        float coverSizeRatio = static_cast<float>(thumbnail.size().width()) / thumbnail.size().height();
                        updateCoverInfo.bindValue(":coverSizeRatio", coverSizeRatio);
                        updateCoverInfo.bindValue(":id", selectQuery.value(0));

                        updateCoverInfo.exec();
                    }

                    db.commit();
                }
            }

            if (pre9_8) {
                { //comic_info
                    QStringList columnDefs;
                    columnDefs << "manga BOOLEAN DEFAULT 0";
                    bool successAddingColumns = addColumns("comic_info", columnDefs, db);
                    returnValue = returnValue && successAddingColumns;
                }
                { //folder
                    QStringList columnDefs;
                    columnDefs << "manga BOOLEAN DEFAULT 0";
                    bool successAddingColumns = addColumns("folder", columnDefs, db);
                    returnValue = returnValue && successAddingColumns;
                }
            }
        }
        connectionName = db.connectionName();
    }

    QSqlDatabase::removeDatabase(connectionName);
    return returnValue;
}

//COMICS_INFO_EXPORTER
ComicsInfoExporter::ComicsInfoExporter()
    : QThread()
{
}

void ComicsInfoExporter::exportComicsInfo(QSqlDatabase &source, QSqlDatabase &dest)
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
    : QThread()
{
}

void ComicsInfoImporter::importComicsInfo(QSqlDatabase &source, QSqlDatabase &dest)
{
    Q_UNUSED(source)
    Q_UNUSED(dest)
    //TODO check this method
}

void ComicsInfoImporter::run()
{
}
