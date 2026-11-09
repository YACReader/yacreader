#include "cbl_import_model.h"

#include <QFont>
#include <QTest>

class CblImportModelTest : public QObject
{
    Q_OBJECT
private slots:
    void statusComesFromTheMatch();
    void comicColumnSaysWhatTheEntryRefersTo();
    void countsAndChoices();
    void importAsMissingChangesOnlyEntriesWithSomething();
    void useOriginalMatchUndoesTheChange();
    void bookTitle_data();
    void bookTitle();
    void filterByStatusAndText();
    void chooseChangesOnlyWhenTheChoiceIsNew();
    void problemsAreAmbiguousAndMissingEntries();
    void eachEntryHasOneDetailRow();

private:
    static CblLibraryComic comic(qulonglong id, const QString &fileName);
    static CblLibraryPlaceholder placeholder(qulonglong id);
    static CblImportEntry entry(const QString &series, const std::optional<CblCandidate> &accepted, const QList<CblCandidate> &candidates = { });
    static QList<CblImportEntry> sampleEntries();
};

CblLibraryComic CblImportModelTest::comic(qulonglong id, const QString &fileName)
{
    CblLibraryComic result;
    result.id = ComicInfoId(id);
    result.hasFile = !fileName.isEmpty();
    result.path = fileName.isEmpty() ? QString() : "/Folder/" + fileName;
    return result;
}

CblLibraryPlaceholder CblImportModelTest::placeholder(qulonglong id)
{
    CblLibraryPlaceholder result;
    result.id = PlaceholderId(id);
    return result;
}

CblImportEntry CblImportModelTest::entry(const QString &series, const std::optional<CblCandidate> &accepted, const QList<CblCandidate> &candidates)
{
    CblImportEntry result;
    result.book.series = series;
    result.book.number = "1";
    result.match.accepted = accepted;
    result.match.candidates = candidates;
    if (accepted && candidates.isEmpty())
        result.match.candidates = { *accepted };
    result.choice = accepted;
    return result;
}

// a matched comic, an ambiguous book, a missing book, a matched placeholder, a matched deleted comic
QList<CblImportEntry> CblImportModelTest::sampleEntries()
{
    return {
        entry("Nova", comic(1, "Nova 001.cbz")),
        entry("Drax", std::nullopt, { comic(2, "Drax 001.cbz"), comic(3, "Drax 001 (c2c).cbz") }),
        entry("Ronan", std::nullopt),
        entry("Quasar", placeholder(4)),
        entry("Annihilus", comic(5, { })),
    };
}

void CblImportModelTest::statusComesFromTheMatch()
{
    CblImportModel model;
    model.setEntries(sampleEntries());

    QCOMPARE(model.status(0), CblImportStatus::Matched);
    QCOMPARE(model.status(1), CblImportStatus::Ambiguous);
    QCOMPARE(model.status(2), CblImportStatus::Missing);
    QCOMPARE(model.status(3), CblImportStatus::Matched);
    QCOMPARE(model.status(4), CblImportStatus::Matched);
    QCOMPARE(model.index(1, CblImportModel::StatusColumn).data().toString(), QString("Ambiguous"));
}

void CblImportModelTest::comicColumnSaysWhatTheEntryRefersTo()
{
    CblImportModel model;
    model.setEntries(sampleEntries());

    const auto text = [&model](int row) { return model.index(row, CblImportModel::ComicColumn).data().toString(); };
    const auto italic = [&model](int row) { return model.index(row, CblImportModel::ComicColumn).data(Qt::FontRole).value<QFont>().italic(); };

    QCOMPARE(text(0), QString("Nova 001.cbz"));
    QVERIFY(!italic(0));
    // without a translation Qt shows the source text of the plural
    QVERIFY(text(1).startsWith("2 possible comic"));
    QVERIFY(italic(1));
    QCOMPARE(text(2), QString("Not in library"));
    QVERIFY(italic(2));
    QCOMPARE(text(3), QString("Already missing in your library"));
    QVERIFY(italic(3));
    QCOMPARE(text(4), QString("Already missing in your library"));
}

void CblImportModelTest::countsAndChoices()
{
    CblImportModel model;
    model.setEntries(sampleEntries());

    QCOMPARE(model.count(CblImportStatus::Matched), 3);
    QCOMPARE(model.count(CblImportStatus::Ambiguous), 1);
    QCOMPARE(model.count(CblImportStatus::Missing), 1);
    QCOMPARE(model.count(CblImportStatus::Changed), 0);
    QCOMPARE(model.withComicCount(), 1);

    const auto choices = model.choices();
    QCOMPARE(choices.size(), 5);
    QCOMPARE(choices.at(0), std::optional<CblCandidate>(comic(1, "Nova 001.cbz")));
    QVERIFY(!choices.at(1));
    QVERIFY(!choices.at(2));
    QCOMPARE(choices.at(3), std::optional<CblCandidate>(placeholder(4)));
}

void CblImportModelTest::importAsMissingChangesOnlyEntriesWithSomething()
{
    CblImportModel model;
    model.setEntries(sampleEntries());

    model.importAsMissing({ 0, 1, 2 });

    QCOMPARE(model.status(0), CblImportStatus::Changed);
    QCOMPARE(model.status(1), CblImportStatus::Changed);
    QCOMPARE(model.status(2), CblImportStatus::Missing);
    QCOMPARE(model.index(0, CblImportModel::ComicColumn).data().toString(), QString("Not in library"));
    QCOMPARE(model.index(1, CblImportModel::ComicColumn).data().toString(), QString("Not in library"));
    QVERIFY(!model.choices().at(0));
    QCOMPARE(model.count(CblImportStatus::Changed), 2);
    QCOMPARE(model.withComicCount(), 0);
}

void CblImportModelTest::useOriginalMatchUndoesTheChange()
{
    CblImportModel model;
    model.setEntries(sampleEntries());
    model.importAsMissing({ 0, 1 });

    model.useOriginalMatch({ 0, 1 });

    QCOMPARE(model.status(0), CblImportStatus::Matched);
    QCOMPARE(model.status(1), CblImportStatus::Ambiguous);
    QCOMPARE(model.choices().at(0), std::optional<CblCandidate>(comic(1, "Nova 001.cbz")));
}

void CblImportModelTest::bookTitle_data()
{
    QTest::addColumn<QString>("series");
    QTest::addColumn<QString>("number");
    QTest::addColumn<QString>("volume");
    QTest::addColumn<QString>("year");
    QTest::addColumn<QString>("expected");

    QTest::newRow("volume") << "Nova" << "8" << "2007" << "2008" << "Nova #8 (2007)";
    QTest::newRow("year without volume") << "Nova" << "8" << "" << "2008" << "Nova #8 (2008)";
    QTest::newRow("no number") << "Nova" << "" << "2007" << "" << "Nova (2007)";
    QTest::newRow("series only") << "Nova" << "" << "" << "" << "Nova";
    QTest::newRow("no series") << "" << "8" << "2007" << "" << "Nova 008.cbz";
}

void CblImportModelTest::bookTitle()
{
    QFETCH(QString, series);
    QFETCH(QString, number);
    QFETCH(QString, volume);
    QFETCH(QString, year);
    QFETCH(QString, expected);

    CblBook book;
    book.series = series;
    book.number = number;
    book.volume = volume;
    book.year = year;
    book.fileName = "Nova 008.cbz";
    QCOMPARE(CblImportModel::bookTitle(book), expected);
}

void CblImportModelTest::filterByStatusAndText()
{
    CblImportModel model;
    model.setEntries(sampleEntries());
    CblImportFilterModel filter;
    filter.setSourceModel(&model);

    filter.setStatusFilter(CblImportStatus::Matched);
    QCOMPARE(filter.rowCount(), 3);

    filter.setFilterFixedString("quasar");
    QCOMPARE(filter.rowCount(), 1);
    QCOMPARE(filter.index(0, CblImportModel::BookColumn).data().toString(), QString("Quasar #1"));

    filter.setStatusFilter(std::nullopt);
    filter.setFilterFixedString("c2c");
    QCOMPARE(filter.rowCount(), 0);
    filter.setFilterFixedString("possible");
    QCOMPARE(filter.rowCount(), 1);

    model.importAsMissing({ 0 });
    filter.setFilterFixedString({ });
    filter.setStatusFilter(CblImportStatus::Changed);
    QCOMPARE(filter.rowCount(), 1);
}

void CblImportModelTest::chooseChangesOnlyWhenTheChoiceIsNew()
{
    CblImportModel model;
    model.setEntries(sampleEntries());

    model.choose(1, comic(3, "Drax 001 (c2c).cbz"));
    QCOMPARE(model.status(1), CblImportStatus::Changed);
    QCOMPARE(model.index(1, CblImportModel::ComicColumn).data().toString(), QString("Drax 001 (c2c).cbz"));
    QCOMPARE(model.withComicCount(), 2);

    model.choose(0, comic(1, "Nova 001.cbz"));
    QCOMPARE(model.status(0), CblImportStatus::Matched);

    model.choose(0, placeholder(9));
    QCOMPARE(model.status(0), CblImportStatus::Changed);
    QCOMPARE(model.choices().at(0), std::optional<CblCandidate>(placeholder(9)));
}

void CblImportModelTest::problemsAreAmbiguousAndMissingEntries()
{
    CblImportModel model;
    model.setEntries(sampleEntries());

    QCOMPARE(model.problemRows(), QList<int>({ 1, 2 }));
    model.choose(1, comic(2, "Drax 001.cbz"));
    QCOMPARE(model.problemRows(), QList<int>({ 2 }));
}

void CblImportModelTest::eachEntryHasOneDetailRow()
{
    CblImportModel model;
    model.setEntries(sampleEntries());

    const auto entry = model.index(2, 0);
    QCOMPARE(model.rowCount(entry), 1);
    QCOMPARE(model.rowCount(model.index(2, CblImportModel::BookColumn)), 0);

    const auto detail = model.index(0, 0, entry);
    QVERIFY(CblImportModel::isDetail(detail));
    QVERIFY(!CblImportModel::isDetail(entry));
    QCOMPARE(model.parent(detail), entry);
    QCOMPARE(model.rowCount(detail), 0);
    QVERIFY(!model.index(1, 0, entry).isValid());
    QVERIFY(!(model.flags(detail) & Qt::ItemIsSelectable));
    QVERIFY(!detail.data().isValid());

    CblImportFilterModel filter;
    filter.setSourceModel(&model);
    filter.setStatusFilter(CblImportStatus::Missing);
    QCOMPARE(filter.rowCount(), 1);
    QCOMPARE(filter.rowCount(filter.index(0, 0)), 1);
}

QTEST_MAIN(CblImportModelTest)
#include "main.moc"
