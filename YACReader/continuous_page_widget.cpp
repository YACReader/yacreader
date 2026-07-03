#include "continuous_page_widget.h"

#include "configuration.h"
#include "continuous_page_provider.h"
#include "continuous_view_model.h"
#include "render.h"
#include "resize_image.h"

#include <QPaintEvent>
#include <QPainter>
#include <QRunnable>
#include <QThread>

#include <utility>

namespace {
// How many pages on either side of the visible range we keep scaled and
// pre-scale ahead of time, so the background pool stays ahead of the viewport.
constexpr int kPrefetchBehind = 1;
constexpr int kPrefetchAhead = 2;
}

// Scales one page off the GUI thread, then hands the result back to the widget
// through a queued invocation. The widget is the invocation context, so if it is
// destroyed before the result is delivered Qt drops the call; the widget's
// destructor waits for running tasks so the raw pointer stays valid during run().
class PageScaleTask : public QRunnable
{
public:
    PageScaleTask(ContinuousPageWidget *widget, int pageIndex, quint64 generation,
                  QImage source, QSize sourceSize, qint64 sourceCacheKey,
                  QSize targetSize, QSize targetPixelSize, qreal devicePixelRatio,
                  ScaleMethod method)
        : m_widget(widget),
          m_pageIndex(pageIndex),
          m_generation(generation),
          m_source(std::move(source)),
          m_sourceSize(sourceSize),
          m_sourceCacheKey(sourceCacheKey),
          m_targetSize(targetSize),
          m_targetPixelSize(targetPixelSize),
          m_devicePixelRatio(devicePixelRatio),
          m_method(method)
    {
        setAutoDelete(true);
    }

    void run() override
    {
        QImage scaled = scaleImage(m_source, m_targetPixelSize.width(), m_targetPixelSize.height(), m_method);
        if (!scaled.isNull()) {
            scaled.setDevicePixelRatio(m_devicePixelRatio);
        }

        ContinuousPageWidget *widget = m_widget;
        const int pageIndex = m_pageIndex;
        const quint64 generation = m_generation;
        const qint64 sourceCacheKey = m_sourceCacheKey;
        const QSize sourceSize = m_sourceSize;
        const QSize targetSize = m_targetSize;
        const QSize targetPixelSize = m_targetPixelSize;
        const qreal dpr = m_devicePixelRatio;

        QMetaObject::invokeMethod(
                widget,
                [widget, pageIndex, generation, scaled = std::move(scaled), sourceCacheKey, sourceSize, targetSize, targetPixelSize, dpr]() mutable {
                    widget->deliverScaledImage(pageIndex, generation, std::move(scaled), sourceCacheKey, sourceSize, targetSize, targetPixelSize, dpr);
                },
                Qt::QueuedConnection);
    }

private:
    ContinuousPageWidget *m_widget;
    int m_pageIndex;
    quint64 m_generation;
    QImage m_source;
    QSize m_sourceSize;
    qint64 m_sourceCacheKey;
    QSize m_targetSize;
    QSize m_targetPixelSize;
    qreal m_devicePixelRatio;
    ScaleMethod m_method;
};

ContinuousPageWidget::ContinuousPageWidget(QWidget *parent)
    : QWidget(parent)
{
    QSizePolicy sp(QSizePolicy::Preferred, QSizePolicy::Preferred);
    sp.setHeightForWidth(true);
    setSizePolicy(sp);
    setMouseTracking(true);
    initTheme(this);

    // Leave headroom for the page-decoding threads; scaling is heavy but should
    // not monopolize every core.
    scalePool.setMaxThreadCount(std::max(1, QThread::idealThreadCount() / 2));
}

ContinuousPageWidget::~ContinuousPageWidget()
{
    // Drop queued-but-not-started tasks and wait for running ones to finish.
    // While we block here the widget is still fully alive, so any in-flight
    // task's queued invocation targets a valid object; Qt removes that posted
    // event when this QObject is destroyed.
    scalePool.clear();
    scalePool.waitForDone();
}

void ContinuousPageWidget::applyTheme(const Theme &)
{
    update();
}

void ContinuousPageWidget::setRender(Render *r)
{
    render = r;
}

void ContinuousPageWidget::setProvider(ContinuousPageProvider *p)
{
    provider = p;
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
    resetScaledCache();
    update();
}

void ContinuousPageWidget::reset()
{
    setMinimumHeight(0);
    setMaximumHeight(QWIDGETSIZE_MAX);
    resetScaledCache();
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

void ContinuousPageWidget::invalidateScaledImageCache()
{
    resetScaledCache();
    update();
}

void ContinuousPageWidget::onPageAvailable(int absolutePageIndex)
{
    if (!provider || !continuousViewModel || absolutePageIndex < 0 || absolutePageIndex >= continuousViewModel->numPages()) {
        return;
    }

    const QImage *img = provider->image(absolutePageIndex);
    if (!img || img->isNull()) {
        return;
    }

    // The page's source pixels changed; drop the stale scaled image and any
    // in-flight scale so the next paint reschedules it from the new source.
    scaledPageCache.invalidatePage(absolutePageIndex);
    pendingScaleRequests.remove(absolutePageIndex);

    update(pageRectFor(absolutePageIndex));
}

void ContinuousPageWidget::paintEvent(QPaintEvent *event)
{
    if (!continuousViewModel || continuousViewModel->numPages() == 0 || !provider) {
        return;
    }

    QPainter painter(this);
    const qreal dpr = devicePixelRatioF();
    const int effectivePixelWidth = std::max(1, qRound(width() * dpr));

    // A width change makes every cached/in-flight scale obsolete. Bump the
    // generation so stale results that are already running get discarded.
    if (scaledPageCache.effectiveWidth != effectivePixelWidth) {
        scaledPageCache.invalidateForWidth(effectivePixelWidth);
        ++cacheGeneration;
        pendingScaleRequests.clear();
        scalePool.clear();
    }

    const int numPages = continuousViewModel->numPages();

    const QRect paintRect = event->rect();
    int firstPage = qBound(0, continuousViewModel->pageAtY(paintRect.top()), numPages - 1);
    int lastPage = qBound(0, continuousViewModel->pageAtY(paintRect.bottom()), numPages - 1);

    // Cache management and prefetch follow the real viewport, not this paint's
    // dirty rect (which can be a single page from an async delivery), so a
    // partial repaint never evicts a still-visible page.
    int prefetchMin = 0;
    int prefetchMax = -1;
    managedPageRange(prefetchMin, prefetchMax);
    scaledPageCache.keepOnlyRange(prefetchMin, prefetchMax);

    // Drive the background decoder to keep the visible + prefetch window ready.
    // This is the only place decoding is requested, and it never blocks.
    provider->requestRange(prefetchMin, prefetchMax);

    auto targetPixelSizeFor = [dpr](const QSize &scaled) {
        return QSize(std::max(1, qRound(scaled.width() * dpr)),
                     std::max(1, qRound(scaled.height() * dpr)));
    };

    const int w = width();
    for (int i = firstPage; i <= lastPage && i < numPages; ++i) {
        const int y = continuousViewModel->yPositionForPage(i);
        const QSize scaled = continuousViewModel->scaledPageSize(i);
        // center horizontally if page is narrower than widget
        int x = (w - scaled.width()) / 2;
        if (x < 0) {
            x = 0;
        }
        const QRect pageRect(x, y, scaled.width(), scaled.height());

        const QImage *img = provider->image(i);
        if (img && !img->isNull()) {
            const QSize targetPixelSize = targetPixelSizeFor(scaled);
            const QImage *drawable = cachedScaledImage(i, img, scaled, targetPixelSize, dpr);
            if (drawable) {
                painter.drawImage(pageRect, *drawable);
            } else {
                // Not scaled yet: schedule the high-quality scale off-thread and
                // draw the source cheaply (fast transform) meanwhile, so the
                // page is visible immediately instead of flashing out.
                enqueueScale(i, img, scaled, targetPixelSize, dpr);
                painter.setRenderHint(QPainter::SmoothPixmapTransform, false);
                painter.drawImage(pageRect, *img);
            }
        } else {
            // placeholder
            painter.fillRect(pageRect, QColor(45, 45, 45));
            painter.setPen(theme.viewer.defaultTextColor);
            painter.drawText(pageRect, Qt::AlignCenter, tr("Loading page %1").arg(i + 1));
        }
    }

    // Pre-scale the pages just outside the viewport so they are ready by the
    // time they scroll in (paint only ever reads the cache, never scales).
    for (int i = prefetchMin; i <= prefetchMax; ++i) {
        if (i >= firstPage && i <= lastPage) {
            continue;
        }
        const QImage *img = provider->image(i);
        if (!img || img->isNull()) {
            continue;
        }
        const QSize scaled = continuousViewModel->scaledPageSize(i);
        if (scaled.isEmpty()) {
            continue;
        }
        const QSize targetPixelSize = targetPixelSizeFor(scaled);
        if (!cachedScaledImage(i, img, scaled, targetPixelSize, dpr)) {
            enqueueScale(i, img, scaled, targetPixelSize, dpr);
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

const QImage *ContinuousPageWidget::cachedScaledImage(int pageIndex, const QImage *source, const QSize &targetSize, const QSize &targetPixelSize, qreal dpr) const
{
    if (!source || source->isNull() || targetPixelSize.isEmpty()) {
        return nullptr;
    }

    auto it = scaledPageCache.pages.constFind(pageIndex);
    if (it == scaledPageCache.pages.constEnd()) {
        return nullptr;
    }

    const ScaledPageCacheEntry &entry = it.value();
    const bool validEntry = entry.sourceCacheKey == source->cacheKey() && entry.sourceSize == source->size() && entry.targetSize == targetSize && entry.targetPixelSize == targetPixelSize && qFuzzyCompare(entry.targetDevicePixelRatio, dpr) && !entry.scaledImage.isNull();

    return validEntry ? &entry.scaledImage : nullptr;
}

void ContinuousPageWidget::enqueueScale(int pageIndex, const QImage *source, const QSize &targetSize, const QSize &targetPixelSize, qreal dpr)
{
    if (!source || source->isNull() || targetPixelSize.isEmpty()) {
        return;
    }

    ScaleRequestKey key;
    key.sourceCacheKey = source->cacheKey();
    key.targetPixelSize = targetPixelSize;
    key.devicePixelRatio = dpr;
    key.generation = cacheGeneration;

    auto it = pendingScaleRequests.constFind(pageIndex);
    if (it != pendingScaleRequests.constEnd() && it.value() == key) {
        return; // identical scale already in flight
    }

    pendingScaleRequests.insert(pageIndex, key);

    // *source copies the QImage (implicitly shared, copy-on-write), so the task
    // reads it on the worker thread without racing the render buffer.
    auto *task = new PageScaleTask(this, pageIndex, cacheGeneration, *source, source->size(),
                                   source->cacheKey(), targetSize, targetPixelSize, dpr,
                                   Configuration::getConfiguration().getScalingMethod());
    scalePool.start(task);
}

void ContinuousPageWidget::deliverScaledImage(int pageIndex, quint64 generation, QImage scaledImage,
                                              qint64 sourceCacheKey, QSize sourceSize, QSize targetSize,
                                              QSize targetPixelSize, qreal devicePixelRatio)
{
    ScaleRequestKey deliveredKey;
    deliveredKey.sourceCacheKey = sourceCacheKey;
    deliveredKey.targetPixelSize = targetPixelSize;
    deliveredKey.devicePixelRatio = devicePixelRatio;
    deliveredKey.generation = generation;

    // Only consume this result if it still matches the page's pending request.
    // A stale task (older source / superseded request) is dropped without
    // clearing the newer pending marker.
    auto it = pendingScaleRequests.find(pageIndex);
    if (it == pendingScaleRequests.end() || !(it.value() == deliveredKey)) {
        return;
    }

    pendingScaleRequests.erase(it);

    if (generation != cacheGeneration || scaledImage.isNull()) {
        return;
    }

    ScaledPageCacheEntry entry;
    entry.sourceCacheKey = sourceCacheKey;
    entry.sourceSize = sourceSize;
    entry.targetSize = targetSize;
    entry.targetPixelSize = targetPixelSize;
    entry.targetDevicePixelRatio = devicePixelRatio;
    entry.scaledImage = std::move(scaledImage);
    scaledPageCache.pages.insert(pageIndex, std::move(entry));

    update(pageRectFor(pageIndex));
}

void ContinuousPageWidget::resetScaledCache()
{
    scaledPageCache.invalidateAll();
    ++cacheGeneration;
    pendingScaleRequests.clear();
    scalePool.clear();
}

QRect ContinuousPageWidget::pageRectFor(int pageIndex) const
{
    if (!continuousViewModel || pageIndex < 0 || pageIndex >= continuousViewModel->numPages()) {
        return QRect();
    }

    const QSize scaled = continuousViewModel->scaledPageSize(pageIndex);
    const int y = continuousViewModel->yPositionForPage(pageIndex);
    int x = (width() - scaled.width()) / 2;
    if (x < 0) {
        x = 0;
    }
    return QRect(x, y, scaled.width(), scaled.height());
}

void ContinuousPageWidget::managedPageRange(int &minPageIndex, int &maxPageIndex) const
{
    minPageIndex = 0;
    maxPageIndex = -1;

    if (!continuousViewModel || continuousViewModel->numPages() <= 0) {
        return;
    }

    const int numPages = continuousViewModel->numPages();
    const int viewportHeight = continuousViewModel->viewportHeight() > 0 ? continuousViewModel->viewportHeight() : height();
    const int visibleTop = continuousViewModel->scrollY();
    const int visibleBottom = visibleTop + std::max(0, viewportHeight - 1);

    const int firstVisiblePage = qBound(0, continuousViewModel->pageAtY(visibleTop), numPages - 1);
    const int lastVisiblePage = qBound(0, continuousViewModel->pageAtY(visibleBottom), numPages - 1);

    minPageIndex = std::max(0, firstVisiblePage - kPrefetchBehind);
    maxPageIndex = std::min(numPages - 1, lastVisiblePage + kPrefetchAhead);
}
