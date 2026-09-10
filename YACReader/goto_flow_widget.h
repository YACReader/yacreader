#ifndef __GOTO_FLOW_WIDGET_H
#define __GOTO_FLOW_WIDGET_H

#include "themable.h"
#include "yacreader_global_gui.h"
#include "yacreader_page_flow_rhi.h"

#include <QQueue>
#include <QSet>
#include <QSettings>
#include <QVector>
#include <QWidget>

using namespace YACReader;

class QSettings;
class GoToFlowToolBar;
class QVBoxLayout;
class QKeyEvent;
class QEvent;
class QHideEvent;
class QListWidget;
class QShowEvent;

class GoToFlowWidget : public QWidget, protected Themable
{
    Q_OBJECT

public:
    GoToFlowWidget(QWidget *parent = nullptr, FlowType flowType = CoverFlowLike);
    ~GoToFlowWidget() override;

public slots:
    void reset();
    void centerSlide(int slide);
    void setPageNumber(int page);
    void setFlowType(FlowType flowType);
    void setNumSlides(unsigned int slides);
    void setImageReady(int index, const QByteArray &image);
    void updateSize();
    void updateConfig(QSettings *settings);
    void setFlowRightToLeft(bool b);

signals:
    void goToPage(unsigned int);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void applyTheme(const Theme &theme) override;

private:
    void updateSoftwareFlowMetrics();
    void updateSoftwareFlowEdgeSpacing();
    void updateSoftwareThumbnail(int index);
    void updateSoftwareThumbnailWindow();
    void enqueueSoftwareThumbnail(int index);
    void processNextSoftwareThumbnail();
    void clearSoftwareThumbnailQueue();

    QVBoxLayout *mainLayout;
    GoToFlowToolBar *toolBar;
    YACReaderPageFlow3D *rhiFlow = nullptr;
    QListWidget *softwareFlow = nullptr;
    QVector<QByteArray> softwareImages;
    QSize imageSize;
    bool softwareRendering = false;
    bool rightToLeft = false;
    int softwareCurrentPage = -1;
    int wheelDeltaAccumulator = 0;
    int softwareThumbnailFirst = -1;
    int softwareThumbnailLast = -1;
    bool softwareThumbnailDecodeScheduled = false;
    QQueue<int> softwareThumbnailQueue;
    QSet<int> softwareQueuedThumbnails;
    QSet<int> softwareDecodedThumbnails;
};

#endif
