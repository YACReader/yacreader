#ifndef __YACREADER_COMIC_FLOW_RHI_H
#define __YACREADER_COMIC_FLOW_RHI_H

#include "yacreader_flow_rhi.h"

class ImageLoader3D;

class YACReaderComicFlow3D : public YACReaderFlow3D
{
public:
    YACReaderComicFlow3D(QWidget *parent = nullptr, struct Preset p = defaultYACReaderFlowConfig);
    void setImagePaths(QStringList paths);
    void updateImageData() override;
    void remove(int item) override;
    void add(const QString &path, int index);
    void resortCovers(QList<int> newOrder);
    friend class ImageLoader3D;

private:
    ImageLoader3D *worker;

protected:
    QList<QString> paths;
};

class ImageLoader3D : public QThread
{
public:
    ImageLoader3D(YACReaderFlow3D *flow);
    ~ImageLoader3D();
    bool busy() const;
    void generate(int index, const QString &fileName);
    void reset()
    {
        idx = -1;
        fileName = "";
    }
    int index() const { return idx; }
    void lock();
    void unlock();
    QImage result();
    YACReaderFlow3D *flow;
    QImage loadImage(const QString &fileName);

protected:
    void run() override;

private:
    QMutex mutex;
    QWaitCondition condition;

    bool restart;
    bool working;
    int idx;
    QString fileName;
    QImage img;
};

#endif // __YACREADER_COMIC_FLOW_RHI_H
