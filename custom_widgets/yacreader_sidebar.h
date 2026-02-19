#ifndef YACREADER_SIDEBAR_H
#define YACREADER_SIDEBAR_H

#include <QtWidgets>

#include "themable.h"

class YACReaderFoldersView;
class YACReaderLibraryListWidget;
class YACReaderSearchLineEdit;
class YACReaderTitledToolBar;
class YACReaderTitledToolBar;
class YACReaderReadingListsView;

class YACReaderSideBarSeparator : public QWidget
{
public:
    explicit YACReaderSideBarSeparator(QWidget *parent = 0);
    void setColor(const QColor &color);

protected:
    void paintEvent(QPaintEvent *event);

private:
    QColor separatorColor;
};

class YACReaderSideBar : public QWidget, protected Themable
{
    Q_OBJECT
public:
    explicit YACReaderSideBar(QWidget *parent = 0);
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
    void applyTheme(const Theme &theme) override;

    QSettings *settings;
    QSplitter *splitter;
    QList<YACReaderSideBarSeparator *> separators;
};

#endif // YACREADER_SIDEBAR_H
