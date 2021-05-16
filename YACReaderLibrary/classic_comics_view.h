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
    explicit ClassicComicsView(QWidget *parent = nullptr);
    void setToolBar(QToolBar *toolBar) override;
    void setModel(ComicModel *model) override;

    QModelIndex currentIndex() override;
    QItemSelectionModel *selectionModel() override;
    void scrollTo(const QModelIndex &mi, QAbstractItemView::ScrollHint hint) override;
    void toFullScreen() override;
    void toNormal() override;
    void updateConfig(QSettings *settings) override;
    void enableFilterMode(bool enabled) override;
    void selectIndex(int index) override;
    void updateCurrentComicView() override;
    void focusComicsNavigation(Qt::FocusReason reason) override;

public slots:
    void setCurrentIndex(const QModelIndex &index) override;
    void centerComicFlow(const QModelIndex &mi);
    void updateTableView(int i);
    void saveTableHeadersStatus();
    void saveSplitterStatus();
    void applyModelChanges(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles);
    void removeItemsFromFlow(const QModelIndex &parent, int from, int to);
    //ComicsView
    void setShowMarks(bool show) override;
    void selectAll() override;
    void selectedComicForOpening(const QModelIndex &mi);

protected slots:
    void hideComicFlow(bool hide);
    void requestedViewContextMenu(const QPoint &point);
    void requestedItemContextMenu(const QPoint &point);

private:
    YACReaderTableView *tableView;
    YACReaderToolBarStretch *toolBarStretch;
    QAction *toolBarStretchAction;
    QToolBar *toolbar;
    QWidget *comics;
    QSplitter *sVertical;
    ComicFlowWidget *comicFlow;
    QSettings *settings;
    void closeEvent(QCloseEvent *event) override;
    QAction *hideFlowViewAction;

    QStackedWidget *stack;

    QByteArray previousSplitterStatus;
    QWidget *searchingIcon;
    bool searching;
    void setupSearchingIcon();
    void showSearchingIcon();
    void hideSearchingIcon();
    void updateSearchingIconPosition();

    QMutex settingsMutex;
};

#endif // CLASSIC_COMICS_VIEW_H
