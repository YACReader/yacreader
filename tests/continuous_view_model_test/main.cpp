#include "continuous_view_model.h"

#include <QObject>
#include <QSize>
#include <QTest>

class ContinuousViewModelTest : public QObject
{
    Q_OBJECT

private slots:
    void usesCenterPageForNormalProgress();
    void doesNotReportLastPageWhenLessThanHalfIsVisible();
    void reportsLastPageWhenAtLeastHalfIsVisible();
};

void ContinuousViewModelTest::usesCenterPageForNormalProgress()
{
    ContinuousViewModel model;
    model.setNumPages(4);
    model.setViewportSize(100, 250);
    model.setPageNaturalSize(0, QSize(100, 100));
    model.setPageNaturalSize(1, QSize(100, 100));
    model.setPageNaturalSize(2, QSize(100, 100));
    model.setPageNaturalSize(3, QSize(100, 25));

    model.setScrollYFromUser(49);

    QCOMPARE(model.centerPage(), 1);
    QCOMPARE(model.readingProgressPage(), 1);
}

// Layout: pages 0-2 at y=0,100,200 (100px each), page 3 at y=300 (25px).
// lastPageMidY = 300 + 12 = 312. viewportBottomY = scrollY + 249.
// Threshold scroll: 312 - 249 = 63.

void ContinuousViewModelTest::doesNotReportLastPageWhenLessThanHalfIsVisible()
{
    ContinuousViewModel model;
    model.setNumPages(4);
    model.setViewportSize(100, 250);
    model.setPageNaturalSize(0, QSize(100, 100));
    model.setPageNaturalSize(1, QSize(100, 100));
    model.setPageNaturalSize(2, QSize(100, 100));
    model.setPageNaturalSize(3, QSize(100, 25));

    model.setScrollYFromUser(62); // viewportBottomY=311, one pixel short of midpoint

    QCOMPARE(model.readingProgressPage(), model.centerPage());
}

void ContinuousViewModelTest::reportsLastPageWhenAtLeastHalfIsVisible()
{
    ContinuousViewModel model;
    model.setNumPages(4);
    model.setViewportSize(100, 250);
    model.setPageNaturalSize(0, QSize(100, 100));
    model.setPageNaturalSize(1, QSize(100, 100));
    model.setPageNaturalSize(2, QSize(100, 100));
    model.setPageNaturalSize(3, QSize(100, 25));

    model.setScrollYFromUser(63); // viewportBottomY=312, exactly at midpoint

    QCOMPARE(model.readingProgressPage(), 3);
}

QTEST_MAIN(ContinuousViewModelTest)

#include "main.moc"
