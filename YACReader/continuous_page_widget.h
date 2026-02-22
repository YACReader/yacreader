#ifndef CONTINUOUS_PAGE_WIDGET_H
#define CONTINUOUS_PAGE_WIDGET_H

#include <QWidget>
#include <QSize>
#include <QVector>

#include "themable.h"

class Render;

class ContinuousPageWidget : public QWidget, protected Themable
{
    Q_OBJECT
public:
    explicit ContinuousPageWidget(QWidget *parent = nullptr);

    void setRender(Render *r);
    void setNumPages(int count);
    void setZoomFactor(int zoom);
    void probeBufferedPages();
    void reset();

    int centerPage(int scrollY, int viewportHeight) const;
    int yPositionForPage(int pageIndex) const;
    int totalHeight() const;

    bool hasHeightForWidth() const override;
    int heightForWidth(int w) const override;
    QSize sizeHint() const override;

    void setAnchorPage(int page);
    void setViewportState(int scrollY, int viewportHeight);

signals:
    // emitted after layout recomputation when the preserved viewport anchor
    // resolves to an absolute scroll position
    void layoutScrollPositionRequested(int scrollY);

public slots:
    void onPageAvailable(int absolutePageIndex);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void applyTheme(const Theme &theme) override;

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

    void updateLayout();
    void updateLayoutWithAnchor();
    void relayout(bool preserveAnchor);
    LayoutSnapshot buildLayoutSnapshot(int w) const;
    int pageAtY(const LayoutSnapshot &snapshot, int y) const;
    ViewportAnchor anchorFromViewport(const LayoutSnapshot &snapshot, int scrollY, int viewportHeight) const;
    int resolveAnchorToScrollY(const LayoutSnapshot &snapshot, const ViewportAnchor &anchor, int viewportHeight) const;
    int pageAtY(int y) const;
    QSize scaledPageSize(int pageIndex, int forWidth) const;
    float scaleForPage(int pageIndex, int forWidth) const;

    Render *render = nullptr;
    int numPages = 0;
    QVector<QSize> pageSizes;
    QVector<int> yPositions;
    int currentTotalHeight = 0;
    LayoutSnapshot layoutSnapshot;
    QSize defaultPageSize { 800, 1200 };
    int zoomFactor = 100;
    int anchorPage = -1;
    int viewportScrollY = 0;
    int currentViewportHeight = 0;
    bool hasViewportState = false;
};

#endif // CONTINUOUS_PAGE_WIDGET_H
