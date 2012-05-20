#ifndef __TRANSLATOR_H
#define __TRANSLATOR_H
 
class QUrl; 
class QMouseEvent;
class QPoint;
#include<QWidget>
#include<Phonon/MediaObject>

using namespace Phonon;

class YACReaderTranslator : public QWidget
{
  Q_OBJECT
  public:
	YACReaderTranslator(QWidget * parent = 0);
	~YACReaderTranslator();

	public slots:
		void play(const QUrl & url);

protected:
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void mouseMoveEvent ( QMouseEvent * event );
	bool drag;
	QPoint click; 
private:
	MediaObject * music;
};

#endif