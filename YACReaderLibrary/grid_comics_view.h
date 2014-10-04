#ifndef GRID_COMICS_VIEW_H
#define GRID_COMICS_VIEW_H

#include "comics_view.h"

#include <QModelIndex>

class QAbstractListModel;
class QItemSelectionModel;
class QQuickView;
class QQuickView;


class GridComicsView : public ComicsView
{
    Q_OBJECT
public:
    explicit GridComicsView(QWidget *parent = 0);
    virtual ~GridComicsView();
    void setToolBar(QToolBar * toolBar);
    void setModel(TableModel *model);
    void setCurrentIndex(const QModelIndex &index);
    QModelIndex currentIndex();
    QItemSelectionModel * selectionModel();
    void scrollTo(const QModelIndex & mi, QAbstractItemView::ScrollHint hint );
    void toFullScreen();
    void toNormal();
    void updateConfig(QSettings * settings);
    void setItemActions(const QList<QAction *> & actions);
    void setViewActions(const QList<QAction *> & actions);
    void enableFilterMode(bool enabled);

    QSize sizeHint();
signals:
signals:
    void comicRated(int,QModelIndex);
    void doubleClicked(QModelIndex);

public slots:
    //selection helper
    void selectIndex(int index);
    bool isSelectedIndex(int index);
    void clear();
    //double clicked item
    void selectedItem(int index);

    //ComicsView
    void setShowMarks(bool show);
    void selectAll();

private:
    QItemSelectionModel * _selectionModel;
    QQuickView *view;
    QWidget *container;
    bool dummy;
    void closeEvent ( QCloseEvent * event );

};

#endif // GRID_COMICS_VIEW_H
