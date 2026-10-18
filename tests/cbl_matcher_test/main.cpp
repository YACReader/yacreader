#include "cbl_matcher.h"
#include "data_base_management.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QTest>

class CblMatcherTest : public QObject
{
    Q_OBJECT
private slots:
    void matchesByName_data();
    void matchesByName();
    void sameSeriesComesBeforeOtherLevels();
    void marksStayInOtherScripts();
    void leadingZerosDoNotCount();
    void numbersAreText_data();
    void numbersAreText();
    void reprintsAreOtherBooks();
    void volumeChoosesBetweenComics();
    void yearWithinOneAgrees_data();
    void yearWithinOneAgrees();
    void noConfirmationGoesToTheUser();
    void sameComicTwiceGoesToTheUser();
    void comicVineIdComesFirst();
    void comicVineIdNamesAndProviders();
    void comicVineIdOnSeveralComics();
    void comicWithAFileWinsOverADeletedOne();
    void aDeletedComicAloneIsAccepted();
    void bookTwiceMatchesTheSameComic();
    void bookWithoutSeriesOrIdsHasNoMatch();
    void jsonCoverDateGivesTheYear();
    void loadsLibraryComics();

private:
    static CblLibraryComic comic(qulonglong id, const QString &series, const QString &number, const QString &volume = { }, const QString &date = { }, bool hasFile = true);
    static CblBook book(const QString &series, const QString &number, const QString &volume = { }, const QString &year = { });
};

CblLibraryComic CblMatcherTest::comic(qulonglong id, const QString &series, const QString &number, const QString &volume, const QString &date, bool hasFile)
{
    CblLibraryComic result;
    result.id = ComicInfoId(id);
    result.hasFile = hasFile;
    result.series = series;
    result.number = number;
    result.volume = volume;
    result.date = date;
    return result;
}

CblBook CblMatcherTest::book(const QString &series, const QString &number, const QString &volume, const QString &year)
{
    CblBook result;
    result.series = series;
    result.number = number;
    result.volume = volume;
    result.year = year;
    return result;
}

void CblMatcherTest::matchesByName_data()
{
    QTest::addColumn<QString>("bookSeries");
    QTest::addColumn<QString>("comicSeries");
    QTest::addColumn<int>("level");

    QTest::newRow("same") << "Batman" << "Batman" << int(CblMatchLevel::SameSeries);
    QTest::newRow("case") << "The Amazing Spider-Man" << "the amazing spider-man" << int(CblMatchLevel::SameSeries);
    QTest::newRow("spaces") << "Green  Lantern " << "Green Lantern" << int(CblMatchLevel::SameSeries);
    QTest::newRow("volume in the book name") << "X-Men Vol. 2" << "X-Men" << int(CblMatchLevel::SeriesWithoutVolume);
    QTest::newRow("volume in the comic name") << "Thor" << "Thor v3" << int(CblMatchLevel::SeriesWithoutVolume);
    QTest::newRow("hyphen") << "Spider-Man" << "Spider Man" << int(CblMatchLevel::LettersAndDigits);
    QTest::newRow("ampersand") << "Batman & Robin" << "Batman and Robin" << int(CblMatchLevel::LettersAndDigits);
    QTest::newRow("leading article") << "The Flash" << "Flash" << int(CblMatchLevel::LettersAndDigits);
    QTest::newRow("accents") << QString::fromUtf8("Pokémon Adventures") << "Pokemon Adventures" << int(CblMatchLevel::LettersAndDigits);
    QTest::newRow("other script") << QString::fromUtf8("ドラゴンボール") << QString::fromUtf8("ドラゴン・ボール") << int(CblMatchLevel::LettersAndDigits);
    QTest::newRow("comic vine name") << "Batman" << "Batman (2016)" << int(CblMatchLevel::SeriesWithYear);
}

void CblMatcherTest::matchesByName()
{
    QFETCH(QString, bookSeries);
    QFETCH(QString, comicSeries);
    QFETCH(int, level);

    const CblMatcher matcher({ comic(1, comicSeries, "1", comicSeries.endsWith(")") ? QString() : "2016", "01/05/2016") });

    const auto match = matcher.match(book(bookSeries, "1", "2016", "2016"));

    QVERIFY(match.accepted);
    QCOMPARE(match.accepted->id, ComicInfoId(1));
    QCOMPARE(int(*match.level), level);
}

void CblMatcherTest::marksStayInOtherScripts()
{
    const CblMatcher matcher({ comic(1, QString::fromUtf8("ホール"), "1", "2000", "01/01/2000"), comic(2, QString::fromUtf8("Мой"), "1", "2000", "01/01/2000") });

    QVERIFY(matcher.match(book(QString::fromUtf8("ボール"), "1", "2000")).candidates.isEmpty());
    QVERIFY(matcher.match(book(QString::fromUtf8("Мои"), "1", "2000")).candidates.isEmpty());
    QCOMPARE(matcher.match(book(QString::fromUtf8("ホール"), "1", "2000")).accepted->id, ComicInfoId(1));
}

// ComicRack's order: a later level is not used when an earlier one finds a comic
void CblMatcherTest::sameSeriesComesBeforeOtherLevels()
{
    const CblMatcher matcher({ comic(1, "Batman", "1", "1940", "01/04/1940"), comic(2, "Batman (2016)", "1", { }, "01/06/2016") });

    const auto match = matcher.match(book("Batman", "1", "2016", "2016"));

    QCOMPARE(*match.level, CblMatchLevel::SameSeries);
    QCOMPARE(match.candidates.size(), 1);
    QCOMPARE(match.candidates.first().id, ComicInfoId(1));
    QVERIFY(!match.accepted);
}

void CblMatcherTest::leadingZerosDoNotCount()
{
    const CblMatcher matcher({ comic(1, "Batman", "001", "2016", "01/05/2016"), comic(2, "Hulk", "-01", "1999", "01/01/1999"), comic(3, "Hulk", "0", "1999", "01/01/1999") });

    QCOMPARE(matcher.match(book("Batman", "1", "2016")).accepted->id, ComicInfoId(1));
    QCOMPARE(matcher.match(book("Hulk", "-1", "1999")).accepted->id, ComicInfoId(2));
    QCOMPARE(matcher.match(book("Hulk", "000", "1999")).accepted->id, ComicInfoId(3));
}

void CblMatcherTest::numbersAreText_data()
{
    QTest::addColumn<QString>("bookNumber");
    QTest::addColumn<QString>("comicNumber");
    QTest::addColumn<bool>("matches");

    QTest::newRow("fraction sign") << QString::fromUtf8("½") << QString::fromUtf8("½") << true;
    QTest::newRow("fraction forms differ") << QString::fromUtf8("½") << "1/2" << false;
    QTest::newRow("decimal") << "23.2" << "23.2" << true;
    QTest::newRow("decimal with zeros") << "023.2" << "23.2" << true;
    QTest::newRow("text without case") << "1.MU" << "1.mu" << true;
    QTest::newRow("different text") << "1.MU" << "1.HU" << false;
    QTest::newRow("not the same number") << "12" << "1" << false;
}

void CblMatcherTest::numbersAreText()
{
    QFETCH(QString, bookNumber);
    QFETCH(QString, comicNumber);
    QFETCH(bool, matches);

    const CblMatcher matcher({ comic(1, "Hulk", comicNumber, "1999", "01/01/1999") });

    QCOMPARE(matcher.match(book("Hulk", bookNumber, "1999")).accepted.has_value(), matches);
}

void CblMatcherTest::reprintsAreOtherBooks()
{
    const CblMatcher matcher({ comic(1, "Watchmen Deluxe Edition", "1", "1986", "01/09/1986") });

    const auto match = matcher.match(book("Watchmen", "1", "1986", "1986"));

    QVERIFY(match.candidates.isEmpty());
    QVERIFY(!match.level);
}

void CblMatcherTest::volumeChoosesBetweenComics()
{
    const CblMatcher matcher({ comic(1, "Batman", "1", "1940", "01/04/1940"), comic(2, "Batman", "1", "2011", "01/11/2011") });

    const auto match = matcher.match(book("Batman", "1", "2011", "2011"));

    QCOMPARE(match.candidates.size(), 1);
    QCOMPARE(match.accepted->id, ComicInfoId(2));
}

void CblMatcherTest::yearWithinOneAgrees_data()
{
    QTest::addColumn<QString>("date");
    QTest::addColumn<bool>("accepted");

    QTest::newRow("same year") << "01/03/2012" << true;
    QTest::newRow("one year later") << "//2013" << true;
    QTest::newRow("one year earlier") << "01/12/2011" << true;
    QTest::newRow("two years later") << "01/01/2014" << false;
}

void CblMatcherTest::yearWithinOneAgrees()
{
    QFETCH(QString, date);
    QFETCH(bool, accepted);

    const CblMatcher matcher({ comic(1, "Saga", "1", { }, date) });

    const auto match = matcher.match(book("Saga", "1", "2012", "2012"));

    QCOMPARE(match.candidates.size(), 1);
    QCOMPARE(match.accepted.has_value(), accepted);
}

void CblMatcherTest::noConfirmationGoesToTheUser()
{
    // no volume or year on the comic: nothing confirms it
    QVERIFY(!CblMatcher({ comic(1, "Saga", "1") }).match(book("Saga", "1", "2012", "2012")).accepted);
    // the year agrees but the volume does not
    QVERIFY(!CblMatcher({ comic(1, "Saga", "1", "2020", "01/01/2012") }).match(book("Saga", "1", "2012", "2012")).accepted);
    // the book has no volume and year "0", which is unknown
    QVERIFY(!CblMatcher({ comic(1, "Saga", "1", "2012", "01/01/2012") }).match(book("Saga", "1", { }, "0")).accepted);
}

void CblMatcherTest::sameComicTwiceGoesToTheUser()
{
    const CblMatcher matcher({ comic(1, "Saga", "1", "2012", "01/03/2012"), comic(2, "Saga", "1", "2012", "01/03/2012") });

    const auto match = matcher.match(book("Saga", "1", "2012", "2012"));

    QCOMPARE(match.candidates.size(), 2);
    QVERIFY(!match.accepted);
}

void CblMatcherTest::comicVineIdComesFirst()
{
    auto byId = comic(1, "All-Star Batman and Robin", "1", "2005", "01/09/2005");
    byId.comicVineId = "228824";
    const CblMatcher matcher({ byId, comic(2, "All Star Batman & Robin, the Boy Wonder", "1", "2005", "01/09/2005") });

    auto request = book("All Star Batman & Robin, the Boy Wonder", "1", "2005", "2005");
    request.databases = { { "cv", "34837", "228824" } };
    const auto match = matcher.match(request);

    QCOMPARE(*match.level, CblMatchLevel::ComicVineId);
    QCOMPARE(match.accepted->id, ComicInfoId(1));
}

void CblMatcherTest::comicVineIdNamesAndProviders()
{
    auto byId = comic(1, "A", "1");
    byId.comicVineId = "99";
    const CblMatcher matcher({ byId });

    for (const auto &name : { "cv", "CV", "comicvine", "ComicVine" }) {
        auto request = book({ }, { });
        request.databases = { { name, "1", "99" } };
        QVERIFY2(matcher.match(request).accepted, name);
    }

    auto otherProvider = book({ }, { });
    otherProvider.databases = { { "metron", "1", "99" } };
    QVERIFY(!matcher.match(otherProvider).level);
}

void CblMatcherTest::comicVineIdOnSeveralComics()
{
    auto first = comic(1, "A", "1");
    first.comicVineId = "99";
    auto second = comic(2, "A", "1");
    second.comicVineId = "99";

    auto request = book("A", "1");
    request.databases = { { "cv", "1", "99" } };
    const auto match = CblMatcher({ first, second }).match(request);

    QCOMPARE(*match.level, CblMatchLevel::ComicVineId);
    QCOMPARE(match.candidates.size(), 2);
    QVERIFY(!match.accepted);
}

void CblMatcherTest::comicWithAFileWinsOverADeletedOne()
{
    const CblMatcher matcher({ comic(1, "Batman", "1", "2016", "01/06/2016", false), comic(2, "Batman", "1", "2016", "01/06/2016", true) });

    const auto match = matcher.match(book("Batman", "1", "2016", "2016"));

    QCOMPARE(match.candidates.size(), 1);
    QCOMPARE(match.accepted->id, ComicInfoId(2));
}

void CblMatcherTest::aDeletedComicAloneIsAccepted()
{
    const auto match = CblMatcher({ comic(1, "Batman", "1", "2016", "01/06/2016", false) }).match(book("Batman", "1", "2016", "2016"));

    QCOMPARE(match.accepted->id, ComicInfoId(1));
    QVERIFY(!match.accepted->hasFile);
}

void CblMatcherTest::bookTwiceMatchesTheSameComic()
{
    const CblMatcher matcher({ comic(1, "Batman", "1", "2016", "01/06/2016") });
    const auto request = book("Batman", "1", "2016", "2016");

    QCOMPARE(matcher.match(request).accepted->id, ComicInfoId(1));
    QCOMPARE(matcher.match(request).accepted->id, ComicInfoId(1));
}

void CblMatcherTest::bookWithoutSeriesOrIdsHasNoMatch()
{
    const CblMatcher matcher({ comic(1, { }, "1") });

    const auto match = matcher.match(book({ }, "1"));

    QVERIFY(match.candidates.isEmpty());
    QVERIFY(!match.level);
}

void CblMatcherTest::jsonCoverDateGivesTheYear()
{
    const CblMatcher matcher({ comic(1, "Heroes Reborn", "1", { }, "01/07/2021") });
    auto request = book("Heroes Reborn", "1");
    request.coverDate = "2021-07-01";

    QVERIFY(matcher.match(request).accepted);
}

void CblMatcherTest::loadsLibraryComics()
{
    auto db = QSqlDatabase::addDatabase("QSQLITE", "cbl_matcher_test");
    db.setDatabaseName(":memory:");
    QVERIFY(db.open());
    QVERIFY(DataBaseManagement::createTables(db));

    QSqlQuery query(db);
    QVERIFY(query.exec("INSERT INTO folder (id, parentId, name, path) VALUES (2, 1, 'S', '/S')"));
    QVERIFY(query.exec("INSERT INTO comic_info (id, hash, series, number, volume, date, format, comicVineID) VALUES "
                       "(1, 'h1', 'Batman', '1', '2016', '01/06/2016', 'Main Series', '1001'), "
                       "(2, 'h2', 'Batman', '2', '2016', NULL, NULL, NULL), "
                       "(3, 'h3', NULL, '7', NULL, NULL, NULL, '1003'), "
                       "(4, 'h4', NULL, '8', NULL, NULL, NULL, ''), "
                       "(5, 'h5', '', '9', NULL, NULL, NULL, NULL)"));
    QVERIFY(query.exec("INSERT INTO comic (id, parentId, comicInfoId, fileName, path) VALUES (10, 2, 1, 'a.cbz', '/S/a.cbz'), (11, 2, 3, 'b.cbz', '/S/b.cbz')"));

    const auto comics = CblMatcher::loadLibraryComics(db);
    QVERIFY(comics);

    auto expectedFirst = comic(1, "Batman", "1", "2016", "01/06/2016", true);
    expectedFirst.format = "Main Series";
    expectedFirst.comicVineId = "1001";
    auto expectedThird = comic(3, { }, "7", { }, { }, true);
    expectedThird.comicVineId = "1003";
    QCOMPARE(*comics, QList<CblLibraryComic>({ expectedFirst, comic(2, "Batman", "2", "2016", { }, false), expectedThird }));

    query = QSqlQuery();
    db.close();
    db = QSqlDatabase();
    QSqlDatabase::removeDatabase("cbl_matcher_test");
}

QTEST_GUILESS_MAIN(CblMatcherTest)
#include "main.moc"
