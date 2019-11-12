#ifndef __COMICFLOW_H
#define __COMICFLOW_H

#include "yacreader_flow.h"

#include <QtCore>
#include <QImage>
#include <QString>
#include <QWheelEvent>

#include <memory>

template<typename Result>
class WorkerThread;

class ComicFlow : public YACReaderFlow
{
    Q_OBJECT
public:
    ComicFlow(QWidget *parent = nullptr, FlowType flowType = CoverFlowLike);
    ~ComicFlow() override;

    void setImagePaths(const QStringList &paths);
    //bool eventFilter(QObject *target, QEvent *event);
    void keyPressEvent(QKeyEvent *event) override;
    void removeSlide(int cover);
    void resortCovers(QList<int> newOrder);

private slots:
    void preload();
    void updateImageData();

private:
    void resetWorkerIndex() { workerIndex = -1; }

    QStringList imageFiles;
    QVector<bool> imagesLoaded;
    QVector<bool> imagesSetted;
    int numImagesLoaded;
    int workerIndex;
    QTimer *updateTimer;
    std::unique_ptr<WorkerThread<QImage>> worker;
    virtual void wheelEvent(QWheelEvent *event);
};

#endif
