#ifndef COMICS_VIEW_H
#define COMICS_VIEW_H

#include "comic_model.h"
#include "content_view_state.h"

#include <QAbstractItemView>
#include <QSettings>
#include <QWidget>

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
    explicit ComicsView(QWidget *parent = nullptr);
    ~ComicsView() override;
    virtual void setToolBar(QToolBar *toolBar) = 0;
    virtual void releaseToolBar() = 0;
    virtual void saveViewConfig() { }
    virtual void setModel(ComicModel *model);
    virtual void setCurrentIndex(const QModelIndex &index) = 0;
    virtual QModelIndex currentIndex() = 0;
    virtual QItemSelectionModel *selectionModel() = 0;
    virtual void scrollTo(const QModelIndex &mi, QAbstractItemView::ScrollHint hint) = 0;
    virtual void toFullScreen() = 0;
    virtual void toNormal() = 0;
    virtual void updateConfig(QSettings *settings) = 0;
    virtual void enableFilterMode(bool enabled) = 0;
    virtual void selectIndex(int index) = 0;
    virtual void updateCurrentComicView() = 0;
    virtual void focusComicsNavigation(Qt::FocusReason reason) = 0;
    virtual void reloadContent();
    virtual ContentViewState captureViewState() const { return { }; }
    virtual void restoreViewState(const ContentViewState &state) { Q_UNUSED(state); }

    // Import of comics dropped on the view. Native child windows (e.g. the 3D flow) use these,
    // because their drag events do not propagate to this widget.
    bool canImportDrop(const QDropEvent *event) const;
    void importDrop(QDropEvent *event);

public slots:
    virtual void updateInfoForIndex(int index);
    virtual void setShowMarks(bool show) = 0;
    virtual void selectAll() = 0;

signals:
    void selected(unsigned int);
    void openComic(const ComicDB &comic, const ComicModel::Mode mode);
    void comicRated(int, QModelIndex);

    // Context menus
    void customContextMenuViewRequested(QPoint);
    void customContextMenuItemRequested(QPoint);

    // Drops
    void copyComicsToCurrentFolder(QList<QPair<QString, QString>>);
    void moveComicsToCurrentFolder(QList<QPair<QString, QString>>);
    void customComicCoverRequested(qulonglong comicId, const QString &imagePath);
    void customFolderCoverRequested(qulonglong folderId, const QString &imagePath);

protected:
    ComicModel *model;

    // Drop to import
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

    // The QML view is a native window embedded with QWidget::createWindowContainer.
    // Add 'container' to layouts and give it the focus; do not use 'view' as a widget.
    QQuickView *view;
    QWidget *container;

    ComicDB *comicDB;

private:
};

#endif // COMICS_VIEW_H
