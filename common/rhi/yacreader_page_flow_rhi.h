#ifndef __YACREADER_PAGE_FLOW_RHI_H
#define __YACREADER_PAGE_FLOW_RHI_H

#include "yacreader_flow_rhi.h"

#include <QMutex>
#include <QWaitCondition>

class ImageLoaderByteArray3D;

class YACReaderPageFlow3D : public YACReaderFlow3D
{
public:
    YACReaderPageFlow3D(QWidget *parent = nullptr, struct Preset p = defaultYACReaderFlowConfig);
    ~YACReaderPageFlow3D();
    void updateImageData() override;
    void populate(int n);
    QVector<bool> imagesReady;
    QVector<QByteArray> rawImages;
    QVector<bool> imagesSetted;
    friend class ImageLoaderByteArray3D;

private:
    ImageLoaderByteArray3D *worker;
};

class ImageLoaderByteArray3D : public QThread
{
public:
    ImageLoaderByteArray3D(YACReaderFlow3D *flow);
    ~ImageLoaderByteArray3D();
    bool busy() const;
    void generate(int index, const QByteArray &raw);
    void reset()
    {
        idx = -1;
        rawData.clear();
    }
    int index() const { return idx; }
    QImage result();
    YACReaderFlow3D *flow;
    QImage loadImage(const QByteArray &rawData);

protected:
    void run() override;

private:
    QMutex mutex;
    QWaitCondition condition;

    bool restart;
    bool working;
    int idx;
    QByteArray rawData;
    QImage img;
};

#endif // __YACREADER_PAGE_FLOW_RHI_H
