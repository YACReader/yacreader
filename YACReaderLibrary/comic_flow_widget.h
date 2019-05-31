#ifndef __COMIC_FLOW_WIDGET_H
#define __COMIC_FLOW_WIDGET_H

#include <QWidget>

#include "pictureflow.h"
#include "comic_flow.h"
#ifndef NO_OPENGL
#include "yacreader_flow_gl.h"
#endif
class ComicFlowWidget : public QWidget
{
    Q_OBJECT
public:
    ComicFlowWidget(QWidget *paret = nullptr);

public slots:
    virtual void setShowMarks(bool value) = 0;
    virtual void setMarks(QVector<YACReaderComicReadStatus> marks) = 0;
    virtual void setMarkImage(QImage &image) = 0;
    virtual void markSlide(int index, YACReaderComicReadStatus status) = 0;
    virtual void unmarkSlide(int index) = 0;
    virtual void setSlideSize(QSize size) = 0;
    virtual void clear() = 0;
    virtual void setImagePaths(QStringList paths) = 0;
    virtual void setCenterIndex(int index) = 0;
    virtual void showSlide(int index) = 0;
    virtual int centerIndex() = 0;
    virtual void updateMarks() = 0;
    virtual void setFlowType(FlowType flowType) = 0;
    virtual void render() = 0;
    virtual void updateConfig(QSettings *settings) = 0;
    virtual void remove(int cover) = 0;
    virtual void resortCovers(QList<int> newOrder) = 0;
signals:
    void centerIndexChanged(int);
    void selected(unsigned int);
};

class ComicFlowWidgetSW : public ComicFlowWidget
{
    Q_OBJECT
private:
    ComicFlow *flow;

public:
    ComicFlowWidgetSW(QWidget *parent = nullptr);

    void setShowMarks(bool value);
    void setMarks(QVector<YACReaderComicReadStatus> marks);
    void setMarkImage(QImage &image);
    void markSlide(int index, YACReaderComicReadStatus status);
    void unmarkSlide(int index);
    void setSlideSize(QSize size);
    void clear();
    void setImagePaths(QStringList paths);
    void setCenterIndex(int index);
    void showSlide(int index);
    int centerIndex();
    void updateMarks();
    void setFlowType(FlowType flowType);
    void render();
    void updateConfig(QSettings *settings);
    void remove(int cover);
    void resortCovers(QList<int> newOrder);

protected:
    void keyPressEvent(QKeyEvent *event);
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void resizeEvent(QResizeEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    QSize minimumSizeHint() const;
    QSize sizeHint() const;
    QSize slideSizeW;
    QSize slideSizeF;
};

#ifndef NO_OPENGL
class ComicFlowWidgetGL : public ComicFlowWidget
{
    Q_OBJECT
private:
    YACReaderComicFlowGL *flow;

public:
    ComicFlowWidgetGL(QWidget *parent = nullptr);

    void setShowMarks(bool value);
    void setMarks(QVector<YACReaderComicReadStatus> marks);
    void setMarkImage(QImage &image);
    void markSlide(int index, YACReaderComicReadStatus status);
    void unmarkSlide(int index);
    void setSlideSize(QSize size);
    void clear();
    void setImagePaths(QStringList paths);
    void setCenterIndex(int index);
    void showSlide(int index);
    int centerIndex();
    void updateMarks();
    void setFlowType(FlowType flowType);
    void render();
    void updateConfig(QSettings *settings);
    void remove(int cover);
    void resortCovers(QList<int> newOrder);
    //public slots:
    //	void setCF_RX(int value);
    //	//the Y Rotation of the Coverflow
    //	void setCF_RY(int value);
    //	//the Z Rotation of the Coverflow
    //	void setCF_RZ(int value);
    //	//perspective
    //	void setZoom(int zoom);
    //	void setRotation(int angle);
    //	//sets the distance between the covers
    //	void setX_Distance(int distance);
    //	//sets the distance between the centered and the non centered covers
    //	void setCenter_Distance(int distance);
    //	//sets the pushback amount
    //	void setZ_Distance(int distance);
    //	void setCF_Y(int value);
    //	void setY_Distance(int value);
    //	void setPreset(const Preset & p);
protected:
    void keyPressEvent(QKeyEvent *event);
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void resizeEvent(QResizeEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    QSize minimumSizeHint() const;
    QSize sizeHint() const;
};
#endif
#endif
