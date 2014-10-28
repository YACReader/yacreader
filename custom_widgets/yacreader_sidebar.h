#ifndef YACREADER_SIDEBAR_H
#define YACREADER_SIDEBAR_H

#include <QtWidgets>

class YACReaderTreeView;
class YACReaderLibraryListWidget;
class YACReaderSearchLineEdit;
class YACReaderTitledToolBar;
class YACReaderTitledToolBar;

class YACReaderSideBarSeparator : public QWidget
{
public:
    explicit YACReaderSideBarSeparator(QWidget * parent = 0);
protected:
    void paintEvent(QPaintEvent *event);
};

class YACReaderSideBar : public QWidget
{
	Q_OBJECT
public:
	explicit YACReaderSideBar(QWidget *parent = 0);
	QSize sizeHint() const;

	YACReaderTreeView * foldersView;
	YACReaderLibraryListWidget * selectedLibrary;
	YACReaderTitledToolBar * librariesTitle;
	YACReaderTitledToolBar * foldersTitle;
    YACReaderTitledToolBar * readingListsTitle;

    QSplitter * splitter;
	
signals:
	
public slots:

protected:
	void paintEvent(QPaintEvent *);
	
};

#endif // YACREADER_SIDEBAR_H
