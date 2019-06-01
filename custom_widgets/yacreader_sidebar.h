#ifndef YACREADER_SIDEBAR_H
#define YACREADER_SIDEBAR_H

#include <QtWidgets>

#include "theme.h"

class YACReaderFoldersView;
class YACReaderLibraryListWidget;
class YACReaderSearchLineEdit;
class YACReaderTitledToolBar;
class YACReaderTitledToolBar;
class YACReaderReadingListsView;

class YACReaderSideBarSeparator : public QWidget
{
public:
    explicit YACReaderSideBarSeparator(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event);
};

class YACReaderSideBar : public QWidget
{
    Q_OBJECT
public:
    explicit YACReaderSideBar(QWidget *parent = nullptr);
    QSize sizeHint() const;

    YACReaderFoldersView *foldersView;
    YACReaderReadingListsView *readingListsView;
    YACReaderLibraryListWidget *selectedLibrary;
    YACReaderTitledToolBar *librariesTitle;
    YACReaderTitledToolBar *foldersTitle;
    YACReaderTitledToolBar *readingListsTitle;

signals:

public slots:

protected:
    void paintEvent(QPaintEvent *);
    void closeEvent(QCloseEvent *event);
    QSettings *settings;
    QSplitter *splitter;
    Theme theme = Theme::currentTheme();
};

#endif // YACREADER_SIDEBAR_H
