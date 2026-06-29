#include "continuous_page_provider.h"

#include "render.h"

#include <QRunnable>
#include <QThread>
#include <QTransform>

#include <utility>

namespace {
// Keep a few pages beyond the requested window cached so a small scroll back
// does not force an immediate re-decode.
constexpr int kEvictMargin = 2;
}

// Decodes one page off the GUI thread (load + rotate + filters, matching
// PageRender), then hands the result to the provider through a queued
// invocation. The provider is the invocation context, so a result for a
// destroyed provider is dropped; the provider's destructor waits for running
// tasks, keeping the raw pointer valid for the duration of run().
class PageDecodeTask : public QRunnable
{
public:
    PageDecodeTask(ContinuousPageProvider *provider, int pageIndex, quint64 generation,
                   QByteArray rawData, int rotation, int brightness, int contrast, int gamma)
        : m_provider(provider),
          m_pageIndex(pageIndex),
          m_generation(generation),
          m_rawData(std::move(rawData)),
          m_rotation(rotation),
          m_brightness(brightness),
          m_contrast(contrast),
          m_gamma(gamma)
    {
        setAutoDelete(true);
    }

    void run() override
    {
        QImage img;
        img.loadFromData(m_rawData);

        if (!img.isNull() && m_rotation > 0) {
            QTransform t;
            t.rotate(m_rotation);
            img = img.transformed(t, Qt::SmoothTransformation);
        }

        if (!img.isNull()) {
            // Same filters, same order as Render: brightness, contrast, gamma.
            BrightnessFilter(m_brightness).setFilter(img).swap(img);
            ContrastFilter(m_contrast).setFilter(img).swap(img);
            GammaFilter(m_gamma).setFilter(img).swap(img);
        }

        ContinuousPageProvider *provider = m_provider;
        const int pageIndex = m_pageIndex;
        const quint64 generation = m_generation;

        QMetaObject::invokeMethod(
                provider,
                [provider, pageIndex, generation, img = std::move(img)]() mutable {
                    provider->deliverDecodedImage(pageIndex, generation, std::move(img));
                },
                Qt::QueuedConnection);
    }

private:
    ContinuousPageProvider *m_provider;
    int m_pageIndex;
    quint64 m_generation;
    QByteArray m_rawData;
    int m_rotation;
    int m_brightness;
    int m_contrast;
    int m_gamma;
};

ContinuousPageProvider::ContinuousPageProvider(QObject *parent)
    : QObject(parent)
{
    // Leave headroom for the scaling pool and the rest of the app.
    decodePool.setMaxThreadCount(std::max(1, QThread::idealThreadCount() / 2));
}

ContinuousPageProvider::~ContinuousPageProvider()
{
    decodePool.clear();
    decodePool.waitForDone();
}

void ContinuousPageProvider::setRender(Render *r)
{
    if (render == r) {
        return;
    }

    if (render) {
        disconnect(render, QOverload<int>::of(&Render::imageLoaded), this, &ContinuousPageProvider::onRawDataReady);
    }

    render = r;

    if (render) {
        connect(render, QOverload<int>::of(&Render::imageLoaded), this, &ContinuousPageProvider::onRawDataReady);
    }

    invalidateAll();
}

void ContinuousPageProvider::setNumPages(int numPages)
{
    numPagesValue = std::max(0, numPages);
    invalidateAll();
}

void ContinuousPageProvider::reset()
{
    numPagesValue = 0;
    requestedFirst = 0;
    requestedLast = -1;
    rawDataReady.clear();
    invalidateAll();
}

void ContinuousPageProvider::setRotation(int degrees)
{
    int normalized = degrees % 360;
    if (normalized < 0) {
        normalized += 360;
    }
    if (rotation == normalized) {
        return;
    }
    rotation = normalized;
    invalidateAll();
}

void ContinuousPageProvider::setFilters(int brightness, int contrast, int gamma)
{
    if (brightnessLevel == brightness && contrastLevel == contrast && gammaLevel == gamma) {
        return;
    }
    brightnessLevel = brightness;
    contrastLevel = contrast;
    gammaLevel = gamma;
    invalidateAll();
}

void ContinuousPageProvider::invalidate()
{
    invalidateAll();
}

void ContinuousPageProvider::requestRange(int firstPage, int lastPage)
{
    if (!render || numPagesValue <= 0) {
        return;
    }

    firstPage = qBound(0, firstPage, numPagesValue - 1);
    lastPage = qBound(0, lastPage, numPagesValue - 1);
    if (lastPage < firstPage) {
        return;
    }

    requestedFirst = firstPage;
    requestedLast = lastPage;

    const int keepMin = std::max(0, firstPage - kEvictMargin);
    const int keepMax = std::min(numPagesValue - 1, lastPage + kEvictMargin);

    // Evict cached pages outside the kept window.
    for (auto it = cache.begin(); it != cache.end();) {
        if (it.key() < keepMin || it.key() > keepMax) {
            it = cache.erase(it);
        } else {
            ++it;
        }
    }

    // Schedule decode for requested pages whose raw bytes are loaded and that
    // are not already decoded or in flight.
    for (int i = firstPage; i <= lastPage; ++i) {
        if (rawDataReady.contains(i) && !cache.contains(i) && !pending.contains(i)) {
            scheduleDecode(i);
        }
    }
}

const QImage *ContinuousPageProvider::image(int pageIndex) const
{
    auto it = cache.constFind(pageIndex);
    if (it == cache.constEnd() || it.value().isNull()) {
        return nullptr;
    }
    return &it.value();
}

void ContinuousPageProvider::onRawDataReady(int page)
{
    if (page >= 0) {
        rawDataReady.insert(page);
    }

    if (page < requestedFirst || page > requestedLast) {
        return;
    }
    if (cache.contains(page) || pending.contains(page)) {
        return;
    }
    scheduleDecode(page);
}

void ContinuousPageProvider::scheduleDecode(int pageIndex)
{
    if (!render || pageIndex < 0 || pageIndex >= numPagesValue) {
        return;
    }

    const QByteArray rawData = render->getRawPage(pageIndex);
    if (rawData.isEmpty()) {
        return; // raw bytes not loaded yet; onRawDataReady will retrigger us
    }

    pending.insert(pageIndex);

    auto *task = new PageDecodeTask(this, pageIndex, generation, rawData,
                                    rotation, brightnessLevel, contrastLevel, gammaLevel);
    decodePool.start(task);
}

void ContinuousPageProvider::deliverDecodedImage(int pageIndex, quint64 generationValue, QImage decoded)
{
    pending.remove(pageIndex);

    if (generationValue != generation || decoded.isNull()) {
        return;
    }

    // Ignore results that fell outside the current window while decoding.
    if (pageIndex < std::max(0, requestedFirst - kEvictMargin) || pageIndex > requestedLast + kEvictMargin) {
        return;
    }

    cache.insert(pageIndex, std::move(decoded));
    emit pageReady(pageIndex);
}

void ContinuousPageProvider::invalidateAll()
{
    ++generation;
    cache.clear();
    pending.clear();
    decodePool.clear();
}
