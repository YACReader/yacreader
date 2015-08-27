#ifndef GRID_COMICS_VIEW_H
#define GRID_COMICS_VIEW_H

#include "comics_view.h"

#include <QModelIndex>

class QAbstractListModel;
class QItemSelectionModel;
class QQuickView;
class QQuickView;

class YACReaderToolBarStretch;

class GridComicsView : public ComicsView
{
    Q_OBJECT
public:
    explicit GridComicsView(QWidget *parent = 0);
    virtual ~GridComicsView();
    void setToolBar(QToolBar * toolBar);
    void setModel(ComicModel *model);
    void setCurrentIndex(const QModelIndex &index);
    QModelIndex currentIndex();
    QItemSelectionModel * selectionModel();
    void scrollTo(const QModelIndex & mi, QAbstractItemView::ScrollHint hint );
    void toFullScreen();
    void toNormal();
    void updateConfig(QSettings * settings);
    void enableFilterMode(bool enabled);
    QSize sizeHint();
    QByteArray getMimeDataFromSelection();


signals:
    void comicRated(int,QModelIndex);
    void doubleClicked(QModelIndex);

public slots:
    //selection helper
    void selectIndex(int index);
    void setCurrentIndex(int index);
    void deselectIndex(int index);
    bool isSelectedIndex(int index);
    void clear();
    //double clicked item
    void selectedItem(int index);
    int numItemsSelected();
    int lastSelectedIndex();

    //ComicsView
    void setShowMarks(bool show);
    void selectAll();

    //rating
    void rate(int index, int rating);

    //dragManager
    void startDrag();
    //dropManager
    bool canDropUrls(const QList<QUrl> & urls, Qt::DropAction action);
    bool canDropFormats(const QString &formats);
    void droppedFiles(const QList<QUrl> & urls, Qt::DropAction action);
    void droppedComicsForResortingAt(const QString & data, int index);


protected slots:
    void requestedContextMenu(const QPoint & point);
    void setCoversSize(int width);

private:
    QSettings * settings;
    QToolBar * toolbar;
    YACReaderToolBarStretch * toolBarStretch;
    QAction * toolBarStretchAction;
    QWidget * coverSizeSliderWidget;
    QSlider * coverSizeSlider;
    QAction * coverSizeSliderAction;
    QItemSelectionModel * _selectionModel;
    QQuickView *view;
    QWidget *container;
    bool dummy;
    void closeEvent ( QCloseEvent * event );
    void createCoverSizeSliderWidget();

};

#endif // GRID_COMICS_VIEW_H
