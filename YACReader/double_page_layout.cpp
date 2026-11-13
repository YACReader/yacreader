#include "double_page_layout.h"

#include <QtGlobal>

void DoublePageLayout::reset(int numPages, bool coverIsSinglePage)
{
    pageCount = qMax(0, numPages);
    coverAlone = coverIsSinglePage;
    shapes.fill(PageShape::Unknown, pageCount);
    userAnchor = -1;
    keptAnchors.clear();
    dirty = true;
}

int DoublePageLayout::numPages() const
{
    return pageCount;
}

void DoublePageLayout::setCoverIsSinglePage(bool coverIsSinglePage)
{
    if (coverAlone != coverIsSinglePage) {
        coverAlone = coverIsSinglePage;
        dirty = true;
    }
}

bool DoublePageLayout::coverIsSinglePage() const
{
    return coverAlone;
}

void DoublePageLayout::setPageShape(int page, PageShape shape)
{
    if (page < 0 || page >= pageCount || shapes[page] == shape) {
        return;
    }
    shapes[page] = shape;
    dirty = true;
}

void DoublePageLayout::setAnchor(int page)
{
    userAnchor = (page >= 0 && page < pageCount) ? page : -1;
    keptAnchors.clear();
    dirty = true;
}

void DoublePageLayout::keepView(const View &view)
{
    if (!view.isValid() || view.last >= pageCount || viewStartingAt(view.first) == view) {
        return;
    }

    keptAnchors.insert(view.first);
    if (!view.isDouble() && view.first + 1 < pageCount) {
        keptAnchors.insert(view.first + 1);
    }
    dirty = true;
}

DoublePageLayout::View DoublePageLayout::viewAt(int page) const
{
    if (page < 0 || page >= pageCount) {
        return { };
    }
    ensureLayout();
    const int first = viewStarts[page];
    const int last = (first + 1 < pageCount && viewStarts[first + 1] == first) ? first + 1 : first;
    return { first, last };
}

DoublePageLayout::View DoublePageLayout::viewStartingAt(int page) const
{
    const View view = viewAt(page);
    if (!view.isValid() || view.first == page) {
        return view;
    }
    return { page, page };
}

DoublePageLayout::View DoublePageLayout::nextView(const View &view) const
{
    if (!view.isValid() || view.last + 1 >= pageCount) {
        return { };
    }
    return viewAt(view.last + 1);
}

DoublePageLayout::View DoublePageLayout::previousView(const View &view) const
{
    if (!view.isValid() || view.first <= 0) {
        return { };
    }
    return viewAt(view.first - 1);
}

DoublePageLayout::View DoublePageLayout::firstView() const
{
    return viewAt(0);
}

bool DoublePageLayout::isSinglePage(int page) const
{
    return (page == 0 && coverAlone) || shapes[page] == PageShape::Wide;
}

void DoublePageLayout::ensureLayout() const
{
    if (!dirty) {
        return;
    }
    dirty = false;

    viewStarts.resize(pageCount);

    auto isAnchor = [this](int page) {
        return page == userAnchor || keptAnchors.contains(page);
    };

    int page = 0;
    while (page < pageCount) {
        if (isSinglePage(page)) {
            viewStarts[page] = page;
            page++;
            continue;
        }

        // run of pages that can be paired: [start, end]
        const int start = page;
        int end = start;
        while (end + 1 < pageCount && !isSinglePage(end + 1) && !isAnchor(end + 1)) {
            end++;
        }

        // an anchor on a page that is alone anyway (the cover, a wide page) does not change the run before it
        const bool backwards = end + 1 < pageCount && isAnchor(end + 1) && !isSinglePage(end + 1) && !isAnchor(start);
        if (backwards) {
            for (int i = end; i >= start; i -= 2) {
                if (i - 1 >= start) {
                    viewStarts[i - 1] = i - 1;
                    viewStarts[i] = i - 1;
                } else {
                    viewStarts[i] = i;
                }
            }
        } else {
            for (int i = start; i <= end; i += 2) {
                viewStarts[i] = i;
                if (i + 1 <= end) {
                    viewStarts[i + 1] = i;
                }
            }
        }

        page = end + 1;
    }
}

void DoublePageNavigator::reset(int page)
{
    pageLayout.setAnchor(page);
    intent = Intent::None;
    intentPage = -1;
    lastDisplayedView = { };
}

void DoublePageNavigator::setPageShape(int page, DoublePageLayout::PageShape shape)
{
    pageLayout.setPageShape(page, shape);
    keepDisplayedView();
}

DoublePageNavigator::View DoublePageNavigator::viewFor(int page) const
{
    if (page >= 0 && page < pageLayout.numPages()) {
        return pageLayout.viewStartingAt(page);
    }
    return { page, page };
}

int DoublePageNavigator::next(int currentPage)
{
    keepDisplayedView();
    const View view = viewFor(currentPage);
    if (!view.isValid() || view.last + 1 >= pageLayout.numPages()) {
        return -1;
    }

    intent = Intent::Next;
    intentPage = view.last + 1;
    return resolve(currentPage);
}

int DoublePageNavigator::previous(int currentPage)
{
    keepDisplayedView();
    const View view = viewFor(currentPage);
    if (!view.isValid() || view.first <= 0) {
        return -1;
    }

    intent = Intent::Previous;
    intentPage = view.first - 1;
    return resolve(currentPage);
}

int DoublePageNavigator::last()
{
    if (pageLayout.numPages() <= 0) {
        return -1;
    }

    pageLayout.setAnchor(-1);
    lastDisplayedView = { };
    intent = Intent::Previous;
    intentPage = pageLayout.numPages() - 1;
    return resolve(-1);
}

int DoublePageNavigator::resolve(int currentPage) const
{
    if (intent == Intent::None) {
        return currentPage;
    }

    const View view = pageLayout.viewAt(intentPage);
    if (!view.isValid()) {
        return currentPage;
    }

    // never show again a page of the view the user comes from
    if (intent == Intent::Next && view.first < intentPage && lastDisplayedView.contains(view.first)) {
        return intentPage;
    }
    return view.first;
}

void DoublePageNavigator::viewDisplayed(const View &view)
{
    lastDisplayedView = view;
    intent = Intent::None;
    intentPage = -1;
}

void DoublePageNavigator::keepDisplayedView()
{
    if (lastDisplayedView.isValid()) {
        pageLayout.keepView(lastDisplayedView);
    }
}
