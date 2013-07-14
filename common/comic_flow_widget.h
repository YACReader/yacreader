#pragma once

#include <QWidget>

#include "pictureflow.h"
#include "comic_flow.h"
#include "yacreader_flow_gl.h"

class ComicFlowWidget : public QWidget
{	
	Q_OBJECT
public:
	ComicFlowWidget(QWidget * paret = 0);

public slots:
	virtual void setShowMarks(bool value) = 0;
	virtual void setMarks(QVector<bool> marks) = 0;
	virtual void setMarkImage(QImage & image) = 0;
	virtual void markSlide(int index) = 0;
	virtual void unmarkSlide(int index) = 0;
	virtual void setSlideSize(QSize size) = 0;
	virtual void clear() = 0;
	virtual void setImagePaths(QStringList paths) = 0;
	virtual void setCenterIndex(int index) = 0;
	virtual void showSlide(int index) = 0;
	virtual int centerIndex() = 0;
	virtual void updateMarks() = 0;
	virtual void setFlowType(PictureFlow::FlowType flowType) = 0;
	virtual void render() = 0;
	virtual void updateConfig(QSettings * settings) = 0;
signals:
	void centerIndexChanged(int);
	void selected(unsigned int);
};


class ComicFlowWidgetSW : public ComicFlowWidget
{
	Q_OBJECT
private:
	ComicFlow * flow;
public:
	ComicFlowWidgetSW(QWidget * parent = 0);

	void setShowMarks(bool value);
	void setMarks(QVector<bool> marks);
	void setMarkImage(QImage & image);
	void markSlide(int index);
	void unmarkSlide(int index);
	void setSlideSize(QSize size);
	void clear();
	void setImagePaths(QStringList paths);
	void setCenterIndex(int index);
	void showSlide(int index);
	int centerIndex();
	void updateMarks();
	void setFlowType(PictureFlow::FlowType flowType);
	void render();
	void updateConfig(QSettings * settings);
protected:
	void keyPressEvent(QKeyEvent* event);
	void paintEvent(QPaintEvent *event);
	void mousePressEvent(QMouseEvent* event);
	void resizeEvent(QResizeEvent* event);
	void mouseDoubleClickEvent(QMouseEvent* event);
	QSize minimumSizeHint() const;
	QSize sizeHint() const;
};

class ComicFlowWidgetGL : public ComicFlowWidget
{
	Q_OBJECT
private:
	YACReaderComicFlowGL * flow;
public:
	ComicFlowWidgetGL(QWidget * parent = 0);

	void setShowMarks(bool value);
	void setMarks(QVector<bool> marks);
	void setMarkImage(QImage & image);
	void markSlide(int index);
	void unmarkSlide(int index);
	void setSlideSize(QSize size);
	void clear();
	void setImagePaths(QStringList paths);
	void setCenterIndex(int index);
	void showSlide(int index);
	int centerIndex();
	void updateMarks();
	void setFlowType(PictureFlow::FlowType flowType);
	void render();
	void updateConfig(QSettings * settings);
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
	void keyPressEvent(QKeyEvent* event);
	void paintEvent(QPaintEvent *event);
	void mousePressEvent(QMouseEvent* event);
	void resizeEvent(QResizeEvent* event);
	void mouseDoubleClickEvent(QMouseEvent* event);
	QSize minimumSizeHint() const;
	QSize sizeHint() const;
};