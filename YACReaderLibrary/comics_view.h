#ifndef COMICS_VIEW_H
#define COMICS_VIEW_H

#include <QtWidgets>

#include "comic_model.h"

class YACReaderTableView;
class QSplitter;
class ComicFlowWidget;
class QToolBar;
class ComicModel;
class QQuickView;

class ComicsView : public QWidget
{
    Q_OBJECT
public:
    explicit ComicsView(QWidget *parent = 0);
    virtual void setToolBar(QToolBar * toolBar) = 0;
    virtual void setModel(ComicModel *model);
    virtual void setCurrentIndex(const QModelIndex &index) = 0;
    virtual QModelIndex currentIndex() = 0;
    virtual QItemSelectionModel * selectionModel() = 0;
    virtual void scrollTo(const QModelIndex & mi, QAbstractItemView::ScrollHint hint ) = 0;
    virtual void toFullScreen() = 0;
    virtual void toNormal() = 0;
    virtual void updateConfig(QSettings * settings) = 0;
    virtual void enableFilterMode(bool enabled) = 0;
    virtual void selectIndex(int index) = 0;

public slots:
    virtual void updateInfoForIndex(int index);
    virtual void setShowMarks(bool show) = 0;
    virtual void selectAll() = 0;

signals:
    void selected(unsigned int);
    void comicRated(int,QModelIndex);

    //Context menus
    void customContextMenuViewRequested(QPoint);
    void customContextMenuItemRequested(QPoint);

    //Drops
    void copyComicsToCurrentFolder(QList<QPair<QString, QString> >);
    void moveComicsToCurrentFolder(QList<QPair<QString, QString> >);

protected:
    ComicModel * model;

    //Drop to import
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);

    QQuickView *view;
    QWidget *container;

    ComicDB *comicDB;

private:

};

#endif // COMICS_VIEW_H
