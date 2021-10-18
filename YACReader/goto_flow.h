#ifndef __GOTO_FLOW_H
#define __GOTO_FLOW_H

#include "goto_flow_widget.h"
#include "yacreader_global_gui.h"

#include <QThread>

#include <QWaitCondition>
#include <QMutex>

#include <atomic>

class QLineEdit;
class QPushButton;
class QPixmap;
class QThread;
class QSize;
class QIntValidator;
class QWaitCondition;
class QEvent;
class QLabel;

class Comic;
class PageLoader;
class YACReaderFlow;
class PictureFlow;
class QKeyEvent;

class GoToFlow : public GoToFlowWidget
{
    Q_OBJECT
public:
    GoToFlow(QWidget *parent = nullptr, FlowType flowType = CoverFlowLike);
    ~GoToFlow() override;
    bool ready; // comic is ready for read.
private:
    YACReaderFlow *flow;
    void keyPressEvent(QKeyEvent *event) override;
    // Comic * comic;
    QSize imageSize;

    QVector<bool> imagesLoaded;
    QVector<bool> imagesSetted;
    int numImagesLoaded;
    QVector<bool> imagesReady;
    QVector<QByteArray> rawImages;
    QTimer *updateTimer;
    PageLoader *worker;
    void wheelEvent(QWheelEvent *event) override;
    QMutex mutexGoToFlow;

private slots:
    void preload();
    void updateImageData();
    void resizeEvent(QResizeEvent *event) override;

public slots:
    void centerSlide(int slide) override;
    void reset() override;
    void setNumSlides(unsigned int slides) override;
    void setImageReady(int index, const QByteArray &image) override;
    void setFlowType(FlowType flowType) override;
    void updateConfig(QSettings *settings) override;
    void setFlowRightToLeft(bool b) override;
};

//-----------------------------------------------------------------------------
// PageLoader
//-----------------------------------------------------------------------------
class PageLoader : public QThread
{
public:
    PageLoader(QMutex *m);
    ~PageLoader() override;
    // returns FALSE if worker is still busy and can't take the task
    bool busy() const;
    void generate(int index, QSize size, const QByteArray &rImage);
    void reset() { idx = -1; };
    int index() const { return idx; }
    QImage result() const { return img; }

protected:
    void run() override;

private:
    QMutex *mutex;
    QWaitCondition condition;

    bool restart;
    std::atomic<bool> working;
    int idx;

    QSize size;
    QImage img;
    QByteArray rawImage;
};

#endif
