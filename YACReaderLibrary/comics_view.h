#ifndef COMICS_VIEW_H
#define COMICS_VIEW_H

#include <QtWidgets>

#include "comic_model.h"

class YACReaderTableView;
class QSplitter;
class ComicFlowWidget;
class QToolBar;
class ComicModel;
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
    //Actions for tableviews
    virtual void setItemActions(const QList<QAction *> & actions) = 0;
    //actions for visual-oriented views
    virtual void setViewActions(const QList<QAction *> & actions) = 0;
    virtual void enableFilterMode(bool enabled) = 0;

signals:
    void selected(unsigned int);
    void comicRated(int,QModelIndex);

    //Drops
    void copyComicsToCurrentFolder(QList<QPair<QString, QString> >);
    void moveComicsToCurrentFolder(QList<QPair<QString, QString> >);

public slots:
    virtual void setShowMarks(bool show) = 0;
    virtual void selectAll() = 0;
protected:
    ComicModel * model;

    //Drop to import
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);

private:

};

#endif // COMICS_VIEW_H
