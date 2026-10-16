#include "library_creator.h"

#include "QsLog.h"
#include "comic.h"
#include "data_base_management.h"
#include "db_helper.h"
#include "initial_comic_info_extractor.h"
#include "library_item_types.h"
#include "overloaded.h"
#include "pdf_comic.h"
#include "qnaturalsorting.h"
#include "xml_info_parser.h"
#include "yacreader_global.h"

#include <QCoreApplication>
#include <QDebug>
#include <QLibrary>
#include <QMutex>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>

#include <algorithm>
#include <utility>
#include <variant>

using namespace std;
using namespace YACReader;

Folder rootFolder(QSqlDatabase &db)
{
    QSqlQuery selectQuery(db);
    selectQuery.prepare("SELECT * FROM folder WHERE id = 1");
    selectQuery.exec();

    Folder root;
    root.id = FolderId(1);
    root.parentId = FolderId(1);
    root.name = "root";
    root.path = "/";

    if (!selectQuery.next()) {

        root.type = YACReader::FileType::Comic;
        return root;
    }

    root.type = selectQuery.value("type").value<YACReader::FileType>();

    return root;
}

Folder newFolder(const QString &name, const QString &path)
{
    Folder folder;
    folder.name = name;
    folder.path = path;
    return folder;
}

//--------------------------------------------------------------------------------
LibraryCreator::LibraryCreator(QSettings *settings)
    : creation(false), partialUpdate(false), settings(settings)
{
    _nameFilter = Comic::comicExtensions;
}

void LibraryCreator::createLibrary(const QString &source, const QString &target)
{
    creation = true;
    processLibrary(source, target);
}

void LibraryCreator::updateLibrary(const QString &source, const QString &target)
{
    creation = false;
    checkModifiedDatesOnUpdate = settings->value(COMPARE_MODIFIED_DATE_ON_LIBRARY_UPDATES, false).toBool();
    partialUpdate = false;
    _source = source;
    _target = target;
    _mode = UPDATER;
}

void LibraryCreator::updateFolder(const QString &source, const QString &target, const QString &sourceFolder, qulonglong folderId)
{
    creation = false;
    checkModifiedDatesOnUpdate = settings->value(COMPARE_MODIFIED_DATE_ON_LIBRARY_UPDATES, false).toBool();
    partialUpdate = true;
    _folderDestinationId = folderId;

    _currentPathFolders.clear();

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

        if (!db.isValid()) {
            QString error = "Unable to find database at: " + _target;
            QLOG_ERROR() << error;
            emit failedOpeningDB(error);
            return;
        }

        _currentPathFolders.append({ rootFolder(db), true });

        for (const auto &folderName : std::as_const(folders)) {
            if (folderName.isEmpty()) {
                break;
            }
            qulonglong parentId = _currentPathFolders.last().folder.id.value();
            if (auto folder = DBHelper::loadFolder(folderName, parentId, db)) {
                _currentPathFolders.append({ *folder, true });
            } else {
                Folder missing;
                missing.parentId = FolderId(parentId);
                _currentPathFolders.append({ missing, false });
            }
            const auto &appended = _currentPathFolders.last().folder;
            QLOG_DEBUG() << "Folder appended : " << appended.id.value() << " " << appended.name << " with parent" << appended.parentId.value();
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
        _mode = creation ? CREATOR : UPDATER;
    } else { //
        _mode = UPDATER;
    }
}

//
void LibraryCreator::run()
{
    stopRunning = false;
    canceled = false;
#if !defined use_unarr && !defined use_libarchive
    auto sevenzLib = YACReader::load7zLibrary();

    if (!sevenzLib->load()) {
        QLOG_ERROR() << "Loading 7z.dll : " + sevenzLib->errorString() << Qt::endl;
        QCoreApplication::exit(YACReader::SevenZNotFound);
        exit();
    }
    sevenzLib->deleteLater();
#endif
    if (_mode == CREATOR)
        QDir().mkpath(_target);

    LibraryMaintenanceLock maintenanceLock(_source);
    if (!maintenanceLock.tryLock()) {
        const auto error = maintenanceLock.errorString();
        QLOG_ERROR() << error;
        if (_mode == CREATOR)
            emit failedCreatingDB(error);
        else
            emit failedOpeningDB(error);
        return;
    }

    if (_mode == UPDATER) {
        QString recoveryError;
        if (!DataBaseManagement::recoverInterruptedRestore(_source, &recoveryError, true)) {
            QLOG_ERROR() << recoveryError;
            emit failedOpeningDB(recoveryError);
            return;
        }
    }

    if (_mode == CREATOR) {
        QLOG_INFO() << "Starting to create new library ( " << _source << "," << _target << ")";
        QString cleanupError;
        if (!DataBaseManagement::prepareForRecreation(_source, &cleanupError, true)) {
            QLOG_ERROR() << cleanupError;
            emit failedCreatingDB(cleanupError);
            return;
        }
        _currentPathFolders.clear();
        // se crean los directorios .yacreaderlibrary y .yacreaderlibrary/covers
        QDir dir;
        dir.mkpath(LibraryPaths::libraryCoversPathFromLibraryDataPath(_target));

        // se crea la base de datos .yacreaderlibrary/library.ydb
        {
            auto _database = DataBaseManagement::createDatabase("library", _target); //
            _databaseConnection = _database.connectionName();
            if (!_database.isOpen()) {
                QLOG_ERROR() << "Unable to create data base" << _database.lastError().databaseText() + "-" + _database.lastError().driverText();
                emit failedCreatingDB(_database.lastError().databaseText() + "-" + _database.lastError().driverText());
                creation = false;
                return;
            }

            _currentPathFolders.append({ rootFolder(_database), true });

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
        QString backupError;
        if (!DataBaseManagement::backupLibrary(_source, DatabaseBackupReason::AutoUpdate, &backupError)) {
            const auto error = QString("Unable to back up library database: %1").arg(backupError);
            QLOG_ERROR() << error;
            emit failedOpeningDB(error);
            return;
        }
        {
            auto _database = DataBaseManagement::loadDatabase(_target);

            if (!_database.isValid()) {
                QString error = "Unable to find database at: " + _target;
                QLOG_ERROR() << error;
                emit failedOpeningDB(error);
                return;
            }

            _databaseConnection = _database.connectionName();

            //_database.setDatabaseName(_target+"/library.ydb");
            if (!_database.open()) {
                QLOG_ERROR() << "Unable to open database" << _database.lastError().databaseText() + "-" + _database.lastError().driverText();
                emit failedOpeningDB(_database.lastError().databaseText() + "-" + _database.lastError().driverText());
                creation = false;
                return;
            }

            if (!partialUpdate) {
                _currentPathFolders.clear();
                _currentPathFolders.append({ rootFolder(_database), true });
                QLOG_DEBUG() << "update whole library";
            }

            QSqlQuery pragma("PRAGMA foreign_keys = ON", _database);
            pragma.exec();
            _database.transaction();

            if (partialUpdate) {
                update(QDir(_sourceFolder));
            } else {
                update(QDir(_source));
            }

            QStringList unusedCoverHashes;
            if (!canceled) {
                if (partialUpdate) {
                    auto folder = DBHelper::updateChildrenInfo(_folderDestinationId, _database);
                    DBHelper::propagateFolderUpdatesToParent(folder, _database);
                } else {
                    DBHelper::updateChildrenInfo(_database);
                    unusedCoverHashes = cleanup(_database, _target);
                }
            }

            if (_database.commit()) {
                for (const auto &hash : std::as_const(unusedCoverHashes)) {
                    QFile::remove(LibraryPaths::coverPathFromLibraryDataPath(_target, hash));
                }
            }
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

    if (partialUpdate) {
        emit updatedCurrentFolder(_folderDestinationId);
    }

    creation = false;
}

void LibraryCreator::stop()
{
    QSqlDatabase::database(_databaseConnection).commit();
    stopRunning = true;
}

void LibraryCreator::cancel()
{
    QSqlDatabase::database(_databaseConnection).rollback();
    canceled = true;
    stopRunning = true;
}

QStringList LibraryCreator::cleanup(QSqlDatabase &db, const QString &target)
{
    auto coversPath = LibraryPaths::libraryCoversPathFromLibraryDataPath(target);
    QDir coversDir(coversPath);
    if (!coversDir.exists()) {
        return { };
    }

    return DBHelper::removeUnusedComicInfo(db);
}

// retorna el id del ultimo de los folders
qulonglong LibraryCreator::insertFolders()
{
    auto _database = QSqlDatabase::database(_databaseConnection);
    QList<ScanFolder>::iterator i;
    int currentId = 0;
    Folder currentParent;
    for (i = _currentPathFolders.begin(); i != _currentPathFolders.end(); ++i) {
        if (!i->inDatabase) {
            i->folder.parentId = FolderId(currentId);
            i->folder.type = currentParent.type;
            currentId = DBHelper::insert(&i->folder, _database); // insertFolder(currentId,*i);
            i->folder.id = FolderId(currentId);
            i->inDatabase = true;
        } else {
            currentId = i->folder.id.value();
        }

        currentParent = i->folder;
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
            _currentPathFolders.append({ newFolder(fileInfo.fileName(), relativePath), false });
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
    return QFile::exists(LibraryPaths::coverPathFromLibraryDataPath(_target, hash));
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

bool LibraryCreator::insertComic(const QString &relativePath, const QFileInfo &fileInfo)
{
    auto _database = QSqlDatabase::database(_databaseConnection);

    QString hash = pseudoHash(fileInfo);

    ComicDB comic = DBHelper::loadComic(fileInfo.fileName(), relativePath, hash, _database);
    int numPages = 0;
    QPair<int, int> originalCoverSize = { 0, 0 };
    bool exists = checkCover(hash);

    auto coverPath = LibraryPaths::coverPathFromLibraryDataPath(_target, hash);
    YACReader::InitialComicInfoExtractor ie(QDir::cleanPath(fileInfo.absoluteFilePath()), coverPath, comic.info.coverPage.toInt(), settings->value(IMPORT_COMIC_INFO_XML_METADATA, false).toBool());

    // A comic_info row without pages describes a file that never produced a comic, so it
    // is no proof that the file is one: check it again instead of taking the shortcut.
    const bool knownComic = comic.info.id != ComicInfoId() && comic.info.numPages.toInt() > 0;

    if (!(knownComic && exists)) {
        ie.extract();
        if (!ie.isFileSupported()) {
            // Not a comic YACReader can show, so it does not belong to the library. It
            // leaves no cover behind either, otherwise the next scan would take that
            // cover as proof that the file had been imported before.
            return false;
        }
        numPages = ie.getNumPages();
        originalCoverSize = ie.getOriginalCoverSize();
        if (numPages > 0) {
            emit comicAdded(relativePath, coverPath);
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

        comic.file.folderId = _currentPathFolders.last().folder.id;
        comic.info.type = QVariant::fromValue(_currentPathFolders.last().folder.type);

        return DBHelper::insert(&comic, _database, parsed) != 0;
    }

    return false;
}

ReplaceComicResult classifyReplaceComic(ComicInfoId oldComicInfoId,
                                        std::optional<ComicInfoId> newComicInfoId,
                                        bool newComicInfoExisted)
{
    if (!newComicInfoId) {
        return ReplaceComicResult::NotInserted;
    }

    if (*newComicInfoId == oldComicInfoId) {
        return ReplaceComicResult::SameComicInfo;
    }

    return newComicInfoExisted ? ReplaceComicResult::KnownComicInfo : ReplaceComicResult::NewComicInfo;
}

void LibraryCreator::replaceComic(const QString &relativePath, const QFileInfo &fileInfo, ComicDB *comic)
{
    auto _database = QSqlDatabase::database(_databaseConnection);

    const auto oldComicInfoId = comic->info.id;
    const auto hash = pseudoHash(fileInfo);
    const bool newComicInfoExisted = DBHelper::loadComicInfo(hash, _database).has_value();

    QSqlQuery savepoint(_database);
    if (!savepoint.exec("SAVEPOINT replace_comic")) {
        QLOG_ERROR() << "Unable to replace" << relativePath << ":" << savepoint.lastError().text();
        return;
    }

    DBHelper::removeFromDB(comic, _database);

    std::optional<ComicInfo> newInfo;
    if (insertComic(relativePath, fileInfo)) {
        newInfo = DBHelper::loadComicInfo(hash, _database);
    }

    bool keepChanges = true;
    switch (classifyReplaceComic(oldComicInfoId, newInfo ? std::optional(newInfo->id) : std::nullopt, newComicInfoExisted)) {
    case ReplaceComicResult::NotInserted:
        keepChanges = false;
        break;
    case ReplaceComicResult::SameComicInfo:
        comic->info.added = fileInfo.lastModified().toSecsSinceEpoch();
        keepChanges = DBHelper::updateAdded(&comic->info, _database);
        break;
    case ReplaceComicResult::NewComicInfo:
        keepChanges = copyReplacedComicInfo(comic->info, *newInfo, fileInfo) && DBHelper::moveEntriesToComicInfo(oldComicInfoId, newInfo->id, _database);
        break;
    case ReplaceComicResult::KnownComicInfo:
        keepChanges = DBHelper::moveEntriesToComicInfo(oldComicInfoId, newInfo->id, _database);
        break;
    }

    if (!keepChanges) {
        savepoint.exec("ROLLBACK TO replace_comic");
    }
    savepoint.exec("RELEASE replace_comic");
}

// the new comic_info takes the values of the old one, except what comes from the new file
bool LibraryCreator::copyReplacedComicInfo(const ComicInfo &oldInfo, ComicInfo newInfo, const QFileInfo &fileInfo)
{
    if (newInfo.coverSizeRatio.isNull()) {
        return true;
    }

    auto _database = QSqlDatabase::database(_databaseConnection);

    const auto numPages = newInfo.numPages;
    const auto coverSize = newInfo.originalCoverSize;
    const auto coverRatio = newInfo.coverSizeRatio;
    const auto id = newInfo.id;

    newInfo = oldInfo;

    newInfo.numPages = numPages;
    newInfo.originalCoverSize = coverSize;
    newInfo.coverSizeRatio = coverRatio;
    newInfo.id = id;
    newInfo.coverPage = 1;
    newInfo.added = fileInfo.lastModified().toSecsSinceEpoch();

    return DBHelper::update(&newInfo, _database);
}

void LibraryCreator::update(QDir dirS)
{
    if (stopRunning) {
        return;
    }

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
    // for (const QFileInfo &info : listS)
    //	QLOG_DEBUG() << info.fileName();

    // QLOG_TRACE() << "END Getting info from dir" << dirS.absolutePath();

    // QLOG_TRACE() << "Getting info from DB" << dirS.absolutePath();
    QList<Folder> folders = DBHelper::getFoldersFromParent(_currentPathFolders.last().folder.id.value(), _database);
    QList<ComicDB> comics = DBHelper::getComicsFromParent(_currentPathFolders.last().folder.id.value(), _database);
    // QLOG_TRACE() << "END Getting info from DB" << dirS.absolutePath();

    QList<DiskItem> listD;
    std::sort(folders.begin(), folders.end(), [](const Folder &left, const Folder &right) {
        return naturalSortLessThanCI(left.name, right.name);
    });
    std::sort(comics.begin(), comics.end(), [](const ComicDB &left, const ComicDB &right) {
        return naturalSortLessThanCI(left.file.fileName, right.file.fileName);
    });
    for (const auto &folder : folders)
        listD.append(folder);
    for (const auto &comic : comics)
        listD.append(comic);
    // QLOG_DEBUG() << "---------------------------------------------------------";
    // for (auto *info : listD)
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
            return;
        }
        updated = false;
        if (i >= lenghtS) // finished source files/dirs
        {
            // QLOG_WARN() << "finished source files/dirs" << dirS.absolutePath();
            // delete listD //from j
            for (; j < lenghtD; j++) {
                if (stopRunning) {
                    return;
                }
                DBHelper::removeFromDB(listD[j], (_database));
            }
            updated = true;
        }
        if (j >= lenghtD) // finished library files/dirs
        {
            // QLOG_WARN() << "finished library files/dirs" << dirS.absolutePath();
            // create listS //from i
            for (; i < lenghtS; i++) {
                if (stopRunning) {
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
                    _currentPathFolders.append({ newFolder(fileInfoS.fileName(), path), false }); // folder actual no está en la BD
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
            DiskItem &fileInfoD = listD[j];
            QString nameS = QDir::cleanPath(fileInfoS.absoluteFilePath()).remove(QDir::cleanPath(fileInfoS.absolutePath())); // remove source
            QString nameD = "/" + std::visit(overloaded {
                                                     [](const Folder &folder) { return folder.name; },
                                                     [](const ComicDB &comic) { return comic.file.fileName; },
                                             },
                                             fileInfoD);
            const bool isFolderD = std::holds_alternative<Folder>(fileInfoD);

            int comparation = QString::localeAwareCompare(nameS, nameD);
            if (fileInfoS.isDir() && isFolderD)
                if (comparation == 0) // same folder, update
                {
                    _currentPathFolders.append({ std::get<Folder>(fileInfoD), true }); // fileInfoD conoce su padre y su id
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
                        _currentPathFolders.append({ newFolder(fileInfoS.fileName(), path), false });
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
                        _currentPathFolders.append({ newFolder(fileInfoS.fileName(), path), false });
                        create(QDir(fileInfoS.absoluteFilePath()));
                        _currentPathFolders.pop_back();
                    }
                    i++;
                } else if (isFolderD) // delete this folder from library
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
                            if (fileInfoS.isFile() && !isFolderD) {
                                auto comicDB = std::get_if<ComicDB>(&fileInfoD);
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
}
