#ifndef CONTINUOUS_PAGE_PROVIDER_H
#define CONTINUOUS_PAGE_PROVIDER_H

#include <QByteArray>
#include <QHash>
#include <QImage>
#include <QObject>
#include <QSet>
#include <QThreadPool>
#include <QVector>

class Render;

// Decodes comic pages off the GUI thread for continuous-scroll mode.
//
// This is the continuous-mode replacement for Render's synchronous, blocking
// page buffer. It pulls raw page bytes from Render (which still owns the comic
// and its loading), decodes/rotates/filters them on a background pool exactly
// the way Render's PageRender does, and hands finished full-resolution images
// back on the GUI thread. Nothing here blocks the GUI thread, so scrolling never
// stalls waiting on a decode.
//
// Render itself is left untouched; this object only *reads* from it
// (getRawPage / imageLoaded), so single-page mode is unaffected.
class ContinuousPageProvider : public QObject
{
    Q_OBJECT
public:
    explicit ContinuousPageProvider(QObject *parent = nullptr);
    ~ContinuousPageProvider() override;

    void setRender(Render *render);
    void setNumPages(int numPages);
    void reset();

    // Decode parameters, mirrored from Render so the output matches single-page
    // mode. Changing any of them invalidates everything already decoded.
    void setRotation(int degrees);
    void setFilters(int brightness, int contrast, int gamma);
    // Force re-decoding of everything (e.g. global image options changed).
    void invalidate();

    // Ensure pages in [firstPage, lastPage] are decoded (schedules the ones
    // whose raw bytes are ready) and evict pages well outside that window.
    void requestRange(int firstPage, int lastPage);

    // Full-resolution decoded image for the page, or nullptr if not ready.
    const QImage *image(int pageIndex) const;

    // Delivered from a background decode task on the GUI thread (queued).
    void deliverDecodedImage(int pageIndex, quint64 generation, QImage decoded);

signals:
    void pageReady(int pageIndex);

private slots:
    void onRawDataReady(int page);

private:
    void scheduleDecode(int pageIndex);
    void invalidateAll();

    Render *render = nullptr;
    int numPagesValue = 0;
    int rotation = 0;
    int brightnessLevel = -1;
    int contrastLevel = -1;
    int gammaLevel = -1;

    // Bumped on any change that invalidates decoded output; stale results are
    // discarded when they arrive.
    quint64 generation = 0;
    QHash<int, QImage> cache;
    QSet<int> pending;
    // Pages whose raw bytes have finished loading (signalled by imageLoaded).
    // We only read raw bytes (and decode) for pages in this set, so we never
    // race the comic's loader thread writing that slot. Populated from
    // construction onward — independent of view mode — so pages loaded while in
    // a fit mode are still known when the user later switches to continuous.
    QSet<int> rawDataReady;
    int requestedFirst = 0;
    int requestedLast = -1;

    QThreadPool decodePool;
};

#endif // CONTINUOUS_PAGE_PROVIDER_H
