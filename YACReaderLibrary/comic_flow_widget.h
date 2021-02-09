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
    explicit ComicFlowWidget(QWidget *paret = nullptr);

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
    explicit ComicFlowWidgetSW(QWidget *parent = nullptr);

    void setShowMarks(bool value) override;
    void setMarks(QVector<YACReaderComicReadStatus> marks) override;
    void setMarkImage(QImage &image) override;
    void markSlide(int index, YACReaderComicReadStatus status) override;
    void unmarkSlide(int index) override;
    void setSlideSize(QSize size) override;
    void clear() override;
    void setImagePaths(QStringList paths) override;
    void setCenterIndex(int index) override;
    void showSlide(int index) override;
    int centerIndex() override;
    void updateMarks() override;
    void setFlowType(FlowType flowType) override;
    void render() override;
    void updateConfig(QSettings *settings) override;
    void remove(int cover) override;
    void resortCovers(QList<int> newOrder) override;

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;
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
    explicit ComicFlowWidgetGL(QWidget *parent = nullptr);

    void setShowMarks(bool value) override;
    void setMarks(QVector<YACReaderComicReadStatus> marks) override;
    void setMarkImage(QImage &image) override;
    void markSlide(int index, YACReaderComicReadStatus status) override;
    void unmarkSlide(int index) override;
    void setSlideSize(QSize size) override;
    void clear() override;
    void setImagePaths(QStringList paths) override;
    void setCenterIndex(int index) override;
    void showSlide(int index) override;
    int centerIndex() override;
    void updateMarks() override;
    void setFlowType(FlowType flowType) override;
    void render() override;
    void updateConfig(QSettings *settings) override;
    void remove(int cover) override;
    void resortCovers(QList<int> newOrder) override;
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
    void keyPressEvent(QKeyEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;
};
#endif
#endif
