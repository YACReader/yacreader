#ifndef CLASSIC_COMICS_VIEW_H
#define CLASSIC_COMICS_VIEW_H

#include "comics_view.h"

#include <QModelIndex>
#include <QModelIndexList>

class YACReaderTableView;
class QSplitter;
class ComicFlowWidget;
class QToolBar;
class TableModel;
class QStackedWidget;

class ClassicComicsView : public ComicsView
{
    Q_OBJECT
public:
    ClassicComicsView(QWidget *parent = 0);
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

public slots:
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

private:
    YACReaderTableView * tableView;
    QWidget *comics;
    QSplitter * sVertical;
    ComicFlowWidget * comicFlow;
    QSettings * settings;
    void closeEvent ( QCloseEvent * event );

    QStackedWidget * stack;

    QByteArray previousSplitterStatus;
    QWidget * searchingIcon;
    bool searching;
    void setupSearchingIcon();
    void showSearchingIcon();
    void hideSearchingIcon();
    void updateSearchingIconPosition();
};

#endif // CLASSIC_COMICS_VIEW_H
