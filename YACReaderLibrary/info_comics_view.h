#ifndef INFOCOMICSVIEW_H
#define INFOCOMICSVIEW_H

#include "comics_view.h"

class QQuickView;

class YACReaderComicsSelectionHelper;
class YACReaderComicInfoHelper;

class InfoComicsView : public ComicsView
{
    Q_OBJECT
public:
    explicit InfoComicsView(QWidget *parent = nullptr);
    ~InfoComicsView() override;
    void setToolBar(QToolBar *toolBar) override;
    void setModel(ComicModel *model) override;
    void setCurrentIndex(const QModelIndex &index) override;
    QModelIndex currentIndex() override;
    QItemSelectionModel *selectionModel() override;
    void scrollTo(const QModelIndex &mi, QAbstractItemView::ScrollHint hint) override;
    void toFullScreen() override;
    void toNormal() override;
    void updateConfig(QSettings *settings) override;
    void enableFilterMode(bool enabled) override;
    void selectIndex(int index) override;
    void updateCurrentComicView() override;

public slots:
    void setShowMarks(bool show);
    void selectAll() override;

protected slots:
    void setCurrentIndex(int index);

    bool canDropUrls(const QList<QUrl> &urls, Qt::DropAction action);
    void droppedFiles(const QList<QUrl> &urls, Qt::DropAction action);

    void requestedContextMenu(const QPoint &point);

    void selectedItem(int index);

protected:
    QToolBar *toolbar;
    QObject *flow;
    QObject *list;

    YACReaderComicsSelectionHelper *selectionHelper;
    YACReaderComicInfoHelper *comicInfoHelper;
};

#endif // INFOCOMICSVIEW_H
