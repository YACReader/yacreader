#ifndef IMPORT_WIDGET_H
#define IMPORT_WIDGET_H

#include <QWidget>

class QLabel;
class QGraphicsView;
class QGraphicsScene;
class QElapsedTimer;
class QVBoxLayout;
class QToolButton;
class QResizeEvent;

class ImportWidget : public QWidget
{
	Q_OBJECT
public:
	explicit ImportWidget(QWidget *parent = 0);
	
signals:
	void stop();
public slots:
	void newComic(const QString & path, const QString & coverPath);
	void newCover(const QPixmap & image);
	void clear();
	void addCoverTest();
	void finishedUpdatingCover();
	void clearScene();
	void setImportLook();
	void setUpdateLook();
	void showCovers(bool hide);
private:
	QLabel * currentComicLabel;
	QLabel * portadasLabel;
	QLabel * iconLabel;
	QLabel * text;
	QLabel * textDescription;
	QWidget * coversViewContainer;
	QGraphicsView * coversView;
	QGraphicsScene * coversScene;
	int previousWidth;
	bool updatingCovers;
	QElapsedTimer * elapsedTimer;
	quint64 i;

	QToolButton * hideButton;

	void resizeEvent(QResizeEvent * event);
	
};

#endif // IMPORT_WIDGET_H
