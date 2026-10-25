#include "static.h"
#include "test_http_server.h"
#include "yacreader_global.h"
#include "yacreader_http_session_store.h"
#include "yacreader_libraries.h"

#include <QDir>
#include <QImage>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTest>

#include <memory>

class ServerV3RoutesTest : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void cleanupTestCase();
    void versionIsTheVersionOfV3();
    void versionAcceptsOnlyGet();
    void unknownRoutesAreNotFound();
    void v3UsesTheSessionOfV2();
    void folderContentHasTheOrderOfTheDesktop();
    void folderContentOfAMissingFolderIsNotFound();
    void readingListContentHasEveryEntryOfTheListAndItsChildLists();
    void labelContentHasEveryEntry();
    void contentOfAMissingListIsNotFound();
    void readingListsSectionHasTheOrderOfTheSidebar();
    void smartListsSectionHasTheOrderOfTheSidebar();
    void contentOfAMissingListFolderIsNotFound();
    void smartListContentHasTheSearchOrder();
    void coverServesComicFolderAndPlaceholderCovers();
    void tagsHaveTheOrderOfTheSidebar();
    void coversOutsideTheCoversFolderAreNotFound();

private:
    bool exec(const QString &sql);
    void saveCover(const QString &path, const QSize &size);
    QJsonArray items(const QString &path);
    std::unique_ptr<QTemporaryDir> root;
    QString libraryPath;
    int libraryId = -1;
    std::unique_ptr<TestHttpServer> server;
};

static const QString libraryName = "Server v3 routes";

void ServerV3RoutesTest::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
    root = std::make_unique<QTemporaryDir>();
    QVERIFY(root->isValid());
    libraryPath = QDir::cleanPath(root->filePath("library"));
    libraryId = createTestLibrary(libraryName, libraryPath);
    QVERIFY(libraryId >= 0);

    QVERIFY(exec("INSERT INTO folder (id, parentId, name, path) VALUES (2, 1, 'Series', '/Series'), "
                 "(3, 2, 'A sub', '/Series/A sub'), (4, 2, 'b sub', '/Series/b sub'), (5, 2, 'a sub', '/Series/a sub')"));
    QVERIFY(exec("INSERT INTO comic_info (id, hash, number, title, series, numPages) VALUES "
                 "(1, 'hash-1', '10', 'Ten', 'Beta', 20), (2, 'hash-2', '2', 'Two', 'Beta', 20), (3, 'hash-3', NULL, NULL, 'Alpha', 20), (4, 'hash-4', '1', 'Deleted', NULL, 20)"));
    QVERIFY(exec("INSERT INTO comic (id, parentId, comicInfoId, fileName, path) VALUES "
                 "(1, 2, 1, 'c1.cbz', '/Series/c1.cbz'), (2, 2, 2, 'c2.cbz', '/Series/c2.cbz'), (3, 2, 3, 'c3.cbz', '/Series/c3.cbz')"));
    QVERIFY(exec("INSERT INTO placeholder (id, series, number, coverKey) VALUES (1, 'Series', '3', 'key-1'), (2, 'Series', NULL, NULL)"));
    QVERIFY(exec("INSERT INTO folder_placeholder (folder_id, placeholder_id) VALUES (2, 1), (2, 2)"));
    // child lists in their manual order: 3, then 2
    QVERIFY(exec("INSERT INTO reading_list (id, parentId, ordering, name) VALUES (1, NULL, 0, 'List'), (2, 1, 1, 'Second child'), (3, 1, 0, 'First child')"));
    QVERIFY(exec("INSERT INTO reading_list_entry (id, reading_list_id, ordering, comic_info_id, placeholder_id, last_known_name, last_known_path) VALUES "
                 "(1, 1, 0, 2, NULL, NULL, NULL), (2, 1, 1, NULL, 1, NULL, NULL), (3, 1, 2, 4, NULL, 'c4.cbz', '/Series/c4.cbz'), (4, 1, 3, 1, NULL, NULL, NULL), "
                 "(5, 1, 4, 2, NULL, NULL, NULL), (6, 2, 0, NULL, 2, NULL, NULL), (7, 3, 0, 3, NULL, NULL, NULL)"));
    QVERIFY(exec("INSERT INTO reading_list_folder (id, parent_id, name, ordering) VALUES (1, NULL, 'Marvel', 0), (2, NULL, 'dc', 1), (3, 1, 'Events', 0)"));
    // equal names without case: the last one loaded comes first, as in the sidebar
    QVERIFY(exec("INSERT INTO reading_list (id, parentId, ordering, name, folder_id) VALUES (4, NULL, 0, 'Zeta', NULL), (5, NULL, 0, 'alpha', 1), (6, NULL, 0, 'batman', NULL), (7, NULL, 0, 'Batman', NULL)"));
    QVERIFY(exec("INSERT INTO reading_list_entry (id, reading_list_id, ordering, comic_info_id) VALUES (8, 5, 0, 4)"));
    QVERIFY(exec("INSERT INTO smart_list_folder (id, parent_id, name, ordering) VALUES (1, NULL, 'Saved', 0)"));
    QVERIFY(exec("INSERT INTO smart_list (id, name, query, ordering, folder_id) VALUES (1, 'Unread', 'read:false', 0, NULL), (2, 'rated', 'rating>=4', 0, 1), (3, 'Broken', 'series:(batman', 0, NULL)"));
    QVERIFY(exec("INSERT INTO label (id, name, color, ordering) VALUES (1, 'Tag', 'red', 0), (2, 'Empty', 'blue', 1), (3, 'apple', 'red', 0)"));
    QVERIFY(exec("INSERT INTO label_entry (id, label_id, ordering, comic_info_id, placeholder_id) VALUES (1, 1, 0, 4, NULL), (2, 1, 1, NULL, 1), (3, 1, 2, 1, NULL)"));

    const auto dataPath = YACReader::LibraryPaths::libraryDataPath(libraryPath);
    saveCover(YACReader::LibraryPaths::coverPathFromLibraryDataPath(dataPath, "hash-2"), { 60, 80 });
    saveCover(YACReader::LibraryPaths::coverPathFromLibraryDataPath(dataPath, "hash-4"), { 64, 96 });
    saveCover(YACReader::LibraryPaths::customFolderCoverPathFromDataPath(dataPath, "3"), { 50, 50 });
    saveCover(YACReader::LibraryPaths::placeholderCoverPathFromDataPath(dataPath, "key-1"), { 30, 40 });

    server = std::make_unique<TestHttpServer>(root->path());
    QVERIFY(server->isListening());
}

void ServerV3RoutesTest::cleanupTestCase()
{
    server.reset();
    YACReaderLibraries libraries;
    libraries.load();
    libraries.remove(libraryName);
    libraries.save();
    root.reset();
}

void ServerV3RoutesTest::versionIsTheVersionOfV3()
{
    for (const auto &path : { QString("/v3/version"), QString("/v3/version/") }) {
        const auto answer = server->request("GET", path);
        QCOMPARE(answer.status, 200);
        QCOMPARE(answer.body, QByteArray("3.0"));
    }
    QCOMPARE(server->request("GET", "/v2/version").body, QByteArray("2.3"));
}

void ServerV3RoutesTest::versionAcceptsOnlyGet()
{
    for (const auto &method : { QByteArray("POST"), QByteArray("PATCH"), QByteArray("PUT"), QByteArray("DELETE") })
        QCOMPARE(server->request(method, "/v3/version").status, 405);
}

void ServerV3RoutesTest::unknownRoutesAreNotFound()
{
    const auto library = QString("/v3/library/%1").arg(libraryId);
    // the routes that stay in v2 are not in v3
    for (const auto &path : { QString("/v3"), QString("/v3/"), QString("/v3/versions"), QString("/v3/libraries"), library + "/favs", library + "/reading",
                              library + "/comic/1/fullinfo", QString("/v3/library/77/folder/1/content") }) {
        const auto answer = server->request("GET", path);
        QCOMPARE(answer.status, 404);
        QCOMPARE(answer.body, QByteArray("404 not found"));
    }
}

void ServerV3RoutesTest::v3UsesTheSessionOfV2()
{
    const QByteArray token = "v3-session-token";
    QVERIFY(Static::yacreaderSessionStore->getYACReaderSessionHttpSession(token) == nullptr);
    QCOMPARE(server->request("GET", "/v3/version", { }, { { "x-request-id", token } }).status, 200);
    const auto session = Static::yacreaderSessionStore->getYACReaderSessionHttpSession(token);
    QVERIFY(session != nullptr);

    QCOMPARE(server->request("GET", "/v2/version", { }, { { "x-request-id", token } }).status, 200);
    QCOMPARE(Static::yacreaderSessionStore->getYACReaderSessionHttpSession(token), session);
}

bool ServerV3RoutesTest::exec(const QString &sql)
{
    return execInLibrary(libraryPath, sql);
}

void ServerV3RoutesTest::saveCover(const QString &path, const QSize &size)
{
    QVERIFY(QDir().mkpath(QFileInfo(path).absolutePath()));
    QImage image(size, QImage::Format_RGB32);
    image.fill(Qt::darkCyan);
    QVERIFY(image.save(path, "JPG"));
}

QJsonArray ServerV3RoutesTest::items(const QString &path)
{
    const auto answer = server->request("GET", path);
    if (answer.status != 200 || answer.contentType != "application/json")
        qWarning() << path << answer.status << answer.contentType;
    return QJsonDocument::fromJson(answer.body).array();
}

static QStringList itemKeys(const QJsonArray &items)
{
    QStringList keys;
    for (const auto &item : items) {
        const auto object = item.toObject();
        const auto type = object.value("type").toString();
        const auto id = type == "placeholder" ? "placeholder_id" : (type == "deleted_comic" ? "comic_info_id" : "id");
        auto key = type + " " + object.value(id).toString();
        if (object.contains("entry_id"))
            key += " entry " + object.value("entry_id").toString();
        if (object.contains("reading_list_id"))
            key += " list " + object.value("reading_list_id").toString();
        keys.append(key);
    }
    return keys;
}

// subfolders by name, as the folders tree; then comics and placeholders by number, numbered first, then file name
void ServerV3RoutesTest::folderContentHasTheOrderOfTheDesktop()
{
    const auto library = QString("/v3/library/%1").arg(libraryId);
    const auto content = items(library + "/folder/2/content");
    QCOMPARE(itemKeys(content), QStringList({ "folder 3", "folder 5", "folder 4", "comic 2", "placeholder 1", "comic 1", "placeholder 2", "comic 3" }));

    const auto placeholder = content.at(4).toObject();
    QCOMPARE(placeholder.value("parent_id").toString(), QString("2"));
    QCOMPARE(placeholder.value("cover").toString(), QString("/placeholders/key-1.jpg"));
    QCOMPARE(placeholder.value("library_id").toString(), QString::number(libraryId));
    QVERIFY(!content.at(6).toObject().contains("cover"));
    QCOMPARE(content.at(3).toObject().value("file_name").toString(), QString("c2.cbz"));

    QCOMPARE(itemKeys(items(library + "/folder/1/content")), QStringList({ "folder 2" }));
    QCOMPARE(itemKeys(items(library + "/folder/4/content")), QStringList());
    QCOMPARE(server->request("POST", library + "/folder/2/content").status, 405);
}

void ServerV3RoutesTest::folderContentOfAMissingFolderIsNotFound()
{
    QCOMPARE(server->request("GET", QString("/v3/library/%1/folder/99/content").arg(libraryId)).status, 404);
    QCOMPARE(server->request("GET", "/v3/library/77/folder/2/content").status, 404);
}

// a repeated comic is one item for each entry; the child lists follow in their order, flat
void ServerV3RoutesTest::readingListContentHasEveryEntryOfTheListAndItsChildLists()
{
    const auto library = QString("/v3/library/%1").arg(libraryId);
    const auto content = items(library + "/reading_list/1/content");
    QCOMPARE(itemKeys(content), QStringList({ "comic 2 entry 1 list 1", "placeholder 1 entry 2 list 1", "deleted_comic 4 entry 3 list 1", "comic 1 entry 4 list 1", "comic 2 entry 5 list 1", "comic 3 entry 7 list 3", "placeholder 2 entry 6 list 2" }));
    const auto deleted = content.at(2).toObject();
    QCOMPARE(deleted.value("last_known_name").toString(), QString("c4.cbz"));
    QCOMPARE(deleted.value("hash").toString(), QString("hash-4"));
    QCOMPARE(deleted.value("title").toString(), QString("Deleted"));
    QVERIFY(!deleted.contains("id"));
    QCOMPARE(content.at(1).toObject().value("cover").toString(), QString("/placeholders/key-1.jpg"));

    QCOMPARE(itemKeys(items(library + "/reading_list/2/content")), QStringList({ "placeholder 2 entry 6 list 2" }));
    QCOMPARE(server->request("POST", library + "/reading_list/1/content").status, 405);
}

void ServerV3RoutesTest::labelContentHasEveryEntry()
{
    const auto library = QString("/v3/library/%1").arg(libraryId);
    QCOMPARE(itemKeys(items(library + "/tag/1/content")), QStringList({ "deleted_comic 4 entry 1", "placeholder 1 entry 2", "comic 1 entry 3" }));
    QCOMPARE(itemKeys(items(library + "/tag/2/content")), QStringList());
}

void ServerV3RoutesTest::contentOfAMissingListIsNotFound()
{
    const auto library = QString("/v3/library/%1").arg(libraryId);
    QCOMPARE(server->request("GET", library + "/reading_list/99/content").status, 404);
    QCOMPARE(server->request("GET", library + "/tag/99/content").status, 404);
    QCOMPARE(server->request("GET", "/v3/library/77/reading_list/1/content").status, 404);
    QCOMPARE(server->request("GET", library + "/smart_list/99/content").status, 404);
    QCOMPARE(server->request("GET", "/v3/library/77/smart_list/1/content").status, 404);
}

// the folders, then the top level lists, each group by name; child lists are only in the content of their parent
void ServerV3RoutesTest::readingListsSectionHasTheOrderOfTheSidebar()
{
    const auto library = QString("/v3/library/%1").arg(libraryId);
    const auto top = items(library + "/reading_lists");
    QCOMPARE(itemKeys(top), QStringList({ "reading_list_folder 2", "reading_list_folder 1", "reading_list 7", "reading_list 6", "reading_list 1", "reading_list 4" }));
    QCOMPARE(top.at(0).toObject().value("folder_name").toString(), QString("dc"));
    QVERIFY(!top.at(0).toObject().contains("parent_id"));
    const auto list = top.at(4).toObject();
    QCOMPARE(list.value("reading_list_name").toString(), QString("List"));
    QCOMPARE(list.value("first_comic_hash").toString(), QString("hash-2"));
    QCOMPARE(list.value("library_id").toString(), QString::number(libraryId));
    QVERIFY(!list.contains("folder_id"));
    QVERIFY(!top.at(5).toObject().contains("first_comic_hash"));
    QCOMPARE(items(library + "/reading_lists/"), top);

    const auto marvel = items(library + "/reading_list_folder/1/content");
    QCOMPARE(itemKeys(marvel), QStringList({ "reading_list_folder 3", "reading_list 5" }));
    QCOMPARE(marvel.at(0).toObject().value("parent_id").toString(), QString("1"));
    QCOMPARE(marvel.at(1).toObject().value("folder_id").toString(), QString("1"));
    QCOMPARE(marvel.at(1).toObject().value("first_comic_hash").toString(), QString("hash-4"));
    QCOMPARE(itemKeys(items(library + "/reading_list_folder/3/content")), QStringList());
    QCOMPARE(server->request("POST", library + "/reading_lists").status, 405);
}

void ServerV3RoutesTest::smartListsSectionHasTheOrderOfTheSidebar()
{
    const auto library = QString("/v3/library/%1").arg(libraryId);
    const auto top = items(library + "/smart_lists");
    QCOMPARE(itemKeys(top), QStringList({ "smart_list_folder 1", "smart_list 3", "smart_list 1" }));
    QCOMPARE(top.at(0).toObject().value("folder_name").toString(), QString("Saved"));
    QCOMPARE(top.at(2).toObject().value("smart_list_name").toString(), QString("Unread"));
    QVERIFY(!top.at(2).toObject().contains("folder_id"));

    const auto saved = items(library + "/smart_list_folder/1/content");
    QCOMPARE(itemKeys(saved), QStringList({ "smart_list 2" }));
    QCOMPARE(saved.at(0).toObject().value("folder_id").toString(), QString("1"));
    QCOMPARE(server->request("POST", library + "/smart_list_folder/1/content").status, 405);
}

void ServerV3RoutesTest::contentOfAMissingListFolderIsNotFound()
{
    const auto library = QString("/v3/library/%1").arg(libraryId);
    QCOMPARE(server->request("GET", library + "/reading_list_folder/99/content").status, 404);
    // the ids of the two kinds of folders are in two tables
    QCOMPARE(server->request("GET", library + "/smart_list_folder/3/content").status, 404);
    QCOMPARE(server->request("GET", "/v3/library/77/reading_lists").status, 404);
    QCOMPARE(server->request("GET", "/v3/library/77/smart_list_folder/1/content").status, 404);
}

// comics with a file only, by series, volume, number, and file name; the folder order has no series
void ServerV3RoutesTest::smartListContentHasTheSearchOrder()
{
    const auto library = QString("/v3/library/%1").arg(libraryId);
    const auto content = items(library + "/smart_list/1/content");
    QCOMPARE(itemKeys(content), QStringList({ "comic 3", "comic 2", "comic 1" }));
    QCOMPARE(content.at(1).toObject().value("file_name").toString(), QString("c2.cbz"));
    QCOMPARE(itemKeys(items(library + "/smart_list/2/content")), QStringList());
    // a query that cannot be parsed has no comics, as in the desktop
    const auto broken = server->request("GET", library + "/smart_list/3/content");
    QCOMPARE(broken.status, 200);
    QCOMPARE(broken.body, QByteArray("[]"));
    QCOMPARE(server->request("POST", library + "/smart_list/1/content").status, 405);
}

// by color, then by name in natural order without case; v2 sorts by name with SQL ("Tag" before "apple")
void ServerV3RoutesTest::tagsHaveTheOrderOfTheSidebar()
{
    const auto library = QString("/v3/library/%1").arg(libraryId);
    const auto tags = items(library + "/tags");
    QCOMPARE(itemKeys(tags), QStringList({ "label 3", "label 1", "label 2" }));
    QCOMPARE(tags.at(0).toObject().value("label_name").toString(), QString("apple"));
    QCOMPARE(tags.at(2).toObject().value("color_id").toInt(), 1);
    QCOMPARE(items(library + "/tags/"), tags);
    QCOMPARE(server->request("GET", "/v3/library/77/tags").status, 404);
    QCOMPARE(server->request("POST", library + "/tags").status, 405);
}

static QSize imageSize(const HttpAnswer &answer)
{
    return answer.status == 200 && answer.contentType == "image/jpeg" ? QImage::fromData(answer.body, "JPG").size() : QSize();
}

// the clients put the cover of a placeholder, as the custom_image of a folder, after "cover"
void ServerV3RoutesTest::coverServesComicFolderAndPlaceholderCovers()
{
    const auto library = QString("/v3/library/%1").arg(libraryId);
    QCOMPARE(imageSize(server->request("GET", library + "/cover/hash-2.jpg")), QSize(60, 80));
    QCOMPARE(imageSize(server->request("GET", library + "/cover/hash-4.jpg")), QSize(64, 96));
    QCOMPARE(imageSize(server->request("GET", library + "/cover/folders/3.jpg")), QSize(50, 50));
    const auto placeholder = items(library + "/folder/2/content").at(4).toObject();
    QCOMPARE(imageSize(server->request("GET", library + "/cover" + placeholder.value("cover").toString())), QSize(30, 40));

    const auto missing = server->request("GET", library + "/cover/hash-3.jpg");
    QCOMPARE(missing.status, 404);
    QCOMPARE(missing.body, QByteArray("404 not found"));
    QCOMPARE(server->request("GET", "/v3/library/77/cover/hash-2.jpg").status, 404);
    QCOMPARE(server->request("POST", library + "/cover/hash-2.jpg").status, 405);
}

void ServerV3RoutesTest::coversOutsideTheCoversFolderAreNotFound()
{
    const auto dataPath = YACReader::LibraryPaths::libraryDataPath(libraryPath);
    saveCover(QDir(dataPath).filePath("outside.jpg"), { 20, 20 });
    saveCover(QDir(libraryPath).filePath("outside.jpg"), { 20, 20 });
    const auto library = QString("/v3/library/%1").arg(libraryId);
    for (const auto &path : { QString("/cover/..%2Foutside.jpg"), QString("/cover/..%2F..%2Foutside.jpg"), QString("/cover/folders%2F..%2F..%2Foutside.jpg"),
                              QString("/cover/..%5Coutside.jpg"), QString("/cover/") + QDir(dataPath).filePath("outside.jpg") }) {
        const auto answer = server->request("GET", library + path);
        QCOMPARE(answer.status, 404);
        QCOMPARE(answer.body, QByteArray("404 not found"));
    }
}

QTEST_GUILESS_MAIN(ServerV3RoutesTest)

#include "main.moc"
