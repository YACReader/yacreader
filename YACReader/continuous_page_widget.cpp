#include "continuous_page_widget.h"
#include "continuous_view_model.h"
#include "render.h"

#include <QPainter>
#include <QPaintEvent>

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

void ContinuousPageWidget::setViewModel(ContinuousViewModel *viewModel)
{
    if (continuousViewModel == viewModel) {
        return;
    }

    if (continuousViewModel) {
        disconnect(continuousViewModel, &ContinuousViewModel::stateChanged, this, QOverload<>::of(&ContinuousPageWidget::update));
    }

    continuousViewModel = viewModel;

    if (continuousViewModel) {
        connect(continuousViewModel, &ContinuousViewModel::stateChanged, this, QOverload<>::of(&ContinuousPageWidget::update));
    }

    updateGeometry();
    scaledPageCache.invalidateAll();
    update();
}

void ContinuousPageWidget::reset()
{
    setMinimumHeight(0);
    setMaximumHeight(QWIDGETSIZE_MAX);
    scaledPageCache.invalidateAll();
    updateGeometry();
    update();
}

bool ContinuousPageWidget::hasHeightForWidth() const
{
    return true;
}

int ContinuousPageWidget::heightForWidth(int w) const
{
    if (!continuousViewModel || w <= 0) {
        return 0;
    }
    Q_UNUSED(w)
    return continuousViewModel->totalHeight();
}

QSize ContinuousPageWidget::sizeHint() const
{
    if (!continuousViewModel) {
        return QSize(800, 0);
    }
    return QSize(width(), continuousViewModel->totalHeight());
}

void ContinuousPageWidget::onPageAvailable(int absolutePageIndex)
{
    if (!render || !continuousViewModel || absolutePageIndex < 0 || absolutePageIndex >= continuousViewModel->numPages()) {
        return;
    }

    const QImage *img = render->bufferedImage(absolutePageIndex);
    if (!img || img->isNull()) {
        return;
    }

    scaledPageCache.invalidatePage(absolutePageIndex);

    // repaint the region where this page lives
    if (absolutePageIndex < continuousViewModel->numPages()) {
        QSize scaled = continuousViewModel->scaledPageSize(absolutePageIndex);
        const int y = continuousViewModel->yPositionForPage(absolutePageIndex);
        int x = (width() - scaled.width()) / 2;
        if (x < 0) {
            x = 0;
        }
        QRect pageRect(x, y, scaled.width(), scaled.height());
        update(pageRect);
    }
}

void ContinuousPageWidget::paintEvent(QPaintEvent *event)
{
    if (!continuousViewModel || continuousViewModel->numPages() == 0 || !render) {
        return;
    }

    QPainter painter(this);
    scaledPageCache.invalidateForWidth(width());

    QRect visibleRect = event->rect();
    int firstPage = continuousViewModel->pageAtY(visibleRect.top());
    int lastPage = continuousViewModel->pageAtY(visibleRect.bottom());
    firstPage = qBound(0, firstPage, continuousViewModel->numPages() - 1);
    lastPage = qBound(0, lastPage, continuousViewModel->numPages() - 1);

    const int cacheMin = std::max(0, firstPage - 1);
    const int cacheMax = std::min(continuousViewModel->numPages() - 1, lastPage + 1);
    scaledPageCache.keepOnlyRange(cacheMin, cacheMax);

    int w = width();
    for (int i = firstPage; i <= lastPage && i < continuousViewModel->numPages(); ++i) {
        int y = continuousViewModel->yPositionForPage(i);
        QSize scaled = continuousViewModel->scaledPageSize(i);
        // center horizontally if page is narrower than widget
        int x = (w - scaled.width()) / 2;
        if (x < 0) {
            x = 0;
        }
        QRect pageRect(x, y, scaled.width(), scaled.height());

        const QImage *img = render->bufferedImage(i);
        if (img && !img->isNull()) {
            const QImage *drawable = scaledImageForPaint(i, img, scaled, width());
            if (drawable) {
                painter.drawImage(pageRect, *drawable);
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
    if (continuousViewModel) {
        continuousViewModel->setViewportSize(width(), continuousViewModel->viewportHeight());
    }
}

const QImage *ContinuousPageWidget::scaledImageForPaint(int pageIndex, const QImage *source, const QSize &targetSize, int effectiveWidth)
{
    if (!source || source->isNull() || targetSize.isEmpty()) {
        return nullptr;
    }

    if (source->size() == targetSize) {
        return source;
    }

    scaledPageCache.invalidateForWidth(effectiveWidth);

    auto it = scaledPageCache.pages.find(pageIndex);
    const qint64 sourceKey = source->cacheKey();

    if (it != scaledPageCache.pages.end()) {
        const ScaledPageCacheEntry &entry = it.value();
        const bool validEntry = entry.sourceCacheKey == sourceKey
                && entry.sourceSize == source->size()
                && entry.targetSize == targetSize
                && !entry.scaledImage.isNull();
        if (validEntry) {
            return &it.value().scaledImage;
        }
    }

    ScaledPageCacheEntry entry;
    entry.sourceCacheKey = sourceKey;
    entry.sourceSize = source->size();
    entry.targetSize = targetSize;
    entry.scaledImage = source->scaled(targetSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    scaledPageCache.pages.insert(pageIndex, std::move(entry));

    return &scaledPageCache.pages[pageIndex].scaledImage;
}
