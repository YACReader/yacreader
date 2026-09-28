#include "qnaturalsorting.h"

#include <QObject>
#include <QStringList>
#include <QTest>
#include <QVariant>

#include <algorithm>

// naturalCompareFallback() is what runs when Qt has no ICU, so most of these tests
// call it directly. A machine with ICU would never reach it otherwise, and that is
// exactly the platform this code was written for.
class NaturalSortingTest : public QObject
{
    Q_OBJECT

private slots:
    void fallbackOrdersIssueNumbersByValue();
    void fallbackOrdersPaddedFileNames();
    void fallbackOrdersNumbersAcrossSeparators();
    void fallbackOrdersUnicodeDecimalDigits();
    void fallbackIgnoresLeadingZeros();
    void fallbackPutsNumbersBeforeText();
    void fallbackFoldsCase();
    void fallbackKeepsAFixedOrderForEquivalentStrings();
    void fallbackHandlesEmptyStrings();
    void fallbackIsATotalOrder();
    void comicNumbersSortByValue();
    void comicsWithoutANumberGoLast();
};

static QStringList sortedWithFallback(QStringList values)
{
    std::sort(values.begin(), values.end(), [](const QString &l, const QString &r) {
        return naturalCompareFallback(l, r, Qt::CaseInsensitive) < 0;
    });
    return values;
}

void NaturalSortingTest::fallbackOrdersIssueNumbersByValue()
{
    // The reported bug: a plain text compare gives 0, 1, 10, 100, 11, 2.
    const QStringList numbers = { "0", "1", "10", "100", "11", "2", "20", "3" };

    QCOMPARE(sortedWithFallback(numbers),
             QStringList({ "0", "1", "2", "3", "10", "11", "20", "100" }));
}

void NaturalSortingTest::fallbackOrdersPaddedFileNames()
{
    const QStringList names = {
        "Batman_-_Legends_of_the_Dark_Knight_100_(1997).cbz",
        "Batman_-_Legends_of_the_Dark_Knight_002_(1989).cbz",
        "Batman_-_Legends_of_the_Dark_Knight_010_(1990).cbz",
    };

    QCOMPARE(sortedWithFallback(names),
             QStringList({
                     "Batman_-_Legends_of_the_Dark_Knight_002_(1989).cbz",
                     "Batman_-_Legends_of_the_Dark_Knight_010_(1990).cbz",
                     "Batman_-_Legends_of_the_Dark_Knight_100_(1997).cbz",
             }));
}

void NaturalSortingTest::fallbackOrdersNumbersAcrossSeparators()
{
    for (const QString &separator : { QStringLiteral("."), QStringLiteral("-"), QStringLiteral("_"), QStringLiteral(" ") }) {
        QCOMPARE(sortedWithFallback({ "2" + separator + "10", "2" + separator + "1", "2" + separator + "0", "2" + separator + "9" }),
                 QStringList({ "2" + separator + "0", "2" + separator + "1", "2" + separator + "9", "2" + separator + "10" }));
    }

    QVERIFY(naturalCompareFallback("1.a", "1.b", Qt::CaseInsensitive) < 0);
    QVERIFY(naturalCompareFallback("1-a", "1-b", Qt::CaseInsensitive) < 0);
}

void NaturalSortingTest::fallbackOrdersUnicodeDecimalDigits()
{
    // Arabic-Indic and supplementary-plane mathematical digits have the same values
    // as their ASCII counterparts for numeric comparison.
    QVERIFY(naturalCompareFallback(QString::fromUtf8("issue \u0662"), QStringLiteral("issue 10"), Qt::CaseInsensitive) < 0);
    QVERIFY(naturalCompareFallback(QString::fromUcs4(U"issue \U0001D7D8"), QStringLiteral("issue 1"), Qt::CaseInsensitive) < 0);
    QVERIFY(naturalCompareFallback(QStringLiteral("issue 2"), QString::fromUtf8("issue \u0662"), Qt::CaseInsensitive) != 0);
}

void NaturalSortingTest::fallbackIgnoresLeadingZeros()
{
    QCOMPARE(naturalCompareFallback("007", "8", Qt::CaseInsensitive), -1);
    QCOMPARE(naturalCompareFallback("v02 c010", "v2 c9", Qt::CaseInsensitive), 1);
}

void NaturalSortingTest::fallbackPutsNumbersBeforeText()
{
    QVERIFY(naturalCompareFallback("2 covers", "two covers", Qt::CaseInsensitive) < 0);
    QVERIFY(naturalCompareFallback("Annual 1", "1", Qt::CaseInsensitive) > 0);
}

void NaturalSortingTest::fallbackFoldsCase()
{
    // The POSIX collator drops case folding as well, so the fallback does it itself.
    QCOMPARE(sortedWithFallback({ "banana", "Apple", "cherry", "Blueberry" }),
             QStringList({ "Apple", "banana", "Blueberry", "cherry" }));

    QVERIFY(naturalCompareFallback("abc", "ABC", Qt::CaseSensitive) != 0);
}

void NaturalSortingTest::fallbackKeepsAFixedOrderForEquivalentStrings()
{
    // Same value, different spelling. Any order will do, but it must not change.
    const int first = naturalCompareFallback("007", "7", Qt::CaseInsensitive);
    const int second = naturalCompareFallback("7", "007", Qt::CaseInsensitive);

    QVERIFY(first != 0);
    QCOMPARE(first, -second);
}

void NaturalSortingTest::fallbackHandlesEmptyStrings()
{
    QCOMPARE(naturalCompareFallback("", "", Qt::CaseInsensitive), 0);
    QVERIFY(naturalCompareFallback("", "1", Qt::CaseInsensitive) < 0);
    QVERIFY(naturalCompareFallback("1", "", Qt::CaseInsensitive) > 0);
}

void NaturalSortingTest::fallbackIsATotalOrder()
{
    // std::sort needs a strict weak ordering. Check the three rules on a set that
    // mixes every path: digits, text, case, padding and empty chunks.
    const QStringList values = {
        "", "0", "00", "1", "01", "10", "2", "a", "A", "a1", "a01", "a10", "a2",
        "1a", "#1", "1.5", "1.10", "v2c10", "v2c9", "1-2", "1_2",
        QString::fromUtf8("\u0662"), QString::fromUcs4(U"\U0001D7D8")
    };

    const auto less = [](const QString &l, const QString &r) {
        return naturalCompareFallback(l, r, Qt::CaseInsensitive) < 0;
    };

    for (const auto &a : values) {
        QVERIFY(!less(a, a));

        for (const auto &b : values) {
            QVERIFY(!(less(a, b) && less(b, a)));

            for (const auto &c : values) {
                if (less(a, b) && less(b, c)) {
                    QVERIFY2(less(a, c),
                             qPrintable(QString("transitivity broken for %1 < %2 < %3").arg(a, b, c)));
                }
            }
        }
    }
}

void NaturalSortingTest::comicNumbersSortByValue()
{
    // Goes through naturalCompare(), so it covers the collator on machines with ICU
    // and the fallback on machines without it. Both must agree here.
    QVERIFY(comicNumberLessThan(QVariant("2"), "b.cbz", QVariant("10"), "a.cbz"));
    QVERIFY(!comicNumberLessThan(QVariant("10"), "a.cbz", QVariant("2"), "b.cbz"));
    QVERIFY(comicNumberLessThan(QVariant("11"), "a.cbz", QVariant("100"), "b.cbz"));
}

void NaturalSortingTest::comicsWithoutANumberGoLast()
{
    QVERIFY(comicNumberLessThan(QVariant("1"), "a.cbz", QVariant(), "b.cbz"));
    QVERIFY(!comicNumberLessThan(QVariant(), "b.cbz", QVariant("1"), "a.cbz"));

    // No numbers at all, so the file name decides.
    QVERIFY(comicNumberLessThan(QVariant(), "chapter 2.cbz", QVariant(), "chapter 10.cbz"));
}

QTEST_GUILESS_MAIN(NaturalSortingTest)

#include "main.moc"
