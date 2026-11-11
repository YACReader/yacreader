#include "double_page_layout.h"

#include <QHash>
#include <QObject>
#include <QSet>
#include <QStringList>
#include <QTest>

#include <vector>

using Shape = DoublePageLayout::PageShape;
using View = DoublePageLayout::View;

namespace {

QString viewText(const View &view)
{
    if (view.isDouble()) {
        return QStringLiteral("[%1,%2]").arg(view.first + 1).arg(view.last + 1);
    }
    return QStringLiteral("[%1]").arg(view.first + 1);
}

// The comic is extracted while the user reads. Page numbers are 0-based, the texts use 1-based numbers.
struct Scenario {
    const char *id;
    const char *title;
    int numPages;
    std::vector<int> widePages; // as seen on screen
    int openAt;
    bool coverIsSinglePage;
    std::vector<int> extractionOrder;
    double extractionSpeed; // pages extracted per page turn while the key is held, < 0: every size is known at once
    int readingTime; // pages extracted while the user looks at a view
    std::vector<int> failedPages; // never extracted (CRC error), known as portrait pages when the extraction ends
    std::vector<const char *> actions;
    const char *expectedViews;
};

// Plays the part of Render: page sizes arrive in extraction order, a view is displayed when all its pages are known.
class Reader
{
public:
    explicit Reader(const Scenario &scenario)
        : s(scenario)
    {
        for (int page : s.widePages) {
            wide.insert(page);
        }
        for (int page : s.failedPages) {
            failed.insert(page);
        }

        navigator.layout().reset(s.numPages, s.coverIsSinglePage);
        if (s.extractionSpeed < 0) {
            for (int page = 0; page < s.numPages; ++page) {
                if (!failed.contains(page)) {
                    learn(page);
                }
            }
            learnFailedPages();
        }
        navigator.reset(s.openAt);
        position = s.openAt;
        tryDisplay();
    }

    void run(const QString &action)
    {
        const QStringList parts = action.split(QLatin1Char(' '));
        const QString kind = parts.at(0);
        if (kind.contains(QLatin1Char('*'))) {
            const QStringList repeated = kind.split(QLatin1Char('*'));
            for (int i = 0; i < repeated.at(1).toInt(); ++i) {
                turn(repeated.at(0) == QLatin1String("next"), 1, false);
            }
        } else if (kind == QLatin1String("next") || kind == QLatin1String("prev")) {
            const int count = parts.size() > 1 ? parts.at(1).toInt() : 1;
            const bool hold = parts.size() > 2 && parts.at(2) == QLatin1String("hold");
            turn(kind == QLatin1String("next"), count, hold);
        } else if (kind == QLatin1String("goto")) {
            jumpTo(parts.at(1).toInt() - 1);
        } else if (kind == QLatin1String("shift")) {
            jumpTo(qBound(0, position + parts.at(1).toInt(), s.numPages - 1));
        } else if (kind == QLatin1String("cover")) {
            navigator.layout().setCoverIsSinglePage(parts.at(1) == QLatin1String("on"));
            jumpTo(lastShown.isValid() ? lastShown.first : position);
        } else if (kind == QLatin1String("last")) {
            newEpoch();
            position = navigator.last();
            waitForDisplay();
        } else if (kind == QLatin1String("wait")) {
            extract(s.numPages);
            tryDisplay();
        } else {
            QFAIL(qPrintable(QStringLiteral("unknown action %1").arg(action)));
        }
    }

    QString shownViews() const { return shown.join(QLatin1Char(' ')); }
    QStringList problems;

private:
    struct PageTurn {
        bool next = true;
        int presses = 0;
    };

    void turn(bool next, int count, bool hold)
    {
        int presses = 0;
        for (int i = 0; i < count; ++i) {
            const int page = next ? navigator.next(position) : navigator.previous(position);
            if (page >= 0) {
                position = page;
                presses++;
            }
            if (hold && s.extractionSpeed >= 0) {
                extract(s.extractionSpeed);
                tryDisplay(); // a view may flash while the key is held
            }
        }
        if (presses == 0) {
            return;
        }
        pendingTurn = PageTurn { next, presses };
        hasPendingTurn = true;
        waitForDisplay();
    }

    void jumpTo(int page)
    {
        newEpoch();
        position = page;
        navigator.reset(page);
        waitForDisplay();
    }

    void newEpoch()
    {
        lastShown = { };
        hasPendingTurn = false;
        seenViews.clear();
    }

    void learn(int page)
    {
        known.insert(page);
        navigator.setPageShape(page, wide.contains(page) ? Shape::Wide : Shape::Portrait);
        // Render resolves the pending page turn when new sizes are known
        if (navigator.hasPendingPageTurn()) {
            position = navigator.resolve(position);
        }
    }

    void learnFailedPages()
    {
        for (int page : std::as_const(failed)) {
            known.insert(page);
            navigator.setPageShape(page, Shape::Portrait);
        }
    }

    void extract(double pages)
    {
        accumulated += pages;
        while (accumulated >= 1 && known.size() < s.numPages) {
            for (int page : s.extractionOrder) {
                if (!known.contains(page) && !failed.contains(page)) {
                    learn(page);
                    break;
                }
            }
            if (known.size() + failed.size() >= s.numPages) {
                learnFailedPages();
            }
            accumulated -= 1;
        }
    }

    bool tryDisplay()
    {
        if (navigator.hasPendingPageTurn()) {
            position = navigator.resolve(position);
        }
        const View view = navigator.viewFor(position);
        for (int page = view.first; page <= view.last; ++page) {
            if (!known.contains(page)) {
                return false;
            }
        }
        navigator.viewDisplayed(view);
        if (view == lastShown) {
            return true;
        }

        if (view.isDouble() && (wide.contains(view.first) || wide.contains(view.last))) {
            problems << QStringLiteral("wide page paired in %1").arg(viewText(view));
        }
        if (lastShown.isValid() && hasPendingTurn) {
            if (view.contains(lastShown.first) || view.contains(lastShown.last)) {
                problems << QStringLiteral("duplicated page: %1 -> %2").arg(viewText(lastShown), viewText(view));
            }
            if (pendingTurn.presses == 1) {
                const bool skipped = pendingTurn.next ? view.first > lastShown.last + 1 : view.last < lastShown.first - 1;
                if (skipped) {
                    problems << QStringLiteral("skipped pages: %1 -> %2").arg(viewText(lastShown), viewText(view));
                }
            }
        }
        for (int page = view.first; page <= view.last; ++page) {
            const auto seen = seenViews.constFind(page);
            if (seen != seenViews.constEnd() && *seen != viewText(view)) {
                problems << QStringLiteral("page %1 was in %2, now in %3").arg(page + 1).arg(*seen, viewText(view));
            }
            seenViews.insert(page, viewText(view));
        }

        shown << viewText(view);
        lastShown = view;
        hasPendingTurn = false;
        return true;
    }

    void waitForDisplay()
    {
        int guard = 0;
        while (!tryDisplay()) {
            extract(1);
            if (++guard > 10 * s.numPages) {
                problems << QStringLiteral("the view of page %1 is never displayed").arg(position + 1);
                return;
            }
        }
        if (s.extractionSpeed >= 0) {
            extract(s.readingTime);
        }
    }

    const Scenario &s;
    DoublePageNavigator navigator;
    QSet<int> wide;
    QSet<int> failed;
    QSet<int> known;
    double accumulated = 0;
    int position = 0;
    View lastShown;
    PageTurn pendingTurn;
    bool hasPendingTurn = false;
    QHash<int, QString> seenViews;
    QStringList shown;
};

// Generated with a simulator of the double page mode (all the cases reviewed for the fix). Each row: id, title, pages, wide pages,
// open at, cover is single page, extraction order, extraction speed, reading time, failed pages, actions, views shown.
const std::vector<Scenario> scenarios = {
    { "K1", "All portrait, open at cover, read to end and back", 24, { }, 0, true, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23 }, -1, 2, { }, { "next*30", "prev*30" }, "[1] [2,3] [4,5] [6,7] [8,9] [10,11] [12,13] [14,15] [16,17] [18,19] [20,21] [22,23] [24] [22,23] [20,21] [18,19] [16,17] [14,15] [12,13] [10,11] [8,9] [6,7] [4,5] [2,3] [1]" },
    { "K2", "Spread p17, open at cover, read to end and back", 24, { 16 }, 0, true, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23 }, -1, 2, { }, { "next*30", "prev*30" }, "[1] [2,3] [4,5] [6,7] [8,9] [10,11] [12,13] [14,15] [16] [17] [18,19] [20,21] [22,23] [24] [22,23] [20,21] [18,19] [17] [16] [14,15] [12,13] [10,11] [8,9] [6,7] [4,5] [2,3] [1]" },
    { "K3", "Spread p18, open at cover, read to end and back", 24, { 17 }, 0, true, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23 }, -1, 2, { }, { "next*30", "prev*30" }, "[1] [2,3] [4,5] [6,7] [8,9] [10,11] [12,13] [14,15] [16,17] [18] [19,20] [21,22] [23,24] [21,22] [19,20] [18] [16,17] [14,15] [12,13] [10,11] [8,9] [6,7] [4,5] [2,3] [1]" },
    { "K4", "Resume at p16, spread p17, forward to end, back to cover", 24, { 16 }, 15, true, { 15, 16, 17, 18, 19, 20, 21, 22, 23, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14 }, -1, 2, { }, { "next*30", "prev*30" }, "[16] [17] [18,19] [20,21] [22,23] [24] [22,23] [20,21] [18,19] [17] [16] [14,15] [12,13] [10,11] [8,9] [6,7] [4,5] [2,3] [1]" },
    { "K5", "Resume at p15, spread p17, forward, back", 24, { 16 }, 14, true, { 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13 }, -1, 2, { }, { "next*30", "prev*30" }, "[15,16] [17] [18,19] [20,21] [22,23] [24] [22,23] [20,21] [18,19] [17] [15,16] [13,14] [11,12] [9,10] [7,8] [5,6] [3,4] [2] [1]" },
    { "K6", "Resume at p16, spread p17, back first, then forward", 24, { 16 }, 15, true, { 15, 16, 17, 18, 19, 20, 21, 22, 23, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14 }, -1, 2, { }, { "prev*30", "next*30" }, "[16] [14,15] [12,13] [10,11] [8,9] [6,7] [4,5] [2,3] [1] [2,3] [4,5] [6,7] [8,9] [10,11] [12,13] [14,15] [16] [17] [18,19] [20,21] [22,23] [24]" },
    { "K7", "Resume at p17 (the spread itself)", 24, { 16 }, 16, true, { 16, 17, 18, 19, 20, 21, 22, 23, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 }, -1, 2, { }, { "prev*3", "next*5" }, "[17] [16] [14,15] [12,13] [14,15] [16] [17] [18,19] [20,21]" },
    { "K8", "Resume at p18 (just after the spread)", 24, { 16 }, 17, true, { 17, 18, 19, 20, 21, 22, 23, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 }, -1, 2, { }, { "prev*3", "next*5" }, "[18,19] [17] [16] [14,15] [16] [17] [18,19] [20,21] [22,23]" },
    { "K9", "Two spreads with one page between: p10, p12", 24, { 9, 11 }, 0, true, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23 }, -1, 2, { }, { "next*30", "prev*30" }, "[1] [2,3] [4,5] [6,7] [8,9] [10] [11] [12] [13,14] [15,16] [17,18] [19,20] [21,22] [23,24] [21,22] [19,20] [17,18] [15,16] [13,14] [12] [11] [10] [8,9] [6,7] [4,5] [2,3] [1]" },
    { "K10", "Two consecutive spreads: p10, p11", 24, { 9, 10 }, 0, true, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23 }, -1, 2, { }, { "next*30", "prev*30" }, "[1] [2,3] [4,5] [6,7] [8,9] [10] [11] [12,13] [14,15] [16,17] [18,19] [20,21] [22,23] [24] [22,23] [20,21] [18,19] [16,17] [14,15] [12,13] [11] [10] [8,9] [6,7] [4,5] [2,3] [1]" },
    { "K11", "Spread right after cover: p2", 24, { 1 }, 0, true, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23 }, -1, 2, { }, { "next*4", "prev*4" }, "[1] [2] [3,4] [5,6] [7,8] [5,6] [3,4] [2] [1]" },
    { "K12", "Spread is the last page", 24, { 23 }, 0, true, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23 }, -1, 2, { }, { "next*30", "prev*3" }, "[1] [2,3] [4,5] [6,7] [8,9] [10,11] [12,13] [14,15] [16,17] [18,19] [20,21] [22,23] [24] [22,23] [20,21] [18,19]" },
    { "K13", "Wide cover (p1 is a spread)", 24, { 0 }, 0, true, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23 }, -1, 2, { }, { "next*4", "prev*4" }, "[1] [2,3] [4,5] [6,7] [8,9] [6,7] [4,5] [2,3] [1]" },
    { "K14", "Cover-is-single OFF, spread p17", 24, { 16 }, 0, false, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23 }, -1, 2, { }, { "next*30", "prev*30" }, "[1,2] [3,4] [5,6] [7,8] [9,10] [11,12] [13,14] [15,16] [17] [18,19] [20,21] [22,23] [24] [22,23] [20,21] [18,19] [17] [15,16] [13,14] [11,12] [9,10] [7,8] [5,6] [3,4] [1,2]" },
    { "K15", "Odd page count (23 pages), all portrait", 23, { }, 0, true, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22 }, -1, 2, { }, { "next*30", "prev*3" }, "[1] [2,3] [4,5] [6,7] [8,9] [10,11] [12,13] [14,15] [16,17] [18,19] [20,21] [22,23] [20,21] [18,19] [16,17]" },
    { "K16", "Back and forth at the spread many times (p16)", 24, { 16 }, 15, true, { 15, 16, 17, 18, 19, 20, 21, 22, 23, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14 }, -1, 2, { }, { "prev", "next*2", "prev*2", "next*3", "prev*2" }, "[16] [14,15] [16] [17] [16] [14,15] [16] [17] [18,19] [17] [16]" },
    { "K17", "Slider jump p3 -> p20 (spread p17), then back past the spread", 24, { 16 }, 0, true, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23 }, -1, 2, { }, { "next", "goto 20", "prev*6" }, "[1] [2,3] [20,21] [18,19] [17] [16] [14,15] [12,13] [10,11]" },
    { "K18", "Slider jump p3 -> p19 (spread p17), then back", 24, { 16 }, 0, true, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23 }, -1, 2, { }, { "next", "goto 19", "prev*6" }, "[1] [2,3] [19,20] [18] [17] [16] [14,15] [12,13] [10,11]" },
    { "K19", "Manual shift +1 at p6, read past spread p17, back", 24, { 16 }, 0, true, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23 }, -1, 2, { }, { "next*3", "shift 1", "next*8", "prev*10" }, "[1] [2,3] [4,5] [6,7] [7,8] [9,10] [11,12] [13,14] [15,16] [17] [18,19] [20,21] [22,23] [20,21] [18,19] [17] [15,16] [13,14] [11,12] [9,10] [7,8] [5,6] [3,4]" },
    { "K20", "Manual shift -1 at p6, read on", 24, { 16 }, 0, true, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23 }, -1, 2, { }, { "next*3", "shift -1", "next*4", "prev*4" }, "[1] [2,3] [4,5] [6,7] [5,6] [7,8] [9,10] [11,12] [13,14] [11,12] [9,10] [7,8] [5,6]" },
    { "K21", "Jump to p10, then jump back to p2 (old anchor dropped)", 24, { 16 }, 0, true, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23 }, -1, 2, { }, { "goto 10", "next*2", "goto 2", "next*6" }, "[1] [10,11] [12,13] [14,15] [2,3] [4,5] [6,7] [8,9] [10,11] [12,13] [14,15]" },
    { "S1", "FORUM CASE: all portrait, hold the key, extraction 1 page/press", 24, { }, 0, true, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23 }, 1, 2, { }, { "next 20 hold", "prev*30" }, "[24] [22,23] [20,21] [18,19] [16,17] [14,15] [12,13] [10,11] [8,9] [6,7] [4,5] [2,3] [1]" },
    { "S2", "All portrait, hold key, extraction 0.5 page/press", 24, { }, 0, true, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23 }, 0.5, 2, { }, { "next 20 hold", "prev*30", "next*30" }, "[24] [22,23] [20,21] [18,19] [16,17] [14,15] [12,13] [10,11] [8,9] [6,7] [4,5] [2,3] [1] [2,3] [4,5] [6,7] [8,9] [10,11] [12,13] [14,15] [16,17] [18,19] [20,21] [22,23] [24]" },
    { "S3", "Spread p17, slow extraction, normal reading from cover", 24, { 16 }, 0, true, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23 }, 1, 2, { }, { "next*30", "prev*30" }, "[2,3] [4,5] [6,7] [8,9] [10,11] [12,13] [14,15] [16] [17] [18,19] [20,21] [22,23] [24] [22,23] [20,21] [18,19] [17] [16] [14,15] [12,13] [10,11] [8,9] [6,7] [4,5] [2,3] [1]" },
    { "S4", "Spread p17, hold key past the unknown spread, then back", 24, { 16 }, 0, true, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23 }, 0.5, 2, { }, { "next 12 hold", "prev*30" }, "[24] [22,23] [20,21] [18,19] [17] [16] [14,15] [12,13] [10,11] [8,9] [6,7] [4,5] [2,3] [1]" },
    { "S5", "Resume p16, spread p17, slow extraction, back first (p1-15 extracted last)", 24, { 16 }, 15, true, { 15, 16, 17, 18, 19, 20, 21, 22, 23, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14 }, 1, 2, { }, { "prev*30", "next*30" }, "[14,15] [12,13] [10,11] [8,9] [6,7] [4,5] [2,3] [1] [2,3] [4,5] [6,7] [8,9] [10,11] [12,13] [14,15] [16] [17] [18,19] [20,21] [22,23] [24]" },
    { "S6", "Resume p16, spread p10 unknown, back past it while it is still unknown", 24, { 9 }, 15, true, { 15, 16, 17, 18, 19, 20, 21, 22, 23, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14 }, 0.5, 0, { }, { "prev*5", "wait all", "next*5", "prev*5" }, "[14,15] [12,13] [11] [10] [8,9] [10] [11] [12,13] [14,15] [16,17] [14,15] [12,13] [11] [10] [8,9]" },
    { "S7", "Resume p16, spread p17 extracted LAST (archive order != page order)", 24, { 16 }, 15, true, { 15, 17, 18, 19, 20, 21, 22, 23, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 16 }, 1, 2, { }, { "next*6", "prev*6" }, "[18,19] [20,21] [22,23] [24] [22,23] [20,21] [18,19] [17] [16] [14,15]" },
    { "S8", "Read from cover, spread p17 extracted LAST, quick reading", 24, { 16 }, 0, true, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 17, 18, 19, 20, 21, 22, 23, 16 }, 2, 0, { }, { "next*30", "prev*30" }, "[2,3] [4,5] [6,7] [8,9] [10,11] [12,13] [14,15] [16] [17] [18,19] [20,21] [22,23] [24] [22,23] [20,21] [18,19] [17] [16] [14,15] [12,13] [10,11] [8,9] [6,7] [4,5] [2,3] [1]" },
    { "S9", "Spread p5 extracted last; user reads p6-p12, sees it late", 24, { 4 }, 0, true, { 0, 1, 2, 3, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 4 }, 3, 0, { }, { "next*6", "wait all", "prev*6" }, "[2,3] [4] [5] [6,7] [8,9] [10,11] [8,9] [6,7] [5] [4] [2,3] [1]" },
    { "S10", "Jump to p20 while p1-19 unknown, spread p17, back", 24, { 16 }, 0, true, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23 }, 1, 2, { }, { "goto 20", "prev*30" }, "[20,21] [18,19] [17] [16] [14,15] [12,13] [10,11] [8,9] [6,7] [4,5] [2,3] [1]" },
    { "S11", "Hold key across 2 unknown spreads (p9, p17), then read back", 24, { 8, 16 }, 0, true, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23 }, 0.5, 2, { }, { "next 14 hold", "prev*30", "next*30" }, "[24] [22,23] [20,21] [18,19] [17] [16] [14,15] [12,13] [10,11] [9] [8] [6,7] [4,5] [2,3] [1] [2,3] [4,5] [6,7] [8] [9] [10,11] [12,13] [14,15] [16] [17] [18,19] [20,21] [22,23] [24]" },
    { "S12", "Hold key back from resume p22, spread p11, unknown", 24, { 10 }, 21, true, { 21, 22, 23, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20 }, 0.5, 2, { }, { "prev 12 hold", "next*30" }, "[1] [2,3] [4,5] [6,7] [8,9] [10] [11] [12,13] [14,15] [16,17] [18,19] [20,21] [22,23] [24]" },
    { "X1", "Spread p4 extracted LAST; hold key from cover to ~p12, then read on and back", 24, { 3 }, 0, true, { 0, 1, 2, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 3 }, 1, 0, { }, { "next 6 hold", "next*4", "prev*8" }, "[12,13] [14,15] [16,17] [18,19] [20,21] [18,19] [16,17] [14,15] [12,13] [10,11] [8,9] [6,7] [5]" },
    { "X2", "Same as X1 but user stops at p12 and waits until all is extracted", 24, { 3 }, 0, true, { 0, 1, 2, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 3 }, 1, 0, { }, { "next 6 hold", "wait all", "next*3", "prev*8" }, "[12,13] [14,15] [16,17] [18,19] [16,17] [14,15] [12,13] [10,11] [8,9] [6,7] [5] [4]" },
    { "X3", "Resume p16; spread p13 extracted last; user goes back slowly", 24, { 12 }, 15, true, { 15, 16, 17, 18, 19, 20, 21, 22, 23, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 14, 12 }, 1, 0, { }, { "prev*6", "next*6" }, "[14,15] [13] [12] [10,11] [8,9] [6,7] [8,9] [10,11] [12] [13] [14,15] [16,17]" },
    { "X4", "Resume p16; p17 spread extracted last; user presses next at once", 24, { 16 }, 15, true, { 15, 17, 18, 19, 20, 21, 22, 23, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 16 }, 1, 0, { }, { "next*3", "prev*4" }, "[18,19] [20,21] [22,23] [20,21] [18,19] [17] [16]" },
    { "X5", "Random-ish: resume p9, spreads p5,p14,p15 out of order, mixed holds", 24, { 4, 13, 14 }, 8, true, { 8, 9, 10, 11, 15, 16, 17, 0, 1, 2, 3, 12, 18, 19, 13, 4, 5, 6, 7, 14, 20, 21, 22, 23 }, 0.7, 0, { }, { "next 3 hold", "next*3", "prev*2", "prev 4 hold", "next*6", "wait all", "prev*12" }, "[15] [16,17] [18,19] [20,21] [18,19] [16,17] [15] [14] [13] [11,12] [13] [14] [15] [16,17] [18,19] [20,21] [18,19] [16,17] [15] [14] [13] [11,12] [9,10] [7,8] [6] [5] [4] [2,3]" },
    { "E2", "CRC error on p9, policy: portrait when extraction ends", 24, { }, 0, true, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23 }, 1, 2, { 8 }, { "next*6", "prev*6" }, "[2,3] [4,5] [6,7] [8,9] [10,11] [12,13] [10,11] [8,9] [6,7] [4,5] [2,3] [1]" },
    { "E3", "CRC error on p16, spread p17", 24, { 16 }, 0, true, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23 }, 1, 2, { 15 }, { "next*10", "prev*10" }, "[2,3] [4,5] [6,7] [8,9] [10,11] [12,13] [14,15] [16] [17] [18,19] [17] [16] [14,15] [12,13] [10,11] [8,9] [6,7] [4,5] [2,3] [1]" },
    { "C1", "Cover single ON -> OFF on [6,7], read on, back to start", 24, { 16 }, 0, true, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23 }, -1, 2, { }, { "next*3", "cover off", "next*8", "prev*14" }, "[1] [2,3] [4,5] [6,7] [6,7] [8,9] [10,11] [12,13] [14,15] [16] [17] [18,19] [20,21] [18,19] [17] [16] [14,15] [12,13] [10,11] [8,9] [6,7] [4,5] [2,3] [1]" },
    { "C2", "Cover single OFF -> ON on [5,6], read on, back to start", 24, { 16 }, 0, false, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23 }, -1, 2, { }, { "next*2", "cover on", "next*8", "prev*14" }, "[1,2] [3,4] [5,6] [5,6] [7,8] [9,10] [11,12] [13,14] [15,16] [17] [18,19] [20,21] [18,19] [17] [15,16] [13,14] [11,12] [9,10] [7,8] [5,6] [3,4] [2] [1]" },
    { "C3", "Cover single ON -> OFF while on the cover", 24, { }, 0, true, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23 }, -1, 2, { }, { "cover off", "next*3", "prev*3" }, "[1] [1,2] [3,4] [5,6] [7,8] [5,6] [3,4] [1,2]" },
    { "L1", "Go to last page, 23 pages (odd), then back", 23, { }, 0, true, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22 }, -1, 2, { }, { "last", "prev*3" }, "[1] [22,23] [20,21] [18,19] [16,17]" },
    { "L2", "Go to last page, 24 pages, spread p17, then back", 24, { 16 }, 0, true, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23 }, -1, 2, { }, { "last", "prev*6" }, "[1] [24] [22,23] [20,21] [18,19] [17] [16] [14,15]" },
    { "L3", "Go to last page while the end is unknown (slow)", 24, { 22 }, 0, true, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23 }, 1, 2, { }, { "last", "prev*4" }, "[24] [23] [22] [20,21] [18,19]" },
    { "R1", "Rotated 90: raw landscape pages p2-p24 (portrait on screen), raw portrait p10", 24, { 0, 9 }, 0, true, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23 }, -1, 2, { }, { "next*8", "prev*8" }, "[1] [2,3] [4,5] [6,7] [8,9] [10] [11,12] [13,14] [15,16] [13,14] [11,12] [10] [8,9] [6,7] [4,5] [2,3] [1]" },
};

QList<View> allViews(const DoublePageLayout &layout)
{
    QList<View> views;
    for (View view = layout.firstView(); view.isValid(); view = layout.nextView(view)) {
        views << view;
    }
    return views;
}

QString layoutText(const DoublePageLayout &layout)
{
    QStringList texts;
    for (const View &view : allViews(layout)) {
        texts << viewText(view);
    }
    return texts.join(QLatin1Char(' '));
}

} // namespace

class DoublePageLayoutTest : public QObject
{
    Q_OBJECT

private slots:
    void unknownPagesArePortraitPages();
    void widePagesAreAlone();
    void userAnchorPairsTheRunBeforeItBackwards();
    void anchorOnAWidePageChangesNothing();
    void keptViewDoesNotChangeWhenASpreadIsFound();
    void previousAndNextViewsMatch();
    void scenarios_data();
    void scenarios();
};

void DoublePageLayoutTest::unknownPagesArePortraitPages()
{
    DoublePageLayout layout;
    layout.reset(8, true);
    QCOMPARE(layoutText(layout), QStringLiteral("[1] [2,3] [4,5] [6,7] [8]"));

    layout.setCoverIsSinglePage(false);
    QCOMPARE(layoutText(layout), QStringLiteral("[1,2] [3,4] [5,6] [7,8]"));
}

void DoublePageLayoutTest::widePagesAreAlone()
{
    DoublePageLayout layout;
    layout.reset(10, true);
    layout.setPageShape(4, Shape::Wide);
    QCOMPARE(layoutText(layout), QStringLiteral("[1] [2,3] [4] [5] [6,7] [8,9] [10]"));

    layout.setPageShape(5, Shape::Wide);
    QCOMPARE(layoutText(layout), QStringLiteral("[1] [2,3] [4] [5] [6] [7,8] [9,10]"));
}

void DoublePageLayoutTest::userAnchorPairsTheRunBeforeItBackwards()
{
    DoublePageLayout layout;
    layout.reset(10, true);
    layout.setAnchor(4); // open at page 5
    QCOMPARE(layoutText(layout), QStringLiteral("[1] [2] [3,4] [5,6] [7,8] [9,10]"));
}

void DoublePageLayoutTest::anchorOnAWidePageChangesNothing()
{
    DoublePageLayout layout;
    layout.reset(10, true);
    layout.setPageShape(5, Shape::Wide);
    const QString natural = layoutText(layout);
    layout.setAnchor(5);
    QCOMPARE(layoutText(layout), natural);
}

void DoublePageLayoutTest::keptViewDoesNotChangeWhenASpreadIsFound()
{
    DoublePageNavigator navigator;
    navigator.layout().reset(16, true);
    navigator.reset(0);
    QCOMPARE(navigator.viewFor(9), View({ 9, 10 }));
    navigator.viewDisplayed({ 9, 10 });

    // page 4 is a spread, the pages after it are paired again, but not the view the user sees
    navigator.setPageShape(3, Shape::Wide);
    QCOMPARE(navigator.viewFor(9), View({ 9, 10 }));
    QCOMPARE(layoutText(navigator.layout()), QStringLiteral("[1] [2,3] [4] [5] [6,7] [8,9] [10,11] [12,13] [14,15] [16]"));
}

void DoublePageLayoutTest::previousAndNextViewsMatch()
{
    DoublePageLayout layout;
    layout.reset(30, true);
    for (int page : { 6, 7, 13, 21, 29 }) {
        layout.setPageShape(page, Shape::Wide);
    }
    for (int anchor : { -1, 3, 10, 14, 22 }) {
        layout.setAnchor(anchor);
        const QList<View> views = allViews(layout);
        int expectedFirst = 0;
        for (int i = 0; i < views.size(); ++i) {
            QCOMPARE(views[i].first, expectedFirst);
            expectedFirst = views[i].last + 1;
            if (i > 0) {
                QCOMPARE(layout.previousView(views[i]), views[i - 1]);
            }
        }
        QCOMPARE(expectedFirst, 30);
    }
}

void DoublePageLayoutTest::scenarios_data()
{
    QTest::addColumn<int>("index");
    for (int i = 0; i < static_cast<int>(::scenarios.size()); ++i) {
        QTest::newRow(::scenarios[i].id) << i;
    }
}

void DoublePageLayoutTest::scenarios()
{
    QFETCH(int, index);
    const Scenario &scenario = ::scenarios[index];

    Reader reader(scenario);
    for (const char *action : scenario.actions) {
        reader.run(QString::fromLatin1(action));
    }

    QVERIFY2(reader.problems.isEmpty(), qPrintable(QStringLiteral("%1: %2").arg(QString::fromLatin1(scenario.title), reader.problems.join(QStringLiteral("; ")))));
    QCOMPARE(reader.shownViews(), QString::fromLatin1(scenario.expectedViews));
}

QTEST_GUILESS_MAIN(DoublePageLayoutTest)

#include "main.moc"
