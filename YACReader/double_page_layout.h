#ifndef DOUBLE_PAGE_LAYOUT_H
#define DOUBLE_PAGE_LAYOUT_H

#include <QSet>
#include <QVector>

// Groups the pages of a comic in the views of the double page mode.
//
// The layout is a pure function of what is known about the pages, it does not keep any navigation history:
// - the cover is alone when the cover is a single page,
// - a wide page (as seen on screen) is alone,
// - the other pages are paired in order from the start of their run.
//
// The page sizes are not known in advance (an archive is extracted while the user reads), a page with an
// unknown shape is laid out as a portrait page. The shape of a page is set once it is known.
//
// Anchors force a view to start at a page:
// - the anchor set by the user (open at a page, go to a page, manual offset). The run of pages that ends
//   right before it is paired backwards from it, so the lone page of an odd run is placed far from the user.
// - the kept views. A view that the user has seen must not change when new page sizes are known.
//
// An anchor on a page that already starts a view (the cover, a wide page) does not change anything.
class DoublePageLayout
{
public:
    enum class PageShape {
        Unknown,
        Portrait,
        Wide
    };

    struct View {
        int first = -1;
        int last = -1;

        bool isValid() const { return first >= 0 && last >= first; }
        bool isDouble() const { return isValid() && last > first; }
        bool contains(int page) const { return isValid() && page >= first && page <= last; }
        bool operator==(const View &other) const { return first == other.first && last == other.last; }
        bool operator!=(const View &other) const { return !(*this == other); }
    };

    // Removes everything: shapes, anchors and kept views.
    void reset(int numPages, bool coverIsSinglePage);

    int numPages() const;

    void setCoverIsSinglePage(bool coverIsSinglePage);
    bool coverIsSinglePage() const;

    void setPageShape(int page, PageShape shape);

    // The user anchor, -1 removes it. Kept views are removed too, a new anchor is a clean state.
    void setAnchor(int page);

    // Makes sure that `view` stays a view of the layout. It adds an anchor only if the layout changed under it.
    void keepView(const View &view);

    // The view that contains `page`.
    View viewAt(int page) const;
    // The view that starts at `page`, the page is shown alone if it is the second page of a pair.
    View viewStartingAt(int page) const;
    View nextView(const View &view) const;
    View previousView(const View &view) const;
    View firstView() const;

private:
    bool isSinglePage(int page) const;
    void ensureLayout() const;

    int pageCount = 0;
    bool coverAlone = true;
    QVector<PageShape> shapes;
    int userAnchor = -1;
    QSet<int> keptAnchors;

    mutable bool dirty = true;
    // the first page of the view of each page
    mutable QVector<int> viewStarts;
};

// The navigation in double page mode on top of a DoublePageLayout.
//
// A page turn (key, mouse wheel, click...) keeps its intent ("the view after page X", "the view before page X")
// until its view is displayed, because new page sizes can change the view of a page while it is loading.
// The last displayed view is kept, so what the user sees (or comes from) never changes.
class DoublePageNavigator
{
public:
    using View = DoublePageLayout::View;

    DoublePageLayout &layout() { return pageLayout; }
    const DoublePageLayout &layout() const { return pageLayout; }

    // A clean state: `page` starts a view (open, go to, offset, rotation, settings).
    void reset(int page);

    // Sets the shape of a page without changing the last displayed view.
    void setPageShape(int page, DoublePageLayout::PageShape shape);

    // The view shown for `page`, it is alone if the layout pairs it with the previous page.
    View viewFor(int page) const;

    // The page of the next or previous view, -1 when there is none.
    int next(int currentPage);
    int previous(int currentPage);
    // The page of the last view, laid out like when the comic is read from the cover.
    int last();

    // The page that must be current now, the pending page turn is resolved with what is known now.
    int resolve(int currentPage) const;
    bool hasPendingPageTurn() const { return intent != Intent::None; }

    void viewDisplayed(const View &view);

private:
    enum class Intent {
        None,
        Next,
        Previous
    };

    void keepDisplayedView();

    DoublePageLayout pageLayout;
    Intent intent = Intent::None;
    int intentPage = -1;
    View lastDisplayedView;
};

#endif // DOUBLE_PAGE_LAYOUT_H
