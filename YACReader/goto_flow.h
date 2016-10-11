#ifndef __GOTO_FLOW_H
#define __GOTO_FLOW_H

#include "goto_flow_widget.h"
#include "yacreader_global_gui.h"

#include <QThread>

#include <QWaitCondition>
#include <QMutex>

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
class SlideInitializer;
class PageLoader;
class YACReaderFlow;
class PictureFlow;
class QKeyEvent;

class GoToFlow : public GoToFlowWidget
{
	Q_OBJECT
public:
	GoToFlow(QWidget* parent = 0,FlowType flowType = CoverFlowLike);
	~GoToFlow();
	bool ready; //comic is ready for read.
private:
	YACReaderFlow * flow;
	void keyPressEvent(QKeyEvent* event);
	//Comic * comic;
	QSize imageSize;

	QVector<bool> imagesLoaded;
	QVector<bool> imagesSetted;
	int numImagesLoaded;
	QVector<bool> imagesReady;
	QVector<QByteArray> rawImages;
	QTimer* updateTimer;
	PageLoader* worker;
	virtual void wheelEvent(QWheelEvent * event);
	QMutex mutexGoToFlow;

private slots:
    void preload();
    void updateImageData();
    void resizeEvent(QResizeEvent *event);

    public slots:
        void centerSlide(int slide);
    void reset();
    void setNumSlides(unsigned int slides);
    void setImageReady(int index,const QByteArray & image);
    void setFlowType(FlowType flowType);
    void updateSize();
    void updateConfig(QSettings * settings);
signals:
    void goToPage(unsigned int page);

};
//-----------------------------------------------------------------------------
//SlideInitializer
//-----------------------------------------------------------------------------
class SlideInitializer : public QThread
{
public:
	SlideInitializer(QMutex * m,PictureFlow * flow,int slides);
private:
	QMutex * mutex;
	PictureFlow * _flow;
	int _slides;
	void run();
};
//-----------------------------------------------------------------------------
//PageLoader
//-----------------------------------------------------------------------------

class PageLoader : public QThread
{
public:
	PageLoader(QMutex * m);
	~PageLoader();
	// returns FALSE if worker is still busy and can't take the task
	bool busy() const;
	void generate(int index, QSize size,const QByteArray & rImage);
	void reset(){idx = -1;};
	int index() const { return idx; }
	QImage result() const { return img; }
protected:
	void run();
private:
	QMutex * mutex;
	QWaitCondition condition;

	bool restart;
	bool working;
	int idx;

	QSize size;
	QImage img;
	QByteArray rawImage;
};

#endif
