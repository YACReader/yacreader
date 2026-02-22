#include "continuous_page_widget.h"
#include "render.h"

#include <QPainter>
#include <QPaintEvent>
#include <algorithm>
#include <limits>

ContinuousPageWidget::ContinuousPageWidget(QWidget *parent)
    : QWidget(parent)
{
    QSizePolicy sp(QSizePolicy::Preferred, QSizePolicy::Preferred);
    sp.setHeightForWidth(true);
    setSizePolicy(sp);
    setMouseTracking(true);
    initTheme(this);
}

void ContinuousPageWidget::applyTheme(const Theme &)
{
    update();
}

void ContinuousPageWidget::setRender(Render *r)
{
    render = r;
}

void ContinuousPageWidget::setNumPages(int count)
{
    numPages = count;
    defaultPageSize = QSize(800, 1200);
    pageSizes.fill(QSize(0, 0), count);
    relayout(false);
}

void ContinuousPageWidget::setZoomFactor(int zoom)
{
    if (zoomFactor == zoom) {
        return;
    }
    zoomFactor = zoom;
    relayout(true);
    update();
}

void ContinuousPageWidget::probeBufferedPages()
{
    if (!render || numPages == 0) {
        return;
    }

    bool changed = false;
    for (int i = 0; i < numPages; ++i) {
        const QImage *img = render->bufferedImage(i);
        bool hasKnownSize = pageSizes[i].width() > 0 && pageSizes[i].height() > 0;
        if (img && !img->isNull() && !hasKnownSize) {
            pageSizes[i] = img->size();
            if (defaultPageSize == QSize(800, 1200)) {
                defaultPageSize = img->size();
            }
            changed = true;
        }
    }

    if (changed) {
        relayout(true);
        update();
    }
}

void ContinuousPageWidget::reset()
{
    numPages = 0;
    pageSizes.clear();
    yPositions.clear();
    currentTotalHeight = 0;
    layoutSnapshot = LayoutSnapshot();
    defaultPageSize = QSize(800, 1200);
    setMinimumHeight(0);
    setMaximumHeight(QWIDGETSIZE_MAX);
    updateGeometry();
    update();
}

int ContinuousPageWidget::centerPage(int scrollY, int viewportHeight) const
{
    const int centerY = scrollY + std::max(0, viewportHeight / 2);
    return pageAtY(centerY);
}

int ContinuousPageWidget::yPositionForPage(int pageIndex) const
{
    if (pageIndex < 0 || pageIndex >= yPositions.size()) {
        return 0;
    }
    return yPositions[pageIndex];
}

int ContinuousPageWidget::totalHeight() const
{
    return currentTotalHeight;
}

bool ContinuousPageWidget::hasHeightForWidth() const
{
    return true;
}

int ContinuousPageWidget::heightForWidth(int w) const
{
    if (numPages == 0 || w <= 0) {
        return 0;
    }

    int h = 0;
    for (int i = 0; i < numPages; ++i) {
        QSize scaled = scaledPageSize(i, w);
        h += scaled.height();
    }
    return h;
}

QSize ContinuousPageWidget::sizeHint() const
{
    return QSize(defaultPageSize.width(), currentTotalHeight > 0 ? currentTotalHeight : 0);
}

void ContinuousPageWidget::onPageAvailable(int absolutePageIndex)
{
    if (!render || absolutePageIndex < 0 || absolutePageIndex >= numPages) {
        return;
    }

    const QImage *img = render->bufferedImage(absolutePageIndex);
    if (!img || img->isNull()) {
        return;
    }

    QSize naturalSize = img->size();

    // update default page size from the first real page we see
    if (defaultPageSize == QSize(800, 1200) && !naturalSize.isNull()) {
        defaultPageSize = naturalSize;
    }

    bool sizeChanged = (pageSizes[absolutePageIndex] != naturalSize);
    pageSizes[absolutePageIndex] = naturalSize;

    if (sizeChanged) {
        // keep anchor page visually stable while refined page sizes arrive
        relayout(true);
    }

    // repaint the region where this page lives
    if (absolutePageIndex < yPositions.size()) {
        QSize scaled = scaledPageSize(absolutePageIndex, width());
        QRect pageRect(0, yPositions[absolutePageIndex], scaled.width(), scaled.height());
        update(pageRect);
    }
}

void ContinuousPageWidget::paintEvent(QPaintEvent *event)
{
    if (numPages == 0 || !render) {
        return;
    }

    QPainter painter(this);

    QRect visibleRect = event->rect();
    int firstPage = pageAtY(visibleRect.top());
    int lastPage = pageAtY(visibleRect.bottom());

    int w = width();
    for (int i = firstPage; i <= lastPage && i < numPages; ++i) {
        int y = yPositions[i];
        QSize scaled = scaledPageSize(i, w);
        // center horizontally if page is narrower than widget
        int x = (w - scaled.width()) / 2;
        if (x < 0) {
            x = 0;
        }
        QRect pageRect(x, y, scaled.width(), scaled.height());

        const QImage *img = render->bufferedImage(i);
        if (img && !img->isNull()) {
            if (img->size() != scaled) {
                painter.drawImage(pageRect, img->scaled(scaled, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
            } else {
                painter.drawImage(pageRect, *img);
            }
        } else {
            // placeholder
            painter.fillRect(pageRect, QColor(45, 45, 45));
            painter.setPen(theme.viewer.defaultTextColor);
            painter.drawText(pageRect, Qt::AlignCenter, tr("Loading page %1").arg(i + 1));
        }
    }
}

void ContinuousPageWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    relayout(true);
}

void ContinuousPageWidget::setAnchorPage(int page)
{
    anchorPage = page;
}

void ContinuousPageWidget::setViewportState(int scrollY, int viewportHeight)
{
    viewportScrollY = std::max(0, scrollY);
    currentViewportHeight = std::max(0, viewportHeight);
    hasViewportState = true;
}

void ContinuousPageWidget::updateLayout()
{
    relayout(false);
}

void ContinuousPageWidget::updateLayoutWithAnchor()
{
    relayout(true);
}

void ContinuousPageWidget::relayout(bool preserveAnchor)
{
    int w = width();
    if (w <= 0) {
        w = parentWidget() ? parentWidget()->width() : 0;
    }
    if (w <= 0) {
        w = defaultPageSize.width();
    }

    const LayoutSnapshot oldSnapshot = layoutSnapshot;

    ViewportAnchor anchor;
    if (preserveAnchor && hasViewportState && !oldSnapshot.yPositions.isEmpty()) {
        anchor = anchorFromViewport(oldSnapshot, viewportScrollY, currentViewportHeight);
    } else if (preserveAnchor && anchorPage >= 0) {
        anchor.pageIndex = anchorPage;
        anchor.offsetRatio = 0.5f;
        anchor.valid = true;
    }

    layoutSnapshot = buildLayoutSnapshot(w);

#ifndef NDEBUG
    Q_ASSERT(layoutSnapshot.yPositions.size() == numPages);
    Q_ASSERT(layoutSnapshot.scaledSizes.size() == numPages);
    for (int i = 0; i < layoutSnapshot.scaledSizes.size(); ++i) {
        Q_ASSERT(layoutSnapshot.scaledSizes[i].width() > 0);
        Q_ASSERT(layoutSnapshot.scaledSizes[i].height() > 0);
        if (i > 0) {
            Q_ASSERT(layoutSnapshot.yPositions[i] >= layoutSnapshot.yPositions[i - 1]);
        }
    }
#endif

    yPositions = layoutSnapshot.yPositions;
    currentTotalHeight = layoutSnapshot.totalHeight;

    setFixedHeight(currentTotalHeight);
    updateGeometry();

    if (!preserveAnchor || !anchor.valid || currentViewportHeight <= 0) {
        return;
    }

    const int newScrollForAnchor = resolveAnchorToScrollY(layoutSnapshot, anchor, currentViewportHeight);
    emit layoutScrollPositionRequested(newScrollForAnchor);
}

ContinuousPageWidget::LayoutSnapshot ContinuousPageWidget::buildLayoutSnapshot(int w) const
{
    LayoutSnapshot snapshot;

    if (numPages <= 0 || w <= 0) {
        return snapshot;
    }

    snapshot.yPositions.resize(numPages);
    snapshot.scaledSizes.resize(numPages);

    qint64 y = 0;
    for (int i = 0; i < numPages; ++i) {
        snapshot.yPositions[i] = static_cast<int>(std::min<qint64>(y, std::numeric_limits<int>::max()));
        QSize scaled = scaledPageSize(i, w);
        scaled.setWidth(std::max(1, scaled.width()));
        scaled.setHeight(std::max(1, scaled.height()));
        snapshot.scaledSizes[i] = scaled;
        y += scaled.height();
    }

    snapshot.totalHeight = static_cast<int>(std::min<qint64>(y, static_cast<qint64>(QWIDGETSIZE_MAX)));
    return snapshot;
}

int ContinuousPageWidget::pageAtY(const LayoutSnapshot &snapshot, int y) const
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

ContinuousPageWidget::ViewportAnchor ContinuousPageWidget::anchorFromViewport(const LayoutSnapshot &snapshot, int scrollY, int viewportHeight) const
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

int ContinuousPageWidget::resolveAnchorToScrollY(const LayoutSnapshot &snapshot, const ViewportAnchor &anchor, int viewportHeight) const
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

int ContinuousPageWidget::pageAtY(int y) const
{
    return pageAtY(layoutSnapshot, y);
}

QSize ContinuousPageWidget::scaledPageSize(int pageIndex, int forWidth) const
{
    QSize natural = (pageIndex < pageSizes.size() && pageSizes[pageIndex].width() > 0 && pageSizes[pageIndex].height() > 0)
            ? pageSizes[pageIndex]
            : defaultPageSize;

    float scale = scaleForPage(pageIndex, forWidth);
    int scaledW = std::max(1, qRound(natural.width() * scale));
    int scaledH = std::max(1, qRound(natural.height() * scale));
    return QSize(scaledW, scaledH);
}

float ContinuousPageWidget::scaleForPage(int pageIndex, int forWidth) const
{
    QSize natural = (pageIndex < pageSizes.size() && pageSizes[pageIndex].width() > 0 && pageSizes[pageIndex].height() > 0)
            ? pageSizes[pageIndex]
            : defaultPageSize;

    if (natural.width() <= 0 || forWidth <= 0) {
        return 1.0f;
    }

    float baseScale = static_cast<float>(forWidth) / natural.width();
    float zoomMultiplier = zoomFactor / 100.0f;
    return baseScale * zoomMultiplier;
}
