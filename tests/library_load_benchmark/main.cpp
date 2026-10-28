// Measures load time and memory of the comic loaders with libraries of growing size.
// Usage: library_load_benchmark [--sizes 10,100,1000,10000] [--runs 5] [--output results.md]

#include "comic_db.h"
#include "comic_item.h"
#include "comic_model.h"
#include "comic_query_result_processor.h"
#include "current_comic_view_helper.h"
#include "data_base_management.h"
#include "db_helper.h"
#include "descriptive_comic_metadata.h"
#include "search_query.h"
#include "yacreader_global.h"
#include "yacreader_libraries.h"
#include "yacreader_server_data_helper.h"

#include <QCommandLineParser>
#include <QDateTime>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QGuiApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QProcess>
#include <QSignalSpy>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTextStream>

#include <algorithm>
#include <functional>
#include <memory>

#ifdef Q_OS_WIN
// clang-format off
#include <windows.h>
#include <psapi.h>
// clang-format on
#elif defined(Q_OS_MACOS)
#include <mach/mach.h>
#else
#include <unistd.h>
#endif

using namespace YACReader;

namespace {

const QString searchTerm = "benchmark";

qint64 privateBytes()
{
#ifdef Q_OS_WIN
    PROCESS_MEMORY_COUNTERS_EX counters;
    GetProcessMemoryInfo(GetCurrentProcess(), reinterpret_cast<PROCESS_MEMORY_COUNTERS *>(&counters), sizeof(counters));
    return static_cast<qint64>(counters.PrivateUsage);
#elif defined(Q_OS_MACOS)
    task_vm_info_data_t info;
    mach_msg_type_number_t count = TASK_VM_INFO_COUNT;
    if (task_info(mach_task_self(), TASK_VM_INFO, reinterpret_cast<task_info_t>(&info), &count) != KERN_SUCCESS)
        return 0;
    return static_cast<qint64>(info.phys_footprint);
#else
    QFile statm("/proc/self/statm");
    if (!statm.open(QIODevice::ReadOnly))
        return 0;
    const auto fields = QString::fromLatin1(statm.readAll()).split(' ');
    return fields.size() > 5 ? fields.at(5).toLongLong() * sysconf(_SC_PAGESIZE) : 0;
#endif
}

QString text(int seed, int length)
{
    static const QString words = "lorem ipsum dolor sit amet consectetur adipiscing elit sed do eiusmod tempor incididunt ut labore et dolore magna aliqua ";
    QString result;
    result.reserve(length);
    int position = (seed * 7) % words.size();
    while (result.size() < length) {
        result.append(words.at(position));
        position = (position + 1) % words.size();
    }
    return result;
}

QString people(int seed, int count)
{
    QStringList names;
    for (int i = 0; i < count; i++)
        names.append(QString("Person %1 %2").arg(seed % 97).arg(i));
    return names.join('\n');
}

// Every descriptive column set, with lengths close to real ComicVine data.
QVariant descriptiveValue(const DescriptiveComicMetadata::Column &column, int seed)
{
    const QByteArray name(column.name);
    const QByteArray type(column.sqlType);
    if (type == "BOOLEAN")
        return seed % 2 == 0;
    if (type == "INTEGER")
        return 10 + seed % 40;
    if (name == "title")
        return QString("Benchmark Title %1").arg(seed);
    if (name == "number" || name == "arcNumber" || name == "alternateNumber")
        return QString::number(seed % 500 + 1);
    if (name == "date")
        return QString("%1/%2/%3").arg(seed % 28 + 1).arg(seed % 12 + 1).arg(1960 + seed % 60);
    if (name == "synopsis")
        return text(seed, 900);
    if (name == "notes" || name == "review")
        return text(seed + 3, 250);
    if (name == "characters")
        return people(seed, 12);
    if (name == "teams" || name == "locations")
        return people(seed + 1, 5);
    if (name == "writer" || name == "penciller" || name == "inker" || name == "colorist" || name == "letterer" || name == "coverArtist" || name == "editor")
        return people(seed + 2, 2);
    if (name == "tags")
        return QString("tag one\ntag two\ntag three");
    if (name == "comicVineID")
        return QString::number(100000 + seed);
    if (name == "languageISO")
        return QString("en");
    return text(seed + 5, 24);
}

bool exec(QSqlQuery &query)
{
    if (!query.exec()) {
        qCritical() << query.lastError().text() << query.lastQuery();
        return false;
    }
    return true;
}

bool createLibrary(const QString &libraryPath, int size)
{
    QDir().mkpath(LibraryPaths::libraryDataPath(libraryPath));
    const auto dbPath = LibraryPaths::libraryDatabasePath(libraryPath);
    bool success = true;
    QString connectionName;
    {
        auto db = DataBaseManagement::createDatabase(dbPath);
        connectionName = db.connectionName();
        db.transaction();

        QSqlQuery query(db);
        query.prepare("INSERT INTO folder (id, parentId, name, path, added, numChildren) VALUES (2, 1, 'Series', '/Series', :added, :size)");
        query.bindValue(":added", QDateTime::currentSecsSinceEpoch());
        query.bindValue(":size", size);
        success = success && exec(query);
        success = success && query.exec("INSERT INTO reading_list (id, name) VALUES (1, 'Benchmark list')");
        success = success && query.exec("INSERT INTO label (id, name, color, ordering) VALUES (1, 'Benchmark label', 'green', 4)");

        const auto &columns = DescriptiveComicMetadata::columns();
        QStringList names;
        QStringList placeholders;
        for (const auto &column : columns) {
            names.append(column.name);
            placeholders.append(QString(":") + column.name);
        }

        QSqlQuery info(db);
        info.prepare("INSERT INTO comic_info (id, hash, numPages, currentPage, read, hasBeenOpened, rating, coverPage, coverSizeRatio, originalCoverSize, added, lastTimeOpened, type, " +
                     names.join(", ") + ") VALUES (:id, :hash, :numPages, :currentPage, 0, 1, :rating, 1, 0.65, '1300x2000', :added, :lastTimeOpened, 0, " + placeholders.join(", ") + ")");
        QSqlQuery comic(db);
        comic.prepare("INSERT INTO comic (id, parentId, comicInfoId, fileName, path) VALUES (:id, 2, :id, :fileName, :path)");
        QSqlQuery inList(db);
        inList.prepare("INSERT INTO reading_list_entry (reading_list_id, ordering, comic_info_id) VALUES (1, :id, :id)");
        QSqlQuery inLabel(db);
        inLabel.prepare("INSERT INTO label_entry (label_id, ordering, comic_info_id) VALUES (1, :id, :id)");
        QSqlQuery inFavorites(db);
        inFavorites.prepare("INSERT INTO comic_default_reading_list (default_reading_list_id, comic_id, ordering) VALUES (1, :id, :id)");

        const auto now = QDateTime::currentSecsSinceEpoch();
        for (int i = 1; i <= size && success; i++) {
            info.bindValue(":id", i);
            info.bindValue(":hash", QString("%1%2").arg(i, 40, 16, QLatin1Char('0')).arg(25000000 + i));
            info.bindValue(":numPages", 24 + i % 30);
            info.bindValue(":currentPage", 2 + i % 20);
            info.bindValue(":rating", i % 6);
            info.bindValue(":added", now - i);
            info.bindValue(":lastTimeOpened", now - i);
            for (const auto &column : columns)
                info.bindValue(QString(":") + column.name, descriptiveValue(column, i));
            success = success && exec(info);

            const auto fileName = QString("Benchmark Series %1 (2024).cbz").arg(i, 5, 10, QLatin1Char('0'));
            comic.bindValue(":id", i);
            comic.bindValue(":fileName", fileName);
            comic.bindValue(":path", "/Series/" + fileName);
            success = success && exec(comic);

            for (auto *list : { &inList, &inLabel, &inFavorites }) {
                list->bindValue(":id", i);
                success = success && exec(*list);
            }
        }

        success = success && db.commit();
        db.close();
    }
    QSqlDatabase::removeDatabase(connectionName);
    return success;
}

struct Loader {
    QString side;
    QString name;
    // Loads the rows and keeps them alive in `held` until it is reset. Returns the number of rows.
    std::function<int(const QString &libraryPath, qulonglong libraryId, std::shared_ptr<void> &held, qint64 &jsonBytes)> load;
};

int heldModel(std::shared_ptr<void> &held, const std::function<void(ComicModel &)> &setup)
{
    auto model = std::make_shared<ComicModel>();
    setup(*model);
    const auto rows = model->rowCount();
    held = model;
    return rows;
}

int heldJson(qulonglong libraryId, const QList<ComicDB> &comics, std::shared_ptr<void> &held, qint64 &jsonBytes)
{
    auto data = std::make_shared<std::pair<QList<ComicDB>, QJsonArray>>();
    data->first = comics;
    for (const auto &comic : comics)
        data->second.append(YACReaderServerDataHelper::comicToJSON(libraryId, QUuid(), comic));
    jsonBytes = QJsonDocument(data->second).toJson(QJsonDocument::Compact).size();
    held = data;
    return comics.size();
}

QList<Loader> loaders()
{
    const auto dataPath = [](const QString &libraryPath) { return LibraryPaths::libraryDataPath(libraryPath); };
    return {
        { "desktop", "folder", [=](const QString &path, qulonglong, std::shared_ptr<void> &held, qint64 &) { return heldModel(held, [&](ComicModel &m) { m.setupFolderModelData(2, dataPath(path)); }); } },
        { "desktop", "folder + continue reading", [=](const QString &path, qulonglong, std::shared_ptr<void> &held, qint64 &) { return heldModel(held, [&](ComicModel &m) { m.setupFolderModelData(2, dataPath(path)); bool found; currentComicFromModel(&m, found); }); } },
        { "desktop", "reading list", [=](const QString &path, qulonglong, std::shared_ptr<void> &held, qint64 &) { return heldModel(held, [&](ComicModel &m) { m.setupReadingListModelData(1, dataPath(path)); }); } },
        { "desktop", "label", [=](const QString &path, qulonglong, std::shared_ptr<void> &held, qint64 &) { return heldModel(held, [&](ComicModel &m) { m.setupLabelModelData(1, dataPath(path)); }); } },
        { "desktop", "favorites", [=](const QString &path, qulonglong, std::shared_ptr<void> &held, qint64 &) { return heldModel(held, [&](ComicModel &m) { m.setupFavoritesModelData(dataPath(path)); }); } },
        { "desktop", "reading", [=](const QString &path, qulonglong, std::shared_ptr<void> &held, qint64 &) { return heldModel(held, [&](ComicModel &m) { m.setupReadingModelData(dataPath(path)); }); } },
        { "desktop", "search", [=](const QString &path, qulonglong, std::shared_ptr<void> &held, qint64 &) {
             auto model = std::make_shared<ComicModel>();
             ComicQueryResultProcessor processor;
             QObject::connect(&processor, &ComicQueryResultProcessor::newData, model.get(), &ComicModel::setModelData);
             QSignalSpy spy(model.get(), &ComicModel::modelReset);
             processor.createModelData(searchTerm, dataPath(path));
             if (!spy.wait(600000))
                 return -1;
             const auto rows = model->rowCount();
             held = model;
             return rows;
         } },
        { "server", "folder content", [](const QString &, qulonglong libraryId, std::shared_ptr<void> &held, qint64 &jsonBytes) { return heldJson(libraryId, DBHelper::getFolderComicsFromLibrary(libraryId, 2), held, jsonBytes); } },
        { "server", "reading list", [](const QString &, qulonglong libraryId, std::shared_ptr<void> &held, qint64 &jsonBytes) { return heldJson(libraryId, DBHelper::getReadingListFullContent(libraryId, 1), held, jsonBytes); } },
        { "server", "tag", [](const QString &, qulonglong libraryId, std::shared_ptr<void> &held, qint64 &jsonBytes) { return heldJson(libraryId, DBHelper::getLabelComics(libraryId, 1), held, jsonBytes); } },
        { "server", "favorites", [](const QString &, qulonglong libraryId, std::shared_ptr<void> &held, qint64 &jsonBytes) { return heldJson(libraryId, DBHelper::getFavorites(libraryId), held, jsonBytes); } },
        { "server", "reading", [](const QString &, qulonglong libraryId, std::shared_ptr<void> &held, qint64 &jsonBytes) { return heldJson(libraryId, DBHelper::getReading(libraryId), held, jsonBytes); } },
        // the same steps as SearchController::getComics; keep them equal
        { "server", "search", [](const QString &path, qulonglong libraryId, std::shared_ptr<void> &held, qint64 &jsonBytes) {
             QList<ComicDB> comics;
             QString connectionName;
             {
                 auto db = DataBaseManagement::loadDatabase(LibraryPaths::libraryDataPath(path));
                 auto sqlQuery = comicsSearchQuery(db, searchTerm);
                 const auto indexes = ComicDB::columnIndexes(sqlQuery.record(), "c", "ci");
                 while (sqlQuery.next())
                     comics.append(ComicDB::fromQuery(sqlQuery, indexes));
                 connectionName = db.connectionName();
             }
             QSqlDatabase::removeDatabase(connectionName);
             return heldJson(libraryId, comics, held, jsonBytes);
         } },
    };
}

struct Measurement {
    int rows = 0;
    double milliseconds = 0;
    qint64 bytes = 0;
    qint64 jsonBytes = 0;
};

double medianTime(const Loader &loader, const QString &libraryPath, qulonglong libraryId, int runs, int &rows, qint64 &jsonBytes)
{
    QList<double> times;
    for (int i = 0; i < runs; i++) {
        std::shared_ptr<void> held;
        QElapsedTimer timer;
        timer.start();
        rows = loader.load(libraryPath, libraryId, held, jsonBytes);
        times.append(timer.nsecsElapsed() / 1e6);
    }
    std::sort(times.begin(), times.end());
    return times.at(times.size() / 2);
}

// Runs in a fresh process, so freed memory of other loads is not reused.
int measureMemory(const QString &loaderName, const QString &libraryPath, const QString &warmupPath, qulonglong libraryId, qulonglong warmupId)
{
    const auto all = loaders();
    const auto it = std::find_if(all.begin(), all.end(), [&](const Loader &l) { return l.side + "/" + l.name == loaderName; });
    if (it == all.end())
        return 1;

    qint64 jsonBytes = 0;
    {
        std::shared_ptr<void> warmup;
        it->load(warmupPath, warmupId, warmup, jsonBytes);
    }

    std::shared_ptr<void> held;
    const auto before = privateBytes();
    it->load(libraryPath, libraryId, held, jsonBytes);
    const auto after = privateBytes();

    QTextStream(stdout) << "memory " << (after - before) << "\n";
    return 0;
}

}

int main(int argc, char *argv[])
{
    qputenv("QT_QPA_PLATFORM", "offscreen");
    QStandardPaths::setTestModeEnabled(true);
    QGuiApplication app(argc, argv);
    QCoreApplication::setApplicationName("library_load_benchmark");

    QCommandLineParser parser;
    parser.addOptions({
            { "sizes", "Library sizes.", "list", "10,100,1000,10000" },
            { "runs", "Timed runs per loader.", "count", "5" },
            { "output", "Markdown file for the results.", "file" },
            { "measure-memory", "Internal.", "loader" },
            { "library", "Internal.", "path" },
            { "warmup", "Internal.", "path" },
            { "library-id", "Internal.", "id" },
            { "warmup-id", "Internal.", "id" },
    });
    parser.process(app);

    if (parser.isSet("measure-memory")) {
        return measureMemory(parser.value("measure-memory"), parser.value("library"), parser.value("warmup"),
                             parser.value("library-id").toULongLong(), parser.value("warmup-id").toULongLong());
    }

    QTemporaryDir root;
    if (!root.isValid())
        return 1;

    YACReaderLibraries libraries;
    libraries.load();

    const auto addLibrary = [&](const QString &name, int size) -> std::pair<QString, qulonglong> {
        const auto path = QDir::cleanPath(root.filePath(name));
        if (!createLibrary(path, size)) {
            qCritical() << "could not create" << path;
            return { path, 0 };
        }
        if (libraries.contains(name))
            libraries.remove(name);
        libraries.addLibrary(name, path);
        libraries.save();
        return { path, static_cast<qulonglong>(libraries.getId(name)) };
    };

    const auto [warmupPath, warmupId] = addLibrary("warmup", 10);

    QString results;
    QTextStream out(&results);
    out << "| Side | Loader | Comics | Rows | Time (ms, median) | Memory (KB) | KB per row | JSON (KB) |\n";
    out << "|---|---|---:|---:|---:|---:|---:|---:|\n";

    const auto runs = std::max(1, parser.value("runs").toInt());
    for (const auto &sizeText : parser.value("sizes").split(',', Qt::SkipEmptyParts)) {
        const auto size = sizeText.toInt();
        const auto [path, libraryId] = addLibrary(QString("library_%1").arg(size), size);
        if (libraryId == 0)
            return 1;

        for (const auto &loader : loaders()) {
            Measurement m;
            m.milliseconds = medianTime(loader, path, libraryId, runs, m.rows, m.jsonBytes);

            QProcess child;
            child.start(QCoreApplication::applicationFilePath(),
                        { "--measure-memory", loader.side + "/" + loader.name, "--library", path, "--warmup", warmupPath,
                          "--library-id", QString::number(libraryId), "--warmup-id", QString::number(warmupId) });
            child.waitForFinished(-1);
            const auto output = QString::fromLatin1(child.readAllStandardOutput()).trimmed();
            if (output.startsWith("memory "))
                m.bytes = output.mid(7).toLongLong();

            out << "| " << loader.side << " | " << loader.name << " | " << size << " | " << m.rows << " | "
                << QString::number(m.milliseconds, 'f', 2) << " | " << m.bytes / 1024 << " | "
                << (m.rows > 0 ? QString::number(m.bytes / 1024.0 / m.rows, 'f', 2) : QString("-")) << " | "
                << (loader.side == "server" ? QString::number(m.jsonBytes / 1024) : QString("-")) << " |\n";
            out.flush();
            QTextStream(stdout) << loader.side << " " << loader.name << " " << size << " done\n";
        }
    }

    QTextStream(stdout) << "\n"
                        << results;
    if (parser.isSet("output")) {
        QFile file(parser.value("output"));
        if (file.open(QIODevice::WriteOnly | QIODevice::Truncate))
            file.write(results.toUtf8());
    }
    return 0;
}
