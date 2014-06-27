#ifndef YACREADER_SIDEBAR_H
#define YACREADER_SIDEBAR_H

#include <QWidget>

class YACReaderTreeView;
class YACReaderLibraryListWidget;
class YACReaderSearchLineEdit;
class YACReaderTitledToolBar;
class YACReaderTitledToolBar;

class YACReaderSideBar : public QWidget
{
	Q_OBJECT
public:
	explicit YACReaderSideBar(QWidget *parent = 0);
	QSize sizeHint() const;

	YACReaderTreeView * foldersView;
	YACReaderLibraryListWidget * selectedLibrary;
	YACReaderSearchLineEdit * foldersFilter;
	YACReaderTitledToolBar * librariesTitle;
	YACReaderTitledToolBar * foldersTitle;
	
signals:
	
public slots:

protected:
	void paintEvent(QPaintEvent *);
	
};

#endif // YACREADER_SIDEBAR_H
