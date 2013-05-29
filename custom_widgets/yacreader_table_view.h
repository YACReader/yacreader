#ifndef YACREADER_TABLE_VIEW_H
#define YACREADER_TABLE_VIEW_H

#include <QTableView>

class YACReaderDeletingProgress;
class QResizeEvent;

class YACReaderTableView : public QTableView
{
    Q_OBJECT
public:
    explicit YACReaderTableView(QWidget *parent = 0);
    
signals:
    
public slots:
	void showDeleteProgress();
	void hideDeleteProgress();

private:
    YACReaderDeletingProgress * deletingProgress;

	void resizeEvent(QResizeEvent * event);
};

#endif // YACREADER_TABLE_VIEW_H
