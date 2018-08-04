#ifndef CLASSIC_COMICS_VIEW_H
#define CLASSIC_COMICS_VIEW_H

#include "comics_view.h"

#include <QModelIndex>
#include <QModelIndexList>

class QSplitter;
class QStackedWidget;
class QToolBar;

class ComicFlowWidget;
class ComicModel;
class YACReaderTableView;
class YACReaderToolBarStretch;

class ClassicComicsView : public ComicsView
{
    Q_OBJECT
public:
    ClassicComicsView(QWidget *parent = 0);
    void setToolBar(QToolBar * toolBar);
    void setModel(ComicModel *model);

    QModelIndex currentIndex();
    QItemSelectionModel * selectionModel();
    void scrollTo(const QModelIndex & mi, QAbstractItemView::ScrollHint hint );
    void toFullScreen();
    void toNormal();
    void updateConfig(QSettings * settings);
    void enableFilterMode(bool enabled);
    void selectIndex(int index);
    void updateCurrentComicView();

public slots:
    void setCurrentIndex(const QModelIndex &index);
    void centerComicFlow(const QModelIndex & mi);
    void updateTableView(int i);
    void saveTableHeadersStatus();
    void saveSplitterStatus();
    void applyModelChanges(const QModelIndex & topLeft,const QModelIndex & bottomRight,const QVector<int> & roles);
    void removeItemsFromFlow(const QModelIndex & parent, int from, int to);
    //ComicsView
    void setShowMarks(bool show);
    void selectAll();
    void selectedComicForOpening(const QModelIndex & mi);

protected slots:
    void hideComicFlow(bool hide);
    void requestedViewContextMenu(const QPoint & point);
    void requestedItemContextMenu(const QPoint & point);


private:
    YACReaderTableView * tableView;
    YACReaderToolBarStretch * toolBarStretch;
    QAction * toolBarStretchAction;
    QToolBar * toolbar;
    QWidget *comics;
    QSplitter * sVertical;
    ComicFlowWidget * comicFlow;
    QSettings * settings;
    void closeEvent ( QCloseEvent * event );
    QAction * hideFlowViewAction;

    QStackedWidget * stack;

    QByteArray previousSplitterStatus;
    QWidget * searchingIcon;
    bool searching;
    void setupSearchingIcon();
    void showSearchingIcon();
    void hideSearchingIcon();
    void updateSearchingIconPosition();

    QMutex settingsMutex;
};

#endif // CLASSIC_COMICS_VIEW_H
