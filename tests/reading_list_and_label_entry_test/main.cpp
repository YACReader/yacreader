#include "data_base_management.h"
#include "db_helper.h"
#include "descriptive_comic_metadata.h"
#include "placeholder.h"
#include "placeholder_cover.h"
#include "yacreader_global.h"

#include <QDir>
#include <QFile>
#include <QImage>
#include <QSqlError>
#include <QSqlQuery>
#include <QTemporaryDir>
#include <QTest>

#include <optional>
#include <tuple>

class ReadingListAndLabelEntryTest : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();
    void placeholderColumnsMatchSharedList();
    void acceptsValidStates_data();
    void acceptsValidStates();
    void rejectsInvalidStates_data();
    void rejectsInvalidStates();
    void rejectsInvalidUpdates_data();
    void rejectsInvalidUpdates();
    void rejectsNullParentAndOrdering_data();
    void rejectsNullParentAndOrdering();
    void sameTargetCanAppearTwice_data();
    void sameTargetCanAppearTwice();
    void foreignKeyActions_data();
    void foreignKeyActions();
    void entryIdsAreNotReused_data();
    void entryIdsAreNotReused();
    void placeholderIdsAreNotReused();
    void appendRepeatsOnlyInReadingLists_data();
    void appendRepeatsOnlyInReadingLists();
    void appendRowsAddsWhatEachRowRefersTo();
    void placeholderCoverAndMetadataUpdates();
    void appendIgnoresUnknownComics_data();
    void appendIgnoresUnknownComics();
    void appendUsesNextOrderingAfterRemoval_data();
    void appendUsesNextOrderingAfterRemoval();
    void appendRollsBackOnFailure_data();
    void appendRollsBackOnFailure();
    void removeOnlyAffectsOwnParent_data();
    void removeOnlyAffectsOwnParent();
    void reorderSetsOrdering_data();
    void reorderSetsOrdering();
    void reorderRejectsIncompleteLists_data();
    void reorderRejectsIncompleteLists();
    void entryOfTheOtherKindIsRejected_data();
    void entryOfTheOtherKindIsRejected();
    void comicDeleteRecordsLastKnownFile_data();
    void comicDeleteRecordsLastKnownFile();
    void folderDeleteRecordsLastKnownFile_data();
    void folderDeleteRecordsLastKnownFile();
    void moveEntriesToComicInfo_data();
    void moveEntriesToComicInfo();
    void removeUnusedComicInfo();
    void placeholderRoundTrip();
    void placeholderIsDeletedWithItsLastEntry_data();
    void placeholderIsDeletedWithItsLastEntry();
    void deletingAListDeletesItsUnusedPlaceholders_data();
    void deletingAListDeletesItsUnusedPlaceholders();
    void deletingAReadingListDeletesPlaceholdersOfChildLists();
    void removePlaceholderCovers();
    void loadRecord();

private:
    void addReadingListAndLabelRows();
    void fetchReadingListOrLabel();
    bool insertEntry(qlonglong parentId, const QVariant &comicInfoId, const QVariant &placeholderId, QString *error = nullptr);
    bool exec(const QString &sql);
    int scalar(const QString &sql);
    QList<int> ints(const QString &sql);
    bool append(qulonglong parentId, const QList<qulonglong> &comicInfoIds);
    std::optional<QStringList> remove(qulonglong parentId, const QList<qulonglong> &entryIds, std::optional<ListKind> kind = std::nullopt);
    std::optional<QStringList> removeParent(qulonglong parentId);
    bool reorder(qulonglong parentId, const QList<qulonglong> &entryIds, std::optional<ListKind> kind = std::nullopt);
    ListKind listKind() const;
    QList<ListItemId> entries(const QList<qulonglong> &entryIds, std::optional<ListKind> kind) const;
    QList<qulonglong> entryIds(qulonglong parentId);

    QSqlDatabase db;
    QString table;
    QString parentColumn;
    QString parentTable;
};

void ReadingListAndLabelEntryTest::init()
{
    db = QSqlDatabase::addDatabase("QSQLITE", "reading_list_and_label_entry_test");
    db.setDatabaseName(":memory:");
    QVERIFY(db.open());

    QVERIFY(exec("PRAGMA foreign_keys = ON"));
    QVERIFY(DataBaseManagement::createTables(db));

    QVERIFY(exec("INSERT INTO folder (id, parentId, name, path) VALUES (1, 1, 'root', '/')"));
    QVERIFY(exec("INSERT INTO comic_info (id, hash) VALUES (10, 'hash10'), (11, 'hash11'), (12, 'hash12')"));
    QVERIFY(exec("INSERT INTO comic (id, parentId, comicInfoId, fileName, path) VALUES "
                 "(100, 1, 10, 'a.cbz', '/a.cbz'), (101, 1, 10, 'a copy.cbz', '/a copy.cbz'), "
                 "(110, 1, 11, 'b.cbz', '/b.cbz'), (120, 1, 12, 'c.cbz', '/c.cbz')"));
    QVERIFY(exec("INSERT INTO placeholder (id, title) VALUES (1, 'Expected issue')"));
    QVERIFY(exec("INSERT INTO reading_list (id, name) VALUES (1, 'List A'), (2, 'List B')"));
    QVERIFY(exec("INSERT INTO label (id, name, color, ordering) VALUES (1, 'Tag A', 'red', 0), (2, 'Tag B', 'blue', 1)"));
}

void ReadingListAndLabelEntryTest::cleanup()
{
    db.close();
    db = QSqlDatabase();
    QSqlDatabase::removeDatabase("reading_list_and_label_entry_test");
}

void ReadingListAndLabelEntryTest::addReadingListAndLabelRows()
{
    QTest::addColumn<QString>("table");
    QTest::addColumn<QString>("parentColumn");
    QTest::addColumn<QString>("parentTable");
    QTest::newRow("reading list") << "reading_list_entry"
                                  << "reading_list_id"
                                  << "reading_list";
    QTest::newRow("label") << "label_entry"
                           << "label_id"
                           << "label";
}

void ReadingListAndLabelEntryTest::fetchReadingListOrLabel()
{
    QFETCH(QString, table);
    QFETCH(QString, parentColumn);
    QFETCH(QString, parentTable);
    this->table = table;
    this->parentColumn = parentColumn;
    this->parentTable = parentTable;
}

bool ReadingListAndLabelEntryTest::insertEntry(qlonglong parentId, const QVariant &comicInfoId, const QVariant &placeholderId, QString *error)
{
    QSqlQuery q(db);
    q.prepare(QString("INSERT INTO %1 (%2, ordering, comic_info_id, placeholder_id) "
                      "VALUES (:parent, (SELECT COALESCE(MAX(ordering), -1) + 1 FROM %1 WHERE %2 = :parent2), :comicInfo, :placeholder)")
                      .arg(table, parentColumn));
    q.bindValue(":parent", parentId);
    q.bindValue(":parent2", parentId);
    q.bindValue(":comicInfo", comicInfoId);
    q.bindValue(":placeholder", placeholderId);
    const bool success = q.exec();
    if (error) {
        *error = q.lastError().text();
    }
    return success;
}

bool ReadingListAndLabelEntryTest::exec(const QString &sql)
{
    QSqlQuery q(db);
    return q.exec(sql);
}

int ReadingListAndLabelEntryTest::scalar(const QString &sql)
{
    QSqlQuery q(db);
    if (!q.exec(sql) || !q.next()) {
        return -1;
    }
    return q.value(0).toInt();
}

void ReadingListAndLabelEntryTest::placeholderColumnsMatchSharedList()
{
    auto tableColumns = [this](const QString &tableName) {
        QMap<QString, QString> types;
        QSqlQuery q(db);
        q.exec("PRAGMA table_info(" + tableName + ")");
        while (q.next()) {
            types.insert(q.value(1).toString(), q.value(2).toString());
        }
        return types;
    };

    const auto placeholder = tableColumns("placeholder");
    const auto comicInfo = tableColumns("comic_info");

    QCOMPARE(int(placeholder.size()), int(DescriptiveComicMetadata::columns().size()) + 2);
    QCOMPARE(placeholder.value("id"), QString("INTEGER"));
    QCOMPARE(placeholder.value("coverKey"), QString("TEXT"));
    for (const auto &column : DescriptiveComicMetadata::columns()) {
        const auto name = QString::fromLatin1(column.name);
        QCOMPARE(placeholder.value(name), QString::fromLatin1(column.sqlType));
        QCOMPARE(placeholder.value(name), comicInfo.value(name));
    }
}

void ReadingListAndLabelEntryTest::acceptsValidStates_data()
{
    addReadingListAndLabelRows();
}

void ReadingListAndLabelEntryTest::acceptsValidStates()
{
    fetchReadingListOrLabel();

    QString error;
    QVERIFY2(insertEntry(1, 10, QVariant(), &error), qPrintable(error));
    QVERIFY2(insertEntry(1, QVariant(), 1, &error), qPrintable(error));
    QCOMPARE(scalar("SELECT COUNT(*) FROM " + table), 2);
}

void ReadingListAndLabelEntryTest::rejectsInvalidStates_data()
{
    QTest::addColumn<QString>("table");
    QTest::addColumn<QString>("parentColumn");
    QTest::addColumn<QString>("parentTable");
    QTest::addColumn<QVariant>("comicInfoId");
    QTest::addColumn<QVariant>("placeholderId");

    const QList<std::tuple<QString, QString, QString>> entryTables = {
        { "reading_list_entry", "reading_list_id", "reading_list" },
        { "label_entry", "label_id", "label" },
    };
    for (const auto &[table, parentColumn, parentTable] : entryTables) {
        auto row = [&](const char *name) -> QTestData & {
            return QTest::newRow(qPrintable(table + ": " + name)) << table << parentColumn << parentTable;
        };
        row("no target") << QVariant() << QVariant();
        row("metadata and placeholder") << QVariant(10) << QVariant(1);
        row("unknown metadata") << QVariant(999) << QVariant();
        row("unknown placeholder") << QVariant() << QVariant(999);
    }
}

void ReadingListAndLabelEntryTest::rejectsInvalidStates()
{
    fetchReadingListOrLabel();
    QFETCH(QVariant, comicInfoId);
    QFETCH(QVariant, placeholderId);

    QVERIFY(!insertEntry(1, comicInfoId, placeholderId));
    QVERIFY(!insertEntry(999, 10, QVariant()));
    QCOMPARE(scalar("SELECT COUNT(*) FROM " + table), 0);
}

void ReadingListAndLabelEntryTest::rejectsInvalidUpdates_data()
{
    addReadingListAndLabelRows();
}

void ReadingListAndLabelEntryTest::rejectsInvalidUpdates()
{
    fetchReadingListOrLabel();

    QVERIFY(insertEntry(1, 10, QVariant()));
    const auto id = scalar("SELECT id FROM " + table);

    QVERIFY(!exec(QString("UPDATE %1 SET placeholder_id = 1 WHERE id = %2").arg(table).arg(id)));
    QVERIFY(!exec(QString("UPDATE %1 SET comic_info_id = NULL WHERE id = %2").arg(table).arg(id)));
    QVERIFY(!exec(QString("UPDATE %1 SET comic_info_id = 999 WHERE id = %2").arg(table).arg(id)));
    QCOMPARE(scalar("SELECT comic_info_id FROM " + table), 10);

    QVERIFY(exec(QString("UPDATE %1 SET comic_info_id = 11 WHERE id = %2").arg(table).arg(id)));
    QVERIFY(exec(QString("UPDATE %1 SET comic_info_id = NULL, placeholder_id = 1 WHERE id = %2").arg(table).arg(id)));
    QCOMPARE(scalar("SELECT placeholder_id FROM " + table), 1);
}

void ReadingListAndLabelEntryTest::rejectsNullParentAndOrdering_data()
{
    addReadingListAndLabelRows();
}

void ReadingListAndLabelEntryTest::rejectsNullParentAndOrdering()
{
    fetchReadingListOrLabel();

    QVERIFY(!exec(QString("INSERT INTO %1 (%2, ordering, comic_info_id) VALUES (NULL, 0, 10)").arg(table, parentColumn)));
    QVERIFY(!exec(QString("INSERT INTO %1 (%2, ordering, comic_info_id) VALUES (1, NULL, 10)").arg(table, parentColumn)));
    QVERIFY(!exec(QString("INSERT INTO %1 (%2, comic_info_id) VALUES (1, 10)").arg(table, parentColumn)));
    QCOMPARE(scalar("SELECT COUNT(*) FROM " + table), 0);

    QVERIFY(insertEntry(1, 10, QVariant()));
    const auto id = scalar("SELECT id FROM " + table);
    QVERIFY(!exec(QString("UPDATE %1 SET %2 = NULL WHERE id = %3").arg(table, parentColumn).arg(id)));
    QVERIFY(!exec(QString("UPDATE %1 SET ordering = NULL WHERE id = %2").arg(table).arg(id)));
    QCOMPARE(scalar(QString("SELECT %1 FROM %2").arg(parentColumn, table)), 1);
    QCOMPARE(scalar("SELECT ordering FROM " + table), 0);
}

void ReadingListAndLabelEntryTest::sameTargetCanAppearTwice_data()
{
    addReadingListAndLabelRows();
}

void ReadingListAndLabelEntryTest::sameTargetCanAppearTwice()
{
    fetchReadingListOrLabel();

    QVERIFY(insertEntry(1, 10, QVariant()));
    QVERIFY(insertEntry(1, 10, QVariant()));
    QVERIFY(insertEntry(1, QVariant(), 1));
    QVERIFY(insertEntry(1, QVariant(), 1));
    QVERIFY(insertEntry(2, 10, QVariant()));
    QCOMPARE(scalar(QString("SELECT COUNT(*) FROM %1 WHERE %2 = 1").arg(table, parentColumn)), 4);
    QCOMPARE(scalar(QString("SELECT COUNT(DISTINCT ordering) FROM %1 WHERE %2 = 1").arg(table, parentColumn)), 4);
}

void ReadingListAndLabelEntryTest::foreignKeyActions_data()
{
    addReadingListAndLabelRows();
}

void ReadingListAndLabelEntryTest::foreignKeyActions()
{
    fetchReadingListOrLabel();

    QVERIFY(insertEntry(1, 11, QVariant()));
    QVERIFY(insertEntry(1, QVariant(), 1));
    QVERIFY(insertEntry(2, 10, QVariant()));

    QVERIFY(exec("DELETE FROM comic WHERE id = 110"));
    QCOMPARE(scalar(QString("SELECT COUNT(*) FROM %1 WHERE comic_info_id = 11").arg(table)), 1);

    QVERIFY(!exec("DELETE FROM comic_info WHERE id = 11"));
    QVERIFY(!exec("DELETE FROM placeholder WHERE id = 1"));

    QVERIFY(exec(QString("DELETE FROM %1 WHERE id = 1").arg(parentTable)));
    QCOMPARE(scalar(QString("SELECT COUNT(*) FROM %1 WHERE %2 = 1").arg(table, parentColumn)), 0);
    QCOMPARE(scalar("SELECT COUNT(*) FROM " + table), 1);
    QCOMPARE(scalar("SELECT COUNT(*) FROM comic_info WHERE id = 11"), 1);
    QCOMPARE(scalar("SELECT COUNT(*) FROM placeholder WHERE id = 1"), 1);

    QVERIFY(exec("DELETE FROM comic_info WHERE id = 11"));
    QVERIFY(exec("DELETE FROM placeholder WHERE id = 1"));
}

void ReadingListAndLabelEntryTest::entryIdsAreNotReused_data()
{
    addReadingListAndLabelRows();
}

void ReadingListAndLabelEntryTest::entryIdsAreNotReused()
{
    fetchReadingListOrLabel();

    QVERIFY(insertEntry(1, 10, QVariant()));
    QVERIFY(insertEntry(1, 11, QVariant()));
    const auto lastId = scalar("SELECT MAX(id) FROM " + table);

    QVERIFY(exec(QString("DELETE FROM %1 WHERE id = %2").arg(table).arg(lastId)));
    QVERIFY(insertEntry(1, 11, QVariant()));
    QVERIFY(scalar("SELECT MAX(id) FROM " + table) > lastId);
}

void ReadingListAndLabelEntryTest::placeholderIdsAreNotReused()
{
    QVERIFY(exec("INSERT INTO placeholder (title) VALUES ('Second')"));
    const auto lastId = scalar("SELECT MAX(id) FROM placeholder");

    QVERIFY(exec(QString("DELETE FROM placeholder WHERE id = %1").arg(lastId)));
    QVERIFY(exec("INSERT INTO placeholder (title) VALUES ('Third')"));
    QVERIFY(scalar("SELECT MAX(id) FROM placeholder") > lastId);
}

QList<int> ReadingListAndLabelEntryTest::ints(const QString &sql)
{
    QList<int> values;
    QSqlQuery q(db);
    q.exec(sql);
    while (q.next()) {
        values.append(q.value(0).toInt());
    }
    return values;
}

ListKind ReadingListAndLabelEntryTest::listKind() const
{
    return table == "reading_list_entry" ? ListKind::ReadingList : ListKind::Label;
}

QList<ListItemId> ReadingListAndLabelEntryTest::entries(const QList<qulonglong> &entryIds, std::optional<ListKind> kind) const
{
    QList<ListItemId> result;
    for (const auto id : entryIds) {
        result.append({ kind.value_or(listKind()), id });
    }
    return result;
}

bool ReadingListAndLabelEntryTest::append(qulonglong parentId, const QList<qulonglong> &comicInfoIds)
{
    QList<ComicInfoId> ids;
    for (const auto id : comicInfoIds) {
        ids.append(ComicInfoId(id));
    }
    return table == "reading_list_entry" ? DBHelper::appendComicsToReadingList(ReadingListId(parentId), ids, db)
                                         : DBHelper::appendComicsToLabel(LabelId(parentId), ids, db);
}

std::optional<QStringList> ReadingListAndLabelEntryTest::remove(qulonglong parentId, const QList<qulonglong> &entryIds, std::optional<ListKind> kind)
{
    return table == "reading_list_entry" ? DBHelper::removeReadingListEntries(ReadingListId(parentId), entries(entryIds, kind), db)
                                         : DBHelper::removeLabelEntries(LabelId(parentId), entries(entryIds, kind), db);
}

std::optional<QStringList> ReadingListAndLabelEntryTest::removeParent(qulonglong parentId)
{
    return table == "reading_list_entry" ? DBHelper::removeListFromDB(parentId, db) : DBHelper::removeLabelFromDB(parentId, db);
}

bool ReadingListAndLabelEntryTest::reorder(qulonglong parentId, const QList<qulonglong> &entryIds, std::optional<ListKind> kind)
{
    return table == "reading_list_entry" ? DBHelper::reorderReadingListEntries(ReadingListId(parentId), entries(entryIds, kind), db)
                                         : DBHelper::reorderLabelEntries(LabelId(parentId), entries(entryIds, kind), db);
}

QList<qulonglong> ReadingListAndLabelEntryTest::entryIds(qulonglong parentId)
{
    QList<qulonglong> ids;
    for (const auto id : ints(QString("SELECT id FROM %1 WHERE %2 = %3 ORDER BY ordering").arg(table, parentColumn).arg(parentId))) {
        ids.append(id);
    }
    return ids;
}

void ReadingListAndLabelEntryTest::appendRepeatsOnlyInReadingLists_data()
{
    QTest::addColumn<QString>("table");
    QTest::addColumn<QString>("parentColumn");
    QTest::addColumn<QString>("parentTable");
    QTest::addColumn<QList<int>>("comicInfoIds");
    QTest::newRow("reading list") << "reading_list_entry"
                                  << "reading_list_id"
                                  << "reading_list" << QList<int>({ 10, 10, 11, 11, 10 });
    QTest::newRow("label") << "label_entry"
                           << "label_id"
                           << "label" << QList<int>({ 10, 11 });
}

void ReadingListAndLabelEntryTest::appendRepeatsOnlyInReadingLists()
{
    fetchReadingListOrLabel();
    QFETCH(QList<int>, comicInfoIds);

    QVERIFY(append(1, { 10, 10, 11 }));
    QVERIFY(append(1, { 11, 10 }));
    QVERIFY(append(2, { 10 }));

    QList<int> orderings;
    for (int i = 0; i < comicInfoIds.size(); i++) {
        orderings.append(i);
    }
    QCOMPARE(ints(QString("SELECT comic_info_id FROM %1 WHERE %2 = 1 ORDER BY ordering").arg(table, parentColumn)), comicInfoIds);
    QCOMPARE(ints(QString("SELECT ordering FROM %1 WHERE %2 = 1 ORDER BY ordering").arg(table, parentColumn)), orderings);
    QCOMPARE(ints(QString("SELECT comic_info_id FROM %1 WHERE %2 = 2").arg(table, parentColumn)), QList<int>({ 10 }));
}

void ReadingListAndLabelEntryTest::appendIgnoresUnknownComics_data()
{
    addReadingListAndLabelRows();
}

void ReadingListAndLabelEntryTest::appendIgnoresUnknownComics()
{
    fetchReadingListOrLabel();

    QVERIFY(append(1, { 999, 11 }));
    QCOMPARE(ints("SELECT comic_info_id FROM " + table), QList<int>({ 11 }));
}

void ReadingListAndLabelEntryTest::appendUsesNextOrderingAfterRemoval_data()
{
    addReadingListAndLabelRows();
}

void ReadingListAndLabelEntryTest::appendUsesNextOrderingAfterRemoval()
{
    fetchReadingListOrLabel();

    QVERIFY(append(1, { 10, 11 }));
    QVERIFY(remove(1, { entryIds(1).first() }));
    QVERIFY(append(1, { 12, 10 }));

    QCOMPARE(ints(QString("SELECT comic_info_id FROM %1 WHERE %2 = 1 ORDER BY ordering").arg(table, parentColumn)), QList<int>({ 11, 12, 10 }));
    QCOMPARE(ints(QString("SELECT ordering FROM %1 WHERE %2 = 1 ORDER BY ordering").arg(table, parentColumn)), QList<int>({ 1, 2, 3 }));
}

void ReadingListAndLabelEntryTest::appendRollsBackOnFailure_data()
{
    addReadingListAndLabelRows();
}

void ReadingListAndLabelEntryTest::appendRollsBackOnFailure()
{
    fetchReadingListOrLabel();

    QVERIFY(exec(QString("CREATE TRIGGER fail_on_11 BEFORE INSERT ON %1 WHEN NEW.comic_info_id = 11 "
                         "BEGIN SELECT RAISE(ABORT, 'test failure'); END")
                         .arg(table)));

    QVERIFY(!append(1, { 10, 11 }));
    QCOMPARE(scalar("SELECT COUNT(*) FROM " + table), 0);
    QVERIFY(!append(999, { 10 }));
    QCOMPARE(scalar("SELECT COUNT(*) FROM " + table), 0);
}

void ReadingListAndLabelEntryTest::removeOnlyAffectsOwnParent_data()
{
    addReadingListAndLabelRows();
}

void ReadingListAndLabelEntryTest::removeOnlyAffectsOwnParent()
{
    fetchReadingListOrLabel();

    QVERIFY(append(1, { 10, 11 }));
    QVERIFY(append(2, { 10 }));
    const auto otherEntry = entryIds(2).first();

    QVERIFY(remove(1, { otherEntry, 999 }));
    QCOMPARE(scalar("SELECT COUNT(*) FROM " + table), 3);

    QVERIFY(remove(2, { otherEntry }));
    QCOMPARE(entryIds(2), QList<qulonglong>());
    QCOMPARE(entryIds(1).size(), 2);
}

void ReadingListAndLabelEntryTest::reorderSetsOrdering_data()
{
    addReadingListAndLabelRows();
}

void ReadingListAndLabelEntryTest::reorderSetsOrdering()
{
    fetchReadingListOrLabel();

    QVERIFY(append(1, { 10, 11, 12 }));
    QVERIFY(remove(1, { entryIds(1).at(1) }));
    QVERIFY(append(1, { 11 }));
    const auto ids = entryIds(1);

    const QList<qulonglong> newOrder = { ids.at(2), ids.at(0), ids.at(1) };
    QVERIFY(reorder(1, newOrder));
    QCOMPARE(entryIds(1), newOrder);
    QCOMPARE(ints(QString("SELECT ordering FROM %1 WHERE %2 = 1 ORDER BY ordering").arg(table, parentColumn)), QList<int>({ 0, 1, 2 }));
}

void ReadingListAndLabelEntryTest::reorderRejectsIncompleteLists_data()
{
    addReadingListAndLabelRows();
}

void ReadingListAndLabelEntryTest::reorderRejectsIncompleteLists()
{
    fetchReadingListOrLabel();

    QVERIFY(append(1, { 10, 11 }));
    QVERIFY(append(2, { 12 }));
    const auto ids = entryIds(1);
    const auto otherEntry = entryIds(2).first();
    const auto orderings = ints("SELECT ordering FROM " + table + " ORDER BY id");

    QVERIFY(!reorder(1, { ids.at(1) }));
    QVERIFY(!reorder(1, { ids.at(1), ids.at(1) }));
    QVERIFY(!reorder(1, { ids.at(1), otherEntry }));
    QVERIFY(!reorder(1, { ids.at(1), ids.at(0), otherEntry }));
    QCOMPARE(ints("SELECT ordering FROM " + table + " ORDER BY id"), orderings);
    QCOMPARE(entryIds(1), ids);
}

void ReadingListAndLabelEntryTest::entryOfTheOtherKindIsRejected_data()
{
    addReadingListAndLabelRows();
}

void ReadingListAndLabelEntryTest::entryOfTheOtherKindIsRejected()
{
    fetchReadingListOrLabel();

    QVERIFY(append(1, { 10, 11 }));
    const auto ids = entryIds(1);
    const auto otherKind = listKind() == ListKind::ReadingList ? ListKind::Label : ListKind::ReadingList;

    QVERIFY(!remove(1, { ids.first() }, otherKind));
    QVERIFY(!reorder(1, { ids.at(1), ids.at(0) }, otherKind));
    QCOMPARE(entryIds(1), ids);
}

void ReadingListAndLabelEntryTest::comicDeleteRecordsLastKnownFile_data()
{
    addReadingListAndLabelRows();
}

void ReadingListAndLabelEntryTest::comicDeleteRecordsLastKnownFile()
{
    fetchReadingListOrLabel();

    QVERIFY(append(1, { 10, 11 }));
    QVERIFY(insertEntry(1, QVariant(), 1));

    QVERIFY(exec("DELETE FROM comic WHERE id = 110"));

    QSqlQuery q(db);
    QVERIFY(q.exec(QString("SELECT comic_info_id, last_known_name, last_known_path FROM %1 ORDER BY ordering").arg(table)));
    QVERIFY(q.next());
    QCOMPARE(q.value(0).toInt(), 10);
    QVERIFY(q.value(1).isNull());
    QVERIFY(q.value(2).isNull());
    QVERIFY(q.next());
    QCOMPARE(q.value(0).toInt(), 11);
    QCOMPARE(q.value(1).toString(), QString("b.cbz"));
    QCOMPARE(q.value(2).toString(), QString("/b.cbz"));
    QVERIFY(q.next());
    QVERIFY(q.value(0).isNull());
    QVERIFY(q.value(1).isNull());
    QVERIFY(q.value(2).isNull());
}

void ReadingListAndLabelEntryTest::folderDeleteRecordsLastKnownFile_data()
{
    addReadingListAndLabelRows();
}

void ReadingListAndLabelEntryTest::folderDeleteRecordsLastKnownFile()
{
    fetchReadingListOrLabel();

    QVERIFY(exec("INSERT INTO folder (id, parentId, name, path) VALUES (2, 1, 'Batman', '/Batman')"));
    QVERIFY(exec("INSERT INTO comic_info (id, hash) VALUES (13, 'hash13')"));
    QVERIFY(exec("INSERT INTO comic (id, parentId, comicInfoId, fileName, path) VALUES (130, 2, 13, '1.cbz', '/Batman/1.cbz')"));
    QVERIFY(append(1, { 13 }));

    QVERIFY(exec("DELETE FROM folder WHERE id = 2"));

    QCOMPARE(scalar("SELECT COUNT(*) FROM comic WHERE id = 130"), 0);
    QSqlQuery q(db);
    QVERIFY(q.exec(QString("SELECT last_known_name, last_known_path FROM %1 WHERE comic_info_id = 13").arg(table)));
    QVERIFY(q.next());
    QCOMPARE(q.value(0).toString(), QString("1.cbz"));
    QCOMPARE(q.value(1).toString(), QString("/Batman/1.cbz"));
}

void ReadingListAndLabelEntryTest::moveEntriesToComicInfo_data()
{
    addReadingListAndLabelRows();
}

void ReadingListAndLabelEntryTest::moveEntriesToComicInfo()
{
    fetchReadingListOrLabel();

    QVERIFY(append(1, { 10, 11 }));
    QVERIFY(append(2, { 11 }));
    QVERIFY(insertEntry(1, QVariant(), 1));
    const auto ids = entryIds(1);
    const auto otherIds = entryIds(2);

    // comic 110 still uses comic_info 11
    QVERIFY(DBHelper::moveEntriesToComicInfo(ComicInfoId(11), ComicInfoId(12), db));
    QCOMPARE(scalar(QString("SELECT COUNT(*) FROM %1 WHERE comic_info_id = 11").arg(table)), 2);

    QVERIFY(exec("DELETE FROM comic WHERE id = 110"));
    QVERIFY(DBHelper::moveEntriesToComicInfo(ComicInfoId(11), ComicInfoId(12), db));

    QCOMPARE(scalar(QString("SELECT COUNT(*) FROM %1 WHERE comic_info_id = 11").arg(table)), 0);
    QCOMPARE(scalar(QString("SELECT COUNT(*) FROM %1 WHERE comic_info_id = 12").arg(table)), 2);
    QCOMPARE(scalar(QString("SELECT COUNT(*) FROM %1 WHERE comic_info_id = 10").arg(table)), 1);
    QCOMPARE(scalar(QString("SELECT COUNT(*) FROM %1 WHERE placeholder_id = 1").arg(table)), 1);
    QCOMPARE(entryIds(1), ids);
    QCOMPARE(entryIds(2), otherIds);
}

void ReadingListAndLabelEntryTest::removeUnusedComicInfo()
{
    QVERIFY(exec("INSERT INTO comic_info (id, hash) VALUES (13, 'hash13'), (14, 'hash14')"));
    table = "reading_list_entry";
    QVERIFY(append(1, { 11 }));
    table = "label_entry";
    QVERIFY(append(1, { 12 }));
    QVERIFY(exec("DELETE FROM comic WHERE id IN (110, 120)"));

    auto hashes = DBHelper::removeUnusedComicInfo(db);
    hashes.sort();

    QCOMPARE(hashes, QStringList({ "hash13", "hash14" }));
    QCOMPARE(ints("SELECT id FROM comic_info ORDER BY id"), QList<int>({ 10, 11, 12 }));
}

void ReadingListAndLabelEntryTest::placeholderRoundTrip()
{
    Placeholder placeholder;
    placeholder.id = PlaceholderId(1);
    for (const auto &column : DescriptiveComicMetadata::columns()) {
        const auto type = QByteArray(column.sqlType);
        placeholder.*column.member = type == "TEXT" ? QVariant(QString("value:%1").arg(column.name)) : QVariant(1);
    }
    placeholder.title = QVariant();
    placeholder.notes = QString("");
    placeholder.number = QString("-1.5AU");
    placeholder.date = QString("//2024");
    placeholder.color = false;
    placeholder.coverKey = QString("6b0f1f7c-0d1e-4c43-9f0e-4c3a5f0c2a11");

    const auto id = DBHelper::insertPlaceholder(placeholder, db);
    QVERIFY(id);
    QVERIFY(*id != PlaceholderId(1));

    const auto loaded = DBHelper::loadPlaceholder(*id, db);
    QVERIFY(loaded);
    QCOMPARE(loaded->id, *id);
    for (const auto &column : DescriptiveComicMetadata::columns()) {
        const auto expected = placeholder.*column.member;
        const auto actual = (*loaded).*column.member;
        QVERIFY2(actual.isNull() == expected.isNull(), column.name);
        if (QByteArray(column.sqlType) == "TEXT")
            QCOMPARE(actual.toString(), expected.toString());
        else
            QCOMPARE(actual.toInt(), expected.toInt());
    }
    QVERIFY(!loaded->notes.isNull());
    QVERIFY(loaded->coverKey == placeholder.coverKey);

    const auto withoutCover = DBHelper::loadPlaceholder(PlaceholderId(1), db);
    QVERIFY(withoutCover);
    QCOMPARE(withoutCover->title.toString(), QString("Expected issue"));
    QVERIFY(!withoutCover->coverKey);

    QVERIFY(!DBHelper::loadPlaceholder(PlaceholderId(999), db));
}

void ReadingListAndLabelEntryTest::placeholderIsDeletedWithItsLastEntry_data()
{
    addReadingListAndLabelRows();
}

void ReadingListAndLabelEntryTest::placeholderIsDeletedWithItsLastEntry()
{
    fetchReadingListOrLabel();
    const auto otherTable = table == "reading_list_entry" ? QString("label_entry") : QString("reading_list_entry");
    const auto otherParentColumn = table == "reading_list_entry" ? QString("label_id") : QString("reading_list_id");

    QVERIFY(exec("UPDATE placeholder SET coverKey = 'cover-1' WHERE id = 1"));
    QVERIFY(insertEntry(1, QVariant(), 1));
    QVERIFY(insertEntry(1, QVariant(), 1));
    QVERIFY(insertEntry(2, QVariant(), 1));
    QVERIFY(exec(QString("INSERT INTO %1 (%2, ordering, placeholder_id) VALUES (1, 0, 1)").arg(otherTable, otherParentColumn)));
    const auto ids = entryIds(1);

    QVERIFY(remove(1, { ids.first() }) == QStringList());
    QVERIFY(remove(1, { ids.at(1) }) == QStringList());
    QVERIFY(remove(2, entryIds(2)) == QStringList());
    QCOMPARE(scalar("SELECT COUNT(*) FROM placeholder WHERE id = 1"), 1);

    QVERIFY(exec("DELETE FROM " + otherTable));
    QVERIFY(insertEntry(2, QVariant(), 1));
    QVERIFY(remove(2, entryIds(2)) == QStringList({ "cover-1" }));
    QCOMPARE(scalar("SELECT COUNT(*) FROM placeholder WHERE id = 1"), 0);
}

void ReadingListAndLabelEntryTest::deletingAListDeletesItsUnusedPlaceholders_data()
{
    addReadingListAndLabelRows();
}

void ReadingListAndLabelEntryTest::deletingAListDeletesItsUnusedPlaceholders()
{
    fetchReadingListOrLabel();

    QVERIFY(exec("UPDATE placeholder SET coverKey = 'cover-1' WHERE id = 1"));
    QVERIFY(exec("INSERT INTO placeholder (id, title, coverKey) VALUES (2, 'Shared', 'cover-2'), (3, 'Without entries', 'cover-3'), (4, 'Without cover', NULL)"));
    QVERIFY(insertEntry(1, QVariant(), 1));
    QVERIFY(insertEntry(1, QVariant(), 2));
    QVERIFY(insertEntry(1, QVariant(), 4));
    QVERIFY(insertEntry(1, 10, QVariant()));
    QVERIFY(insertEntry(2, QVariant(), 2));

    QVERIFY(removeParent(1) == QStringList({ "cover-1" }));
    QCOMPARE(ints("SELECT id FROM placeholder ORDER BY id"), QList<int>({ 2, 3 }));
    QCOMPARE(scalar(QString("SELECT COUNT(*) FROM %1 WHERE id = 1").arg(parentTable)), 0);
    QCOMPARE(scalar("SELECT COUNT(*) FROM comic_info WHERE id = 10"), 1);
}

void ReadingListAndLabelEntryTest::deletingAReadingListDeletesPlaceholdersOfChildLists()
{
    table = "reading_list_entry";
    parentColumn = "reading_list_id";

    QVERIFY(exec("INSERT INTO reading_list (id, name, parentId) VALUES (3, 'Child', 1), (4, 'Grandchild', 3)"));
    QVERIFY(exec("INSERT INTO placeholder (id, title, coverKey) VALUES (2, 'In grandchild', 'cover-2')"));
    QVERIFY(insertEntry(3, QVariant(), 1));
    QVERIFY(insertEntry(4, QVariant(), 2));

    auto covers = DBHelper::removeListFromDB(1, db);
    QVERIFY(covers);
    QCOMPARE(*covers, QStringList({ "cover-2" }));
    QCOMPARE(scalar("SELECT COUNT(*) FROM placeholder"), 0);
    QCOMPARE(ints("SELECT id FROM reading_list ORDER BY id"), QList<int>({ 2 }));
}

void ReadingListAndLabelEntryTest::removePlaceholderCovers()
{
    QTemporaryDir dataPath;
    QVERIFY(dataPath.isValid());
    QVERIFY(QDir().mkpath(YACReader::LibraryPaths::libraryPlaceholdersCoverPathFromLibraryDataPath(dataPath.path())));

    const auto removed = YACReader::LibraryPaths::placeholderCoverPathFromDataPath(dataPath.path(), "cover-1");
    const auto kept = YACReader::LibraryPaths::placeholderCoverPathFromDataPath(dataPath.path(), "cover-2");
    QCOMPARE(QDir(dataPath.path()).relativeFilePath(removed), QString("covers/placeholders/cover-1.jpg"));
    for (const auto &path : { removed, kept }) {
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
    }

    DBHelper::removePlaceholderCovers(dataPath.path(), { "cover-1", "missing" });

    QVERIFY(!QFile::exists(removed));
    QVERIFY(QFile::exists(kept));
}

void ReadingListAndLabelEntryTest::loadRecord()
{
    const auto comic = [](qulonglong fileId, qulonglong infoId) {
        ComicDB comic;
        comic.file.id = ComicId(fileId);
        comic.info.id = ComicInfoId(infoId);
        return ComicRecord(ComicRecordCase::Comic { comic });
    };
    const auto deleted = [](qulonglong infoId) {
        ComicInfo info;
        info.id = ComicInfoId(infoId);
        return ComicRecord(ComicRecordCase::DeletedComic { info, std::nullopt });
    };

    QVERIFY(DBHelper::loadRecord(comic(100, 10), db));
    QVERIFY(DBHelper::loadRecord(comic(101, 10), db));
    QVERIFY(!DBHelper::loadRecord(comic(100, 11), db));
    QVERIFY(!DBHelper::loadRecord(comic(999, 10), db));
    QVERIFY(DBHelper::loadRecord(deleted(11), db));
    QVERIFY(!DBHelper::loadRecord(deleted(99), db));

    QVERIFY(exec("DELETE FROM comic WHERE id = 110"));
    QVERIFY(!DBHelper::loadRecord(comic(110, 11), db));
    QVERIFY(DBHelper::loadRecord(deleted(11), db));

    QVERIFY(exec("UPDATE comic_info SET title = 'Saved elsewhere' WHERE id = 10"));
    const auto reloaded = DBHelper::loadRecord(comic(101, 10), db);
    QVERIFY(reloaded);
    QCOMPARE(recordComic(*reloaded)->file.fileName, QString("a copy.cbz"));
    QCOMPARE(recordInfo(*reloaded)->title.toString(), QString("Saved elsewhere"));

    ComicInfo info;
    info.id = ComicInfoId(11);
    const auto reloadedDeleted = DBHelper::loadRecord(ComicRecordCase::DeletedComic { info, LastKnownFile { "b.cbz", "/b.cbz" } }, db);
    QVERIFY(reloadedDeleted);
    QVERIFY(!recordComic(*reloadedDeleted));
    QCOMPARE(recordInfo(*reloadedDeleted)->hash, QString("hash11"));
    QVERIFY(std::get<ComicRecordCase::DeletedComic>(*reloadedDeleted).lastKnownFile == (LastKnownFile { "b.cbz", "/b.cbz" }));
}

// in order: a comic gives its comic_info, an entry its comic_info or placeholder; rows that are gone are skipped
void ReadingListAndLabelEntryTest::appendRowsAddsWhatEachRowRefersTo()
{
    QVERIFY(exec("INSERT INTO reading_list_entry (id, reading_list_id, ordering, comic_info_id) VALUES (50, 2, 0, 11)"));
    QVERIFY(exec("INSERT INTO reading_list_entry (id, reading_list_id, ordering, placeholder_id) VALUES (51, 2, 1, 1)"));
    QVERIFY(exec("INSERT INTO label_entry (id, label_id, ordering, comic_info_id) VALUES (60, 2, 0, 12)"));

    QVERIFY(DBHelper::appendRowsToReadingList(ReadingListId(1),
                                              { ComicId(101), ListItemId { ListKind::ReadingList, 51 }, ListItemId { ListKind::Label, 60 }, ComicId(999), ListItemId { ListKind::ReadingList, 999 }, ListItemId { ListKind::ReadingList, 50 } },
                                              db));
    QCOMPARE(ints("SELECT COALESCE(comic_info_id, 0) FROM reading_list_entry WHERE reading_list_id = 1 ORDER BY ordering"), QList<int>({ 10, 0, 12, 11 }));
    QCOMPARE(ints("SELECT COALESCE(placeholder_id, 0) FROM reading_list_entry WHERE reading_list_id = 1 ORDER BY ordering"), QList<int>({ 0, 1, 0, 0 }));

    QVERIFY(DBHelper::appendRowsToLabel(LabelId(1), { ComicId(100), ComicId(101), ListItemId { ListKind::ReadingList, 51 }, ListItemId { ListKind::ReadingList, 51 } }, db));
    QCOMPARE(ints("SELECT COALESCE(comic_info_id, 0) FROM label_entry WHERE label_id = 1 ORDER BY ordering"), QList<int>({ 10, 0 }));
    QCOMPARE(ints("SELECT COALESCE(placeholder_id, 0) FROM label_entry WHERE label_id = 1 ORDER BY ordering"), QList<int>({ 0, 1 }));
}

void ReadingListAndLabelEntryTest::placeholderCoverAndMetadataUpdates()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    QImage image(10, 15, QImage::Format_RGB32);
    image.fill(Qt::red);

    // each image gets a new key and file
    const auto first = savePlaceholderCover(directory.path(), image);
    const auto second = savePlaceholderCover(directory.path(), image);
    QVERIFY(first && second && *first != *second);
    QVERIFY(QFile::exists(YACReader::LibraryPaths::placeholderCoverPathFromDataPath(directory.path(), *first)));
    QVERIFY(!savePlaceholderCover(directory.path(), QImage()));

    QVERIFY(DBHelper::updatePlaceholderCover(PlaceholderId(1), *first, db));
    QCOMPARE(DBHelper::loadPlaceholder(PlaceholderId(1), db)->coverKey, first);
    QVERIFY(DBHelper::updatePlaceholderCover(PlaceholderId(1), std::nullopt, db));
    QVERIFY(!DBHelper::loadPlaceholder(PlaceholderId(1), db)->coverKey);
    QVERIFY(!DBHelper::updatePlaceholderCover(PlaceholderId(99), *first, db));

    DescriptiveMetadataPatch patch;
    patch.set(&DescriptiveComicMetadata::series, QString("Patched"));
    patch.set(&DescriptiveComicMetadata::title, QVariant());
    QVERIFY(DBHelper::updatePlaceholderMetadata(PlaceholderId(1), patch, db));
    const auto placeholder = DBHelper::loadPlaceholder(PlaceholderId(1), db);
    QCOMPARE(placeholder->series.toString(), QString("Patched"));
    QVERIFY(placeholder->title.isNull());
}

QTEST_GUILESS_MAIN(ReadingListAndLabelEntryTest)
#include "main.moc"
