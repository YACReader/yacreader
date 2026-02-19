#ifndef __COMIC_FLOW_WIDGET_H
#define __COMIC_FLOW_WIDGET_H

#include <QWidget>

#include "yacreader_global.h"

#include "yacreader_comic_flow_rhi.h"
#include "yacreader_global_gui.h"

class ComicFlowWidget : public QWidget
{
    Q_OBJECT
public:
    ComicFlowWidget(QWidget *parent = nullptr);

public slots:
    void setShowMarks(bool value);
    void setMarks(QVector<YACReader::YACReaderComicReadStatus> marks);
    void setMarkImage(QImage &image);
    void markSlide(int index, YACReader::YACReaderComicReadStatus status);
    void unmarkSlide(int index);
    void setSlideSize(QSize size);
    void clear();
    void setImagePaths(QStringList paths);
    void setCenterIndex(int index);
    void showSlide(int index);
    int centerIndex();
    void updateMarks();
    void setFlowType(YACReader::FlowType flowType);
    void render();
    void updateConfig(QSettings *settings);
    void add(const QString &path, int index);
    void remove(int cover);
    void resortCovers(QList<int> newOrder);

signals:
    void centerIndexChanged(int);
    void selected(unsigned int);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

private:
    YACReaderComicFlow3D *flow;
};

#endif
