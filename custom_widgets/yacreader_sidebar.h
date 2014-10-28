#ifndef YACREADER_SIDEBAR_H
#define YACREADER_SIDEBAR_H

#include <QtWidgets>

class YACReaderFoldersView;
class YACReaderLibraryListWidget;
class YACReaderSearchLineEdit;
class YACReaderTitledToolBar;
class YACReaderTitledToolBar;
class YACReaderReadingListsView;

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

    YACReaderFoldersView * foldersView;
    YACReaderReadingListsView * readingListsView;
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
