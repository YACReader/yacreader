#ifndef CONTINUOUS_VIEW_MODEL_H
#define CONTINUOUS_VIEW_MODEL_H

#include <QObject>
#include <QSize>
#include <QVector>

class ContinuousViewModel : public QObject
{
    Q_OBJECT
public:
    explicit ContinuousViewModel(QObject *parent = nullptr);

    void reset();

    void setNumPages(int count);
    void setZoomFactor(int zoom);
    void setViewportSize(int width, int height);
    void setScrollYFromUser(int scrollY);
    void setAnchorPage(int page);
    void setCurrentPage(int page);
    void setPageNaturalSize(int pageIndex, const QSize &size);

    int numPages() const;
    int totalHeight() const;
    int scrollY() const;
    int viewportHeight() const;
    int zoomFactor() const;

    int centerPage() const;
    int yPositionForPage(int pageIndex) const;
    int pageAtY(int y) const;
    QSize scaledPageSize(int pageIndex) const;

signals:
    void stateChanged();

private:
    struct LayoutSnapshot {
        QVector<int> yPositions;
        QVector<QSize> scaledSizes;
        int totalHeight = 0;
    };

    struct ViewportAnchor {
        int pageIndex = -1;
        float offsetRatio = 0.0f;
        bool valid = false;
    };

    enum class RecomputePolicy {
        PreserveViewportAnchor,
        PreserveScrollClamped,
        ScrollToPageTop
    };

    void recompute(RecomputePolicy policy, int targetPage = -1);
    LayoutSnapshot buildLayoutSnapshot(int width) const;
    ViewportAnchor anchorFromViewport(const LayoutSnapshot &snapshot, int scrollY, int viewportHeight) const;
    int resolveAnchorToScrollY(const LayoutSnapshot &snapshot, const ViewportAnchor &anchor, int viewportHeight) const;
    int pageAtY(const LayoutSnapshot &snapshot, int y) const;
    int maxScrollFor(const LayoutSnapshot &snapshot) const;
    QSize scaledPageSizeForWidth(int pageIndex, int width) const;
    float scaleForPage(int pageIndex, int width) const;

    int numPagesValue = 0;
    QVector<QSize> pageSizes;
    QSize defaultPageSize { 800, 1200 };

    int zoomFactorValue = 100;
    int viewportWidth = 0;
    int viewportHeightValue = 0;
    int scrollYValue = 0;
    int anchorPage = -1;

    LayoutSnapshot layoutSnapshot;
};

#endif // CONTINUOUS_VIEW_MODEL_H
