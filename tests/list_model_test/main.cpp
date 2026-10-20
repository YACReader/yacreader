#include "data_base_management.h"
#include "reading_list_item.h"
#include "reading_list_model.h"
#include "yacreader_global.h"
#include "yacreader_global_gui.h"

#include <QApplication>
#include <QDataStream>
#include <QDir>
#include <QMimeData>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QTemporaryDir>
#include <QTest>

#include <memory>

using namespace YACReader;

class ListModelTest : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();
    void labelsAreSortedByColorAndName();
    void addedLabelHasItsColorAndId();
    void addedListsHaveTheirOrder();
    void renameChangesTheRowAndTheDatabase();
    void deleteRemovesTheSublist();
    void droppedComicsGiveComicInfoToListsAndComicsToFavorites();

private:
    std::unique_ptr<QTemporaryDir> root;
    QString dataPath;

    bool exec(const QString &sql);
    QString value(const QString &sql);
    QStringList names(ReadingListModel &model, const QModelIndex &parent = QModelIndex());
    QModelIndex find(ReadingListModel &model, const QString &name, const QModelIndex &parent = QModelIndex());
};

void ListModelTest::init()
{
    root = std::make_unique<QTemporaryDir>();
    QVERIFY(root->isValid());
    const auto library = QDir::cleanPath(root->filePath("library"));
    dataPath = LibraryPaths::libraryDataPath(library);
    QVERIFY(QDir().mkpath(dataPath));

    QString name;
    {
        auto db = DataBaseManagement::createDatabase(LibraryPaths::libraryDatabasePath(library));
        name = db.connectionName();
        db.close();
    }
    QSqlDatabase::removeDatabase(name);

    QVERIFY(exec("INSERT INTO label (id, name, color, ordering) VALUES (1, 'Zed', 'green', 4), (2, 'Alpha', 'red', 1), (3, 'Beta', 'green', 4)"));
    QVERIFY(exec("INSERT INTO reading_list (id, parentId, ordering, name) VALUES (1, NULL, 0, 'Events'), (2, 1, 0, 'First')"));
}

void ListModelTest::cleanup()
{
    root.reset();
}

bool ListModelTest::exec(const QString &sql)
{
    bool ok = false;
    QString name;
    {
        auto db = DataBaseManagement::loadDatabase(dataPath);
        name = db.connectionName();
        QSqlQuery query(db);
        ok = query.exec(sql);
        if (!ok)
            qWarning() << query.lastError().text() << sql;
    }
    QSqlDatabase::removeDatabase(name);
    return ok;
}

QString ListModelTest::value(const QString &sql)
{
    QString result;
    QString name;
    {
        auto db = DataBaseManagement::loadDatabase(dataPath);
        name = db.connectionName();
        QSqlQuery query(db);
        if (query.exec(sql) && query.next())
            result = query.value(0).toString();
    }
    QSqlDatabase::removeDatabase(name);
    return result;
}

QStringList ListModelTest::names(ReadingListModel &model, const QModelIndex &parent)
{
    QStringList result;
    for (int row = 0; row < model.rowCount(parent); row++)
        result.append(model.data(model.index(row, 0, parent), Qt::DisplayRole).toString());
    return result;
}

QModelIndex ListModelTest::find(ReadingListModel &model, const QString &name, const QModelIndex &parent)
{
    for (int row = 0; row < model.rowCount(parent); row++) {
        const auto index = model.index(row, 0, parent);
        if (model.data(index, Qt::DisplayRole).toString() == name)
            return index;
    }
    return { };
}

void ListModelTest::labelsAreSortedByColorAndName()
{
    ReadingListModel model;
    model.setupReadingListsData(dataPath);
    QStringList labels;
    for (const auto *label : model.getLabels())
        labels.append(label->name());
    QCOMPARE(labels, QStringList({ "Alpha", "Beta", "Zed" }));
}

void ListModelTest::addedLabelHasItsColorAndId()
{
    ReadingListModel model;
    model.setupReadingListsData(dataPath);
    model.addNewLabel("Cyan one", YACReader::YCyan);

    const auto index = find(model, "Cyan one");
    QVERIFY(index.isValid());
    QCOMPARE(model.data(index, ReadingListModel::TypeListsRole).toInt(), int(ReadingListModel::Label));
    QCOMPARE(model.data(index, ReadingListModel::LabelColorRole).toInt(), int(YACReader::YCyan));
    QCOMPARE(model.data(index, ReadingListModel::IDRole).toULongLong(), value("SELECT id FROM label WHERE name = 'Cyan one'").toULongLong());

    QStringList labels;
    for (const auto *label : model.getLabels())
        labels.append(label->name());
    QCOMPARE(labels, QStringList({ "Alpha", "Beta", "Zed", "Cyan one" }));
}

void ListModelTest::addedListsHaveTheirOrder()
{
    ReadingListModel model;
    model.setupReadingListsData(dataPath);

    model.addReadingList("Arcs");
    QVERIFY(find(model, "Arcs").isValid());
    QCOMPARE(value("SELECT parentId IS NULL FROM reading_list WHERE name = 'Arcs'"), QString("1"));

    const auto events = find(model, "Events");
    QVERIFY(events.isValid());
    model.addReadingListAt("Second", events);
    QCOMPARE(names(model, events), QStringList({ "First", "Second" }));
    QCOMPARE(value("SELECT ordering FROM reading_list WHERE name = 'Second'"), QString("1"));
    QCOMPARE(value("SELECT parentId FROM reading_list WHERE name = 'Second'"), QString("1"));
}

void ListModelTest::renameChangesTheRowAndTheDatabase()
{
    ReadingListModel model;
    model.setupReadingListsData(dataPath);

    model.rename(find(model, "Zed"), "Zeta");
    QVERIFY(find(model, "Zeta").isValid());
    QCOMPARE(value("SELECT name FROM label WHERE id = 1"), QString("Zeta"));

    const auto events = find(model, "Events");
    model.rename(find(model, "First", events), "Renamed");
    QCOMPARE(names(model, find(model, "Events")), QStringList({ "Renamed" }));
    QCOMPARE(value("SELECT name FROM reading_list WHERE id = 2"), QString("Renamed"));
}

void ListModelTest::deleteRemovesTheSublist()
{
    ReadingListModel model;
    model.setupReadingListsData(dataPath);

    const auto events = find(model, "Events");
    model.deleteItem(find(model, "First", events));
    QCOMPARE(model.rowCount(find(model, "Events")), 0);
    QCOMPARE(value("SELECT count(*) FROM reading_list WHERE id = 2"), QString("0"));
}

void ListModelTest::droppedComicsGiveComicInfoToListsAndComicsToFavorites()
{
    ReadingListModel model;
    model.setupReadingListsData(dataPath);

    QList<qulonglong> favorites;
    QList<QPair<QList<ComicRowId>, qulonglong>> labels;
    QList<QPair<QList<ComicRowId>, qulonglong>> lists;
    connect(&model, &ReadingListModel::addComicsToFavorites, this, [&](const QList<qulonglong> &comicIds) { favorites = comicIds; });
    connect(&model, &ReadingListModel::addComicsToLabel, this, [&](const QList<ComicRowId> &rows, qulonglong id) { labels.append({ rows, id }); });
    connect(&model, &ReadingListModel::addComicsToReadingList, this, [&](const QList<ComicRowId> &rows, qulonglong id) { lists.append({ rows, id }); });

    QMimeData data;
    QByteArray comicIds;
    QDataStream out(&comicIds, QIODevice::WriteOnly);
    out << QList<qulonglong> { 9, 9 };
    data.setData(YACReader::YACReaderLibrarComiscSelectionMimeDataFormat, comicIds);

    QVERIFY(model.dropMimeData(&data, Qt::CopyAction, -1, 0, find(model, "Alpha")));
    QVERIFY(model.dropMimeData(&data, Qt::CopyAction, -1, 0, find(model, "Events")));
    QVERIFY(model.dropMimeData(&data, Qt::CopyAction, -1, 0, model.index(0, 0)));

    QCOMPARE(labels.size(), 1);
    QVERIFY(labels.first().first == QList<ComicRowId>({ ComicId(9), ComicId(9) }));
    QCOMPARE(labels.first().second, 2ULL);
    QCOMPARE(lists.size(), 1);
    QCOMPARE(lists.first().second, 1ULL);

    // a drag from a list names its entries, so a placeholder or a deleted comic can be added too
    YACReader::setListEntriesMimeData(&data, { { ListKind::ReadingList, 7 }, { ListKind::Label, 3 } });
    QVERIFY(model.dropMimeData(&data, Qt::CopyAction, -1, 0, find(model, "Events")));
    QCOMPARE(lists.size(), 2);
    QVERIFY(lists.last().first == QList<ComicRowId>({ ListItemId { ListKind::ReadingList, 7 }, ListItemId { ListKind::Label, 3 } }));
    QCOMPARE(favorites, QList<qulonglong>({ 9, 9 }));
}

QTEST_MAIN(ListModelTest)

#include "main.moc"
