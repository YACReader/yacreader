#ifndef INFOCOMICSVIEW_H
#define INFOCOMICSVIEW_H

#include "comics_view.h"



class QQuickView;

class YACReaderComicsSelectionHelper;



class InfoComicsView : public ComicsView
{
    Q_OBJECT
public:
    explicit InfoComicsView(QWidget *parent = 0);
    ~InfoComicsView();
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
    void selectIndex(int index);

public slots:
    void setShowMarks(bool show);
    void selectAll();

protected slots:
    void setCurrentIndex(int index);

    bool canDropUrls(const QList<QUrl> & urls, Qt::DropAction action);
    void droppedFiles(const QList<QUrl> & urls, Qt::DropAction action);

    void requestedContextMenu(const QPoint & point);

    void selectedItem(int index);

protected:
    QToolBar * toolbar;
    QObject *flow;
    QObject *list;

    YACReaderComicsSelectionHelper * selectionHelper;
};

#endif // INFOCOMICSVIEW_H
