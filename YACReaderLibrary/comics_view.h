#ifndef COMICS_VIEW_H
#define COMICS_VIEW_H

#include <QWidget>

#include "tablemodel.h"
#include <QAbstractItemView>
#include <QSettings>
#include <QModelIndex>
#include <QModelIndexList>

class YACReaderTableView;
class QSplitter;
class ComicFlowWidget;
class QToolBar;
class TableModel;
class ComicsView : public QWidget
{
    Q_OBJECT
public:
    explicit ComicsView(QWidget *parent = 0);
    virtual void setToolBar(QToolBar * toolBar) = 0;
    virtual void setModel(TableModel *model);
    virtual void setCurrentIndex(const QModelIndex &index) = 0;
    virtual QModelIndex currentIndex() = 0;
    virtual QItemSelectionModel * selectionModel() = 0;
    virtual void scrollTo(const QModelIndex & mi, QAbstractItemView::ScrollHint hint ) = 0;
    virtual void toFullScreen() = 0;
    virtual void toNormal() = 0;
    virtual void updateConfig(QSettings * settings) = 0;
    //Actions for tableviews
    virtual void setItemActions(const QList<QAction *> & actions) = 0;
    //actions for visual-oriented views
    virtual void setViewActions(const QList<QAction *> & actions) = 0;
    //virtual selectItem(int index) = 0;
signals:
    void selected(unsigned int);
    void comicRated(int,QModelIndex);
public slots:
    virtual void setShowMarks(bool show) = 0;
protected:
    TableModel * model;

};

#endif // COMICS_VIEW_H
