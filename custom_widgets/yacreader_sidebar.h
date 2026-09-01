#ifndef YACREADER_SIDEBAR_H
#define YACREADER_SIDEBAR_H

#include "themable.h"

#include <QCloseEvent>
#include <QColor>
#include <QList>
#include <QPaintEvent>
#include <QSettings>
#include <QSplitter>
#include <QWidget>

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
    QSize sizeHint() const override;

    YACReaderFoldersView *foldersView;
    YACReaderReadingListsView *readingListsView;
    YACReaderLibraryListWidget *selectedLibrary;
    YACReaderTitledToolBar *librariesTitle;
    YACReaderTitledToolBar *foldersTitle;
    YACReaderTitledToolBar *readingListsTitle;

signals:

public slots:

protected:
    void paintEvent(QPaintEvent *) override;
    void closeEvent(QCloseEvent *event) override;
    void applyTheme(const Theme &theme) override;

    QSettings *settings;
    QSplitter *splitter;
    QList<YACReaderSideBarSeparator *> separators;
};

#endif // YACREADER_SIDEBAR_H
