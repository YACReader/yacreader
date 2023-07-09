#include "library_creator.h"
#include "custom_widgets.h"

#include <QMutex>
#include <QDebug>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QCoreApplication>
#include <QLibrary>

#include "data_base_management.h"
#include "qnaturalsorting.h"
#include "db_helper.h"

#include "initial_comic_info_extractor.h"
#include "xml_info_parser.h"
#include "comic.h"
#include "pdf_comic.h"
#include "yacreader_global.h"

#include "QsLog.h"

#include <algorithm>

using namespace std;
using namespace YACReader;

//--------------------------------------------------------------------------------
LibraryCreator::LibraryCreator(QSettings *settings)
    : creation(false), partialUpdate(false), settings(settings)
{
    _nameFilter << Comic::comicExtensions;
}

void LibraryCreator::createLibrary(const QString &source, const QString &target)
{
    creation = true;
    processLibrary(source, target);
}

void LibraryCreator::updateLibrary(const QString &source, const QString &target)
{
    checkModifiedDatesOnUpdate = settings->value(COMPARE_MODIFIED_DATE_ON_LIBRARY_UPDATES, false).toBool();
    partialUpdate = false;
    processLibrary(source, target);
}

void LibraryCreator::updateFolder(const QString &source, const QString &target, const QString &sourceFolder, const QModelIndex &dest)
{
    checkModifiedDatesOnUpdate = settings->value(COMPARE_MODIFIED_DATE_ON_LIBRARY_UPDATES, false).toBool();
    partialUpdate = true;
    folderDestinationModelIndex = dest;

    _currentPathFolders.clear();
    _currentPathFolders.append(Folder(1, 1, "root", "/"));

    QString relativeFolderPath = sourceFolder;
    relativeFolderPath = relativeFolderPath.remove(QDir::cleanPath(source));

    if (relativeFolderPath.startsWith("/")) {
        relativeFolderPath = relativeFolderPath.remove(0, 1); // remove firts '/'
    }

    QStringList folders;

    if (!relativeFolderPath.isEmpty()) // updating root
    {
        folders = relativeFolderPath.split('/');
    }

    QLOG_DEBUG() << "folders found in relative path : " << folders << "-" << relativeFolderPath;
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(target);

        foreach (QString folderName, folders) {
            if (folderName.isEmpty()) {
                break;
            }
            qulonglong parentId = _currentPathFolders.last().id;
            _currentPathFolders.append(DBHelper::loadFolder(folderName, parentId, db));
            QLOG_DEBUG() << "Folder appended : " << _currentPathFolders.last().id << " " << _currentPathFolders.last().name << " with parent" << _currentPathFolders.last().parentId;
        }
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);

    QLOG_DEBUG() << "Relative path : " << relativeFolderPath;

    _sourceFolder = sourceFolder;

    processLibrary(source, target);
}

void LibraryCreator::processLibrary(const QString &source, const QString &target)
{
    _source = source;
    _target = target;
    if (DataBaseManagement::checkValidDB(target + "/library.ydb") == "") {
        // se limpia el directorio ./yacreaderlibrary
        QDir d(target);
        d.removeRecursively();
        _mode = CREATOR;
    } else { //
        _mode = UPDATER;
    }
}

//
void LibraryCreator::run()
{
    stopRunning = false;
#if !defined use_unarr && !defined use_libarchive
// check for 7z lib
#if defined Q_OS_UNIX && !defined Q_OS_MACOS
    QLibrary *sevenzLib = new QLibrary(QString(LIBDIR) + "/p7zip/7z.so");
#else
    QLibrary *sevenzLib = new QLibrary(QCoreApplication::applicationDirPath() + "/utils/7z");
#endif

    if (!sevenzLib->load()) {
        QLOG_ERROR() << "Loading 7z.dll : " + sevenzLib->errorString() << Qt::endl;
        QCoreApplication::exit(YACReader::SevenZNotFound);
        exit();
    }
    sevenzLib->deleteLater();
#endif
    if (_mode == CREATOR) {
        QLOG_INFO() << "Starting to create new library ( " << _source << "," << _target << ")";
        _currentPathFolders.clear();
        _currentPathFolders.append(Folder(1, 1, "root", "/"));
        // se crean los directorios .yacreaderlibrary y .yacreaderlibrary/covers
        QDir dir;
        dir.mkpath(_target + "/covers");

        // se crea la base de datos .yacreaderlibrary/library.ydb
        {
            auto _database = DataBaseManagement::createDatabase("library", _target); //
            _databaseConnection = _database.connectionName();
            if (!_database.isOpen()) {
                QLOG_ERROR() << "Unable to create data base" << _database.lastError().databaseText() + "-" + _database.lastError().driverText();
                emit failedCreatingDB(_database.lastError().databaseText() + "-" + _database.lastError().driverText());
                emit finished();
                creation = false;
                return;
            }

            /*QSqlQuery pragma("PRAGMA foreign_keys = ON",_database);*/
            _database.transaction();
            // se crea la librería
            create(QDir(_source));

            DBHelper::updateChildrenInfo(_database);

            _database.commit();
            _database.close();
        }
        QSqlDatabase::removeDatabase(_databaseConnection);
        emit created();
        QLOG_INFO() << "Create library END";
    } else {
        QLOG_INFO() << "Starting to update folder" << _sourceFolder << "in library ( " << _source << "," << _target << ")";
        if (!partialUpdate) {
            _currentPathFolders.clear();
            _currentPathFolders.append(Folder(1, 1, "root", "/"));
            QLOG_DEBUG() << "update whole library";
        }
        {
            auto _database = DataBaseManagement::loadDatabase(_target);
            _databaseConnection = _database.connectionName();

            //_database.setDatabaseName(_target+"/library.ydb");
            if (!_database.open()) {
                QLOG_ERROR() << "Unable to open data base" << _database.lastError().databaseText() + "-" + _database.lastError().driverText();
                emit failedOpeningDB(_database.lastError().databaseText() + "-" + _database.lastError().driverText());
                emit finished();
                creation = false;
                return;
            }
            QSqlQuery pragma("PRAGMA foreign_keys = ON", _database);
            pragma.exec();
            _database.transaction();

            if (partialUpdate) {
                update(QDir(_sourceFolder));
            } else {
                update(QDir(_source));
            }

            if (partialUpdate) {
                auto folder = DBHelper::updateChildrenInfo(folderDestinationModelIndex.data(FolderModel::IdRole).toULongLong(), _database);
                DBHelper::propagateFolderUpdatesToParent(folder, _database);
            } else
                DBHelper::updateChildrenInfo(_database);

            _database.commit();
            _database.close();
        }

        QSqlDatabase::removeDatabase(_databaseConnection);

        // si estabamos en modo creación, se está añadiendo una librería que ya existía y se ha actualizado antes de añadirse.
        if (!partialUpdate) {
            if (!creation) {
                emit updated();
            } else {
                emit created();
            }
        }
        QLOG_INFO() << "Update library END";
    }
    // msleep(100);//TODO try to solve the problem with the udpate dialog (ya no se usa más...)
    if (partialUpdate) {
        emit updatedCurrentFolder(folderDestinationModelIndex);
        emit finished();
    } else // TODO check this part!!
        emit finished();
    creation = false;
}

void LibraryCreator::stop()
{
    QSqlDatabase::database(_databaseConnection).commit();
    stopRunning = true;
}

// retorna el id del ultimo de los folders
qulonglong LibraryCreator::insertFolders()
{
    auto _database = QSqlDatabase::database(_databaseConnection);
    QList<Folder>::iterator i;
    int currentId = 0;
    Folder currentParent;
    for (i = _currentPathFolders.begin(); i != _currentPathFolders.end(); ++i) {
        if (!(i->knownId)) {
            i->setFather(currentId);
            i->type = currentParent.type;
            currentId = DBHelper::insert(&(*i), _database); // insertFolder(currentId,*i);
            i->setId(currentId);
        } else {
            currentId = i->id;
        }

        currentParent = *i;
    }
    return currentId;
}

void LibraryCreator::create(QDir dir)
{
    dir.setNameFilters(_nameFilter);
    dir.setFilter(QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot);
    QFileInfoList list = dir.entryInfoList();
    for (int i = 0; i < list.size(); ++i) {
        if (stopRunning)
            return;
        QFileInfo fileInfo = list.at(i);
        QString fileName = fileInfo.fileName();
#ifdef Q_OS_MACOS
        QStringList src = _source.split("/");
        QString filePath = fileInfo.absoluteFilePath();
        QStringList fp = filePath.split("/");
        for (int i = 0; i < src.count(); i++) {
            fp.removeFirst();
        }
        QString relativePath = "/" + fp.join("/");
#else
        QString relativePath = QDir::cleanPath(fileInfo.absoluteFilePath()).remove(_source);
#endif
        if (fileInfo.isDir()) {
            QLOG_TRACE() << "Parsing folder" << fileInfo.canonicalPath();
            // se añade al path actual el folder, aún no se sabe si habrá que añadirlo a la base de datos
            _currentPathFolders.append(Folder(fileInfo.fileName(), relativePath));
            create(QDir(fileInfo.absoluteFilePath()));
            // una vez importada la información del folder, se retira del path actual ya que no volverá a ser visitado
            _currentPathFolders.pop_back();
        } else {
            QLOG_TRACE() << "Parsing file" << fileInfo.filePath();
            insertComic(relativePath, fileInfo);
        }
    }
}

bool LibraryCreator::checkCover(const QString &hash)
{
    return QFile::exists(_target + "/covers/" + hash + ".jpg");
}

QString pseudoHash(const QFileInfo &fileInfo)
{
    QCryptographicHash crypto(QCryptographicHash::Sha1);
    QFile file(fileInfo.absoluteFilePath());
    file.open(QFile::ReadOnly);
    crypto.addData(file.read(524288));
    file.close();
    // hash Sha1 del primer 0.5MB + filesize
    return QString(crypto.result().toHex().constData()) + QString::number(fileInfo.size());
}

void LibraryCreator::insertComic(const QString &relativePath, const QFileInfo &fileInfo)
{
    auto _database = QSqlDatabase::database(_databaseConnection);

    QString hash = pseudoHash(fileInfo);

    ComicDB comic = DBHelper::loadComic(fileInfo.fileName(), relativePath, hash, _database);
    int numPages = 0;
    QPair<int, int> originalCoverSize = { 0, 0 };
    bool exists = checkCover(hash);

    YACReader::InitialComicInfoExtractor ie(QDir::cleanPath(fileInfo.absoluteFilePath()), _target + "/covers/" + hash + ".jpg", comic.info.coverPage.toInt(), settings->value(IMPORT_COMIC_INFO_XML_METADATA, false).toBool());

    if (!(comic.hasCover() && exists)) {
        ie.extract();
        numPages = ie.getNumPages();
        originalCoverSize = ie.getOriginalCoverSize();
        if (numPages > 0) {
            emit comicAdded(relativePath, _target + "/covers/" + hash + ".jpg");
        }
    }

    if (numPages > 0 || exists) {
        // en este punto sabemos que todos los folders que hay en _currentPath, deberían estar añadidos a la base de datos
        insertFolders();

        bool parsed = YACReader::parseXMLIntoInfo(ie.getXMLInfoRawData(), comic.info);

        comic.info.numPages = numPages;
        if (originalCoverSize.second > 0) {
            comic.info.originalCoverSize = QString("%1x%2").arg(originalCoverSize.first).arg(originalCoverSize.second);
            comic.info.coverSizeRatio = static_cast<float>(originalCoverSize.first) / originalCoverSize.second;
        }

        comic.parentId = _currentPathFolders.last().id;
        comic.info.type = QVariant::fromValue(_currentPathFolders.last().type); // TODO_METADATA test this

        DBHelper::insert(&comic, _database, parsed);
    }
}

void LibraryCreator::replaceComic(const QString &relativePath, const QFileInfo &fileInfo, ComicDB *comic)
{
    auto _database = QSqlDatabase::database(_databaseConnection);

    DBHelper::removeFromDB(comic, _database);
    insertComic(relativePath, fileInfo);

    QString hash = pseudoHash(fileInfo);

    ComicDB insertedComic = DBHelper::loadComic(fileInfo.fileName(), relativePath, hash, _database);

    if (!insertedComic.info.existOnDb) {
        return;
    }

    if (insertedComic.info.coverSizeRatio.isNull()) {
        return;
    }

    auto numPages = insertedComic.info.numPages;
    auto coverSize = insertedComic.info.originalCoverSize;
    auto coverRatio = insertedComic.info.coverSizeRatio;
    auto id = insertedComic.info.id;

    insertedComic.info = comic->info;

    insertedComic.info.numPages = numPages;
    insertedComic.info.originalCoverSize = coverSize;
    insertedComic.info.coverSizeRatio = coverRatio;
    insertedComic.info.id = id;
    insertedComic.info.coverPage = 1;
    insertedComic.info.added = fileInfo.lastModified().toSecsSinceEpoch();

    DBHelper::update(&(insertedComic.info), _database);
}

void LibraryCreator::update(QDir dirS)
{
    auto _database = QSqlDatabase::database(_databaseConnection);
    // QLOG_TRACE() << "Updating" << dirS.absolutePath();
    // QLOG_TRACE() << "Getting info from dir" << dirS.absolutePath();
    dirS.setNameFilters(_nameFilter);
    dirS.setFilter(QDir::AllDirs | QDir::NoDotAndDotDot);
    dirS.setSorting(QDir::Name | QDir::IgnoreCase | QDir::LocaleAware);
    QFileInfoList listSFolders = dirS.entryInfoList();
    dirS.setFilter(QDir::Files | QDir::NoDotAndDotDot);
    dirS.setSorting(QDir::Name | QDir::IgnoreCase | QDir::LocaleAware);
    QFileInfoList listSFiles = dirS.entryInfoList();

    std::sort(listSFolders.begin(), listSFolders.end(), naturalSortLessThanCIFileInfo);
    std::sort(listSFiles.begin(), listSFiles.end(), naturalSortLessThanCIFileInfo);

    QFileInfoList listS;
    listS.append(listSFolders);
    listS.append(listSFiles);
    // QLOG_DEBUG() << "---------------------------------------------------------";
    // foreach(QFileInfo info,listS)
    //	QLOG_DEBUG() << info.fileName();

    // QLOG_TRACE() << "END Getting info from dir" << dirS.absolutePath();

    // QLOG_TRACE() << "Getting info from DB" << dirS.absolutePath();
    QList<LibraryItem *> folders = DBHelper::getFoldersFromParent(_currentPathFolders.last().id, _database);
    QList<LibraryItem *> comics = DBHelper::getComicsFromParent(_currentPathFolders.last().id, _database);
    // QLOG_TRACE() << "END Getting info from DB" << dirS.absolutePath();

    QList<LibraryItem *> listD;
    std::sort(folders.begin(), folders.end(), naturalSortLessThanCILibraryItem);
    std::sort(comics.begin(), comics.end(), naturalSortLessThanCILibraryItem);
    listD.append(folders);
    listD.append(comics);
    // QLOG_DEBUG() << "---------------------------------------------------------";
    // foreach(LibraryItem * info,listD)
    //	QLOG_DEBUG() << info->name;
    // QLOG_DEBUG() << "---------------------------------------------------------";
    int lenghtS = listS.size();
    int lenghtD = listD.size();
    // QLOG_DEBUG() << "S len" << lenghtS << "D len" << lenghtD;
    // QLOG_DEBUG() << "---------------------------------------------------------";

    bool updated;
    int i, j;
    for (i = 0, j = 0; (i < lenghtS) || (j < lenghtD);) {
        if (stopRunning) {
            qDeleteAll(listD);
            return;
        }
        updated = false;
        if (i >= lenghtS) // finished source files/dirs
        {
            // QLOG_WARN() << "finished source files/dirs" << dirS.absolutePath();
            // delete listD //from j
            for (; j < lenghtD; j++) {
                if (stopRunning) {
                    qDeleteAll(listD);
                    return;
                }
                DBHelper::removeFromDB(listD.at(j), (_database));
            }
            updated = true;
        }
        if (j >= lenghtD) // finished library files/dirs
        {
            // QLOG_WARN() << "finished library files/dirs" << dirS.absolutePath();
            // create listS //from i
            for (; i < lenghtS; i++) {
                if (stopRunning) {
                    qDeleteAll(listD);
                    return;
                }
                QFileInfo fileInfoS = listS.at(i);
                if (fileInfoS.isDir()) // create folder
                {
#ifdef Q_OS_MACOS
                    QStringList src = _source.split("/");
                    QString filePath = fileInfoS.absoluteFilePath();
                    QStringList fp = filePath.split("/");
                    for (int i = 0; i < src.count(); i++) {
                        fp.removeFirst();
                    }
                    QString path = "/" + fp.join("/");
#else
                    QString path = QDir::cleanPath(fileInfoS.absoluteFilePath()).remove(_source);
#endif
                    _currentPathFolders.append(Folder(fileInfoS.fileName(), path)); // folder actual no está en la BD
                    create(QDir(fileInfoS.absoluteFilePath()));
                    _currentPathFolders.pop_back();
                } else // create comic
                {
#ifdef Q_OS_MACOS
                    QStringList src = _source.split("/");
                    QString filePath = fileInfoS.absoluteFilePath();
                    QStringList fp = filePath.split("/");
                    for (int i = 0; i < src.count(); i++) {
                        fp.removeFirst();
                    }
                    QString path = "/" + fp.join("/");
#else

                    QString path = QDir::cleanPath(fileInfoS.absoluteFilePath()).remove(_source);
#endif
                    insertComic(path, fileInfoS);
                }
            }
            updated = true;
        }
        if (!updated) {
            QFileInfo fileInfoS = listS.at(i);
            LibraryItem *fileInfoD = listD.at(j);
            QString nameS = QDir::cleanPath(fileInfoS.absoluteFilePath()).remove(QDir::cleanPath(fileInfoS.absolutePath())); // remove source
            QString nameD = "/" + fileInfoD->name;

            int comparation = QString::localeAwareCompare(nameS, nameD);
            if (fileInfoS.isDir() && fileInfoD->isDir())
                if (comparation == 0) // same folder, update
                {
                    _currentPathFolders.append(*static_cast<Folder *>(fileInfoD)); // fileInfoD conoce su padre y su id
                    update(QDir(fileInfoS.absoluteFilePath()));
                    _currentPathFolders.pop_back();
                    i++;
                    j++;
                } else if (comparation < 0) // nameS doesn't exist on DB
                {

                    if (nameS != "/.yacreaderlibrary") {
                        // QLOG_WARN() << "dir source < dest" << nameS << nameD;
#ifdef Q_OS_MACOS
                        QStringList src = _source.split("/");
                        QString filePath = fileInfoS.absoluteFilePath();
                        QStringList fp = filePath.split("/");
                        for (int i = 0; i < src.count(); i++) {
                            fp.removeFirst();
                        }
                        QString path = "/" + fp.join("/");
#else
                        QString path = QDir::cleanPath(fileInfoS.absoluteFilePath()).remove(_source);
#endif
                        _currentPathFolders.append(Folder(fileInfoS.fileName(), path));
                        create(QDir(fileInfoS.absoluteFilePath()));
                        _currentPathFolders.pop_back();
                    }
                    i++;
                } else // nameD no longer available on Source folder...
                {
                    if (nameS != "/.yacreaderlibrary") {
                        // QLOG_WARN() << "dir source > dest" << nameS << nameD;
                        DBHelper::removeFromDB(fileInfoD, _database);
                        j++;
                    } else
                        i++; // skip library directory
                }
            else // one of them(or both) is a file
                if (fileInfoS.isDir()) // this folder doesn't exist on library
                {
                    if (nameS != "/.yacreaderlibrary") // skip .yacreaderlibrary folder
                    {
                        // QLOG_WARN() << "one of them(or both) is a file" << nameS << nameD;
#ifdef Q_OS_MACOS
                        QStringList src = _source.split("/");
                        QString filePath = fileInfoS.absoluteFilePath();
                        QStringList fp = filePath.split("/");
                        for (int i = 0; i < src.count(); i++) {
                            fp.removeFirst();
                        }
                        QString path = "/" + fp.join("/");
#else
                        QString path = QDir::cleanPath(fileInfoS.absoluteFilePath()).remove(_source);
#endif
                        _currentPathFolders.append(Folder(fileInfoS.fileName(), path));
                        create(QDir(fileInfoS.absoluteFilePath()));
                        _currentPathFolders.pop_back();
                    }
                    i++;
                } else if (fileInfoD->isDir()) // delete this folder from library
                {
                    DBHelper::removeFromDB(fileInfoD, _database);
                    j++;
                } else // both are files  //BUG on windows (no case sensitive)
                {
                    // nameD.remove(nameD.size()-4,4);
                    int comparation = QString::localeAwareCompare(nameS, nameD);
                    if (comparation < 0) // create new thumbnail
                    {
#ifdef Q_OS_MACOS
                        QStringList src = _source.split("/");
                        QString filePath = fileInfoS.absoluteFilePath();
                        QStringList fp = filePath.split("/");
                        for (int i = 0; i < src.count(); i++) {
                            fp.removeFirst();
                        }
                        QString path = "/" + fp.join("/");
#else
                        QString path = QDir::cleanPath(fileInfoS.absoluteFilePath()).remove(_source);
#endif
                        insertComic(path, fileInfoS);
                        i++;
                    } else {
                        if (comparation > 0) // delete thumbnail
                        {
                            DBHelper::removeFromDB(fileInfoD, _database);
                            j++;
                        } else // file with the same name
                        {
                            if (fileInfoS.isFile() && !fileInfoD->isDir()) {
                                auto comicDB = static_cast<ComicDB *>(fileInfoD);
                                auto lastModified = fileInfoS.lastModified().toSecsSinceEpoch();
                                auto added = comicDB->info.added.toULongLong();

                                auto sizeHasChanged = comicDB->getFileSize() != fileInfoS.size();
                                auto hasBeenModified = added > 0 && added < lastModified && checkModifiedDatesOnUpdate;

                                if (sizeHasChanged || hasBeenModified) {
#ifdef Q_OS_MACOS
                                    QStringList src = _source.split("/");
                                    QString filePath = fileInfoS.absoluteFilePath();
                                    QStringList fp = filePath.split("/");
                                    for (int i = 0; i < src.count(); i++) {
                                        fp.removeFirst();
                                    }
                                    QString path = "/" + fp.join("/");
#else
                                    QString path = QDir::cleanPath(fileInfoS.absoluteFilePath()).remove(_source);
#endif
                                    replaceComic(path, fileInfoS, comicDB);
                                    QLOG_INFO() << "Repaced" << QDir::cleanPath(fileInfoS.absoluteFilePath()).remove(_source) << " last modified:  " << fileInfoS.lastModified() << " added: " << QDateTime::fromSecsSinceEpoch(added);
                                } else if (added == 0) { // this file was added before `added` existed on the db, `added` will be updated to match the modified date so future modifications can be detected.
                                    if (lastModified > 0) {
                                        comicDB->info.added = lastModified;
                                        DBHelper::updateAdded(&(comicDB->info), _database);
                                    }
                                }
                            }
                            i++;
                            j++;
                        }
                    }
                }
        }
    }

    qDeleteAll(listD);
}
