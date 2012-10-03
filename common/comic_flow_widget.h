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
protected:
	void keyPressEvent(QKeyEvent* event);
	void paintEvent(QPaintEvent *event);
	void mousePressEvent(QMouseEvent* event);
	void resizeEvent(QResizeEvent* event);
	void mouseDoubleClickEvent(QMouseEvent* event);
	QSize minimumSizeHint() const;
	QSize sizeHint() const;
};