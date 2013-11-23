#ifndef __COMICFLOW_H
#define __COMICFLOW_H

#include "yacreader_flow.h"

#include <QtCore>
#include <QObject>
#include <QThread>
#include <QImage>
#include <QMutex>
#include <QWaitCondition>
#include <QString>
#include <QWheelEvent>

class ImageLoader;
class ComicFlow : public YACReaderFlow
{
	Q_OBJECT
public:
	ComicFlow(QWidget* parent = 0,FlowType flowType = CoverFlowLike);
	virtual ~ComicFlow();

	void setImagePaths(const QStringList& paths);
	//bool eventFilter(QObject *target, QEvent *event);
	void keyPressEvent(QKeyEvent* event);
	void removeSlide(int cover);

private slots:
  void preload();
  void updateImageData();

private:
  //QString imagePath;
  QStringList imageFiles;
  QVector<bool> imagesLoaded;
  QVector<bool> imagesSetted;
  int numImagesLoaded;
  QTimer* updateTimer;
  ImageLoader* worker;
  virtual void wheelEvent(QWheelEvent * event);
}; 


//-----------------------------------------------------------------------------
// Source code of ImageLoader class was modified from http://code.google.com/p/photoflow/
//------------------------------------------------------------------------------
class ImageLoader : public QThread
{
public:
  ImageLoader();
  ~ImageLoader();
  // returns FALSE if worker is still busy and can't take the task
  bool busy() const;
  void generate(int index, const QString& fileName, QSize size);
  void reset(){idx = -1;};
  int index() const { return idx; };
  QImage result();

protected:
  void run();

private:
  QMutex mutex;
  QWaitCondition condition;

  bool restart;
  bool working;
  int idx;
  QString fileName;
  QSize size;
  QImage img;
};


#endif
