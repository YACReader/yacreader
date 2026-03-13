#include "continuous_view_model.h"

#include <QWidget>
#include <QtMath>

#include <algorithm>
#include <limits>

ContinuousViewModel::ContinuousViewModel(QObject *parent)
    : QObject(parent)
{
}

void ContinuousViewModel::reset()
{
    numPagesValue = 0;
    pageSizes.clear();
    defaultPageSize = QSize(800, 1200);
    scrollYValue = 0;
    anchorPage = -1;
    layoutSnapshot = LayoutSnapshot();
    emit stateChanged();
}

void ContinuousViewModel::setNumPages(int count)
{
    numPagesValue = std::max(0, count);
    pageSizes.fill(QSize(0, 0), numPagesValue);
    defaultPageSize = QSize(800, 1200);
    recompute(RecomputePolicy::PreserveScrollClamped);
}

void ContinuousViewModel::setZoomFactor(int zoom)
{
    if (zoomFactorValue == zoom) {
        return;
    }

    zoomFactorValue = zoom;
    recompute(RecomputePolicy::PreserveViewportAnchor);
}

void ContinuousViewModel::setViewportSize(int width, int height)
{
    width = std::max(0, width);
    height = std::max(0, height);

    if (viewportWidth == width && viewportHeightValue == height) {
        return;
    }

    viewportWidth = width;
    viewportHeightValue = height;
    recompute(RecomputePolicy::PreserveViewportAnchor);
}

void ContinuousViewModel::setScrollYFromUser(int scrollY)
{
    scrollYValue = std::max(0, scrollY);
    recompute(RecomputePolicy::PreserveScrollClamped);
}

void ContinuousViewModel::setAnchorPage(int page)
{
    if (page < 0 || page >= numPagesValue) {
        return;
    }

    anchorPage = page;
}

void ContinuousViewModel::setCurrentPage(int page)
{
    if (page < 0 || page >= numPagesValue) {
        return;
    }

    anchorPage = page;
    recompute(RecomputePolicy::ScrollToPageTop, page);
}

void ContinuousViewModel::setPageNaturalSize(int pageIndex, const QSize &size)
{
    if (pageIndex < 0 || pageIndex >= numPagesValue || size.isEmpty()) {
        return;
    }

    if (defaultPageSize == QSize(800, 1200)) {
        defaultPageSize = size;
    }

    if (pageSizes[pageIndex] == size) {
        return;
    }

    pageSizes[pageIndex] = size;
    recompute(RecomputePolicy::PreserveViewportAnchor);
}

int ContinuousViewModel::numPages() const
{
    return numPagesValue;
}

int ContinuousViewModel::totalHeight() const
{
    return layoutSnapshot.totalHeight;
}

int ContinuousViewModel::scrollY() const
{
    return scrollYValue;
}

int ContinuousViewModel::viewportHeight() const
{
    return viewportHeightValue;
}

int ContinuousViewModel::zoomFactor() const
{
    return zoomFactorValue;
}

int ContinuousViewModel::centerPage() const
{
    const int centerY = scrollYValue + std::max(0, viewportHeightValue / 2);
    return pageAtY(centerY);
}

int ContinuousViewModel::yPositionForPage(int pageIndex) const
{
    if (pageIndex < 0 || pageIndex >= layoutSnapshot.yPositions.size()) {
        return 0;
    }

    return layoutSnapshot.yPositions[pageIndex];
}

int ContinuousViewModel::pageAtY(int y) const
{
    return pageAtY(layoutSnapshot, y);
}

QSize ContinuousViewModel::scaledPageSize(int pageIndex) const
{
    if (pageIndex < 0 || pageIndex >= layoutSnapshot.scaledSizes.size()) {
        return QSize();
    }

    return layoutSnapshot.scaledSizes[pageIndex];
}

void ContinuousViewModel::recompute(RecomputePolicy policy, int targetPage)
{
    const LayoutSnapshot oldSnapshot = layoutSnapshot;

    const int effectiveWidth = viewportWidth > 0 ? viewportWidth : defaultPageSize.width();

    ViewportAnchor anchor;
    if (policy == RecomputePolicy::PreserveViewportAnchor) {
        if (!oldSnapshot.yPositions.isEmpty() && viewportHeightValue > 0) {
            anchor = anchorFromViewport(oldSnapshot, scrollYValue, viewportHeightValue);
        } else if (anchorPage >= 0) {
            anchor.pageIndex = anchorPage;
            anchor.offsetRatio = 0.5f;
            anchor.valid = true;
        }
    }

    layoutSnapshot = buildLayoutSnapshot(effectiveWidth);

    if (policy == RecomputePolicy::ScrollToPageTop) {
        scrollYValue = yPositionForPage(targetPage);
    } else if (policy == RecomputePolicy::PreserveViewportAnchor && anchor.valid && viewportHeightValue > 0) {
        scrollYValue = resolveAnchorToScrollY(layoutSnapshot, anchor, viewportHeightValue);
    }

    scrollYValue = qBound(0, scrollYValue, maxScrollFor(layoutSnapshot));

    emit stateChanged();
}

ContinuousViewModel::LayoutSnapshot ContinuousViewModel::buildLayoutSnapshot(int width) const
{
    LayoutSnapshot snapshot;

    if (numPagesValue <= 0 || width <= 0) {
        return snapshot;
    }

    snapshot.yPositions.resize(numPagesValue);
    snapshot.scaledSizes.resize(numPagesValue);

    qint64 y = 0;
    for (int i = 0; i < numPagesValue; ++i) {
        snapshot.yPositions[i] = static_cast<int>(std::min<qint64>(y, std::numeric_limits<int>::max()));
        QSize scaled = scaledPageSizeForWidth(i, width);
        scaled.setWidth(std::max(1, scaled.width()));
        scaled.setHeight(std::max(1, scaled.height()));
        snapshot.scaledSizes[i] = scaled;
        y += scaled.height();
    }

    snapshot.totalHeight = static_cast<int>(std::min<qint64>(y, static_cast<qint64>(QWIDGETSIZE_MAX)));
    return snapshot;
}

ContinuousViewModel::ViewportAnchor ContinuousViewModel::anchorFromViewport(const LayoutSnapshot &snapshot, int scrollY, int viewportHeight) const
{
    ViewportAnchor anchor;

    if (snapshot.yPositions.isEmpty() || viewportHeight <= 0) {
        return anchor;
    }

    const int maxScroll = std::max(0, snapshot.totalHeight - viewportHeight);
    const int clampedScroll = qBound(0, scrollY, maxScroll);
    const int anchorY = clampedScroll + viewportHeight / 2;
    const int page = pageAtY(snapshot, anchorY);

    if (page < 0 || page >= snapshot.scaledSizes.size()) {
        return anchor;
    }

    const int pageTop = snapshot.yPositions[page];
    const int pageHeight = std::max(1, snapshot.scaledSizes[page].height());
    const float ratio = static_cast<float>(anchorY - pageTop) / static_cast<float>(pageHeight);

    anchor.pageIndex = page;
    anchor.offsetRatio = qBound(0.0f, ratio, 1.0f);
    anchor.valid = true;
    return anchor;
}

int ContinuousViewModel::resolveAnchorToScrollY(const LayoutSnapshot &snapshot, const ViewportAnchor &anchor, int viewportHeight) const
{
    if (!anchor.valid || viewportHeight <= 0 || snapshot.yPositions.isEmpty()) {
        return 0;
    }

    if (anchor.pageIndex < 0 || anchor.pageIndex >= snapshot.yPositions.size() || anchor.pageIndex >= snapshot.scaledSizes.size()) {
        return 0;
    }

    const int pageTop = snapshot.yPositions[anchor.pageIndex];
    const int pageHeight = std::max(1, snapshot.scaledSizes[anchor.pageIndex].height());
    const int anchorY = pageTop + qRound(anchor.offsetRatio * pageHeight);
    const int maxScroll = std::max(0, snapshot.totalHeight - viewportHeight);
    const int target = anchorY - viewportHeight / 2;
    return qBound(0, target, maxScroll);
}

int ContinuousViewModel::pageAtY(const LayoutSnapshot &snapshot, int y) const
{
    if (snapshot.yPositions.isEmpty()) {
        return 0;
    }

    auto it = std::upper_bound(snapshot.yPositions.constBegin(), snapshot.yPositions.constEnd(), y);
    if (it == snapshot.yPositions.constBegin()) {
        return 0;
    }

    --it;
    return static_cast<int>(it - snapshot.yPositions.constBegin());
}

int ContinuousViewModel::maxScrollFor(const LayoutSnapshot &snapshot) const
{
    return std::max(0, snapshot.totalHeight - viewportHeightValue);
}

QSize ContinuousViewModel::scaledPageSizeForWidth(int pageIndex, int width) const
{
    QSize natural = (pageIndex < pageSizes.size() && pageSizes[pageIndex].width() > 0 && pageSizes[pageIndex].height() > 0)
            ? pageSizes[pageIndex]
            : defaultPageSize;

    const float scale = scaleForPage(pageIndex, width);
    const int scaledW = std::max(1, qRound(natural.width() * scale));
    const int scaledH = std::max(1, qRound(natural.height() * scale));
    return QSize(scaledW, scaledH);
}

float ContinuousViewModel::scaleForPage(int pageIndex, int width) const
{
    QSize natural = (pageIndex < pageSizes.size() && pageSizes[pageIndex].width() > 0 && pageSizes[pageIndex].height() > 0)
            ? pageSizes[pageIndex]
            : defaultPageSize;

    if (natural.width() <= 0 || width <= 0) {
        return 1.0f;
    }

    const float baseScale = static_cast<float>(width) / natural.width();
    const float zoomMultiplier = zoomFactorValue / 100.0f;
    return baseScale * zoomMultiplier;
}
