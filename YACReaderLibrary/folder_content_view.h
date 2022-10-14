#ifndef FOLDERCONTENTVIEW_H
#define FOLDERCONTENTVIEW_H

#include <QtWidgets>

#include "comic_model.h"

#include "folder.h"
#include "comic_db.h"

class FolderModel;
class ComicModel;
class YACReaderToolBarStretch;

class QQuickWidget;
class QQmlContext;

class FolderContentView : public QWidget
{
    Q_OBJECT
public:
    explicit FolderContentView(QWidget *parent = nullptr);
    void setModel(const QModelIndex &parent, FolderModel *model);
    void setContinueReadingModel(ComicModel *model);
    void reloadContinueReadingModel();

    FolderModel *currentFolderModel() { return folderModel; }
signals:
    void subfolderSelected(QModelIndex, int);
    void openComic(const ComicDB &comic, const ComicModel::Mode mode);

    // Drops
    void copyComicsToCurrentFolder(QList<QPair<QString, QString>>);
    void moveComicsToCurrentFolder(QList<QPair<QString, QString>>);

    void openFolderContextMenu(QPoint point, Folder folder);
    void openContinueReadingComicContextMenu(QPoint point, ComicDB comic);

protected slots:
    // void onItemClicked(const QModelIndex &mi);
    void updateCoversSizeInContext(int width, QQmlContext *ctxt);
    void setCoversSize(int width);
    virtual void showEvent(QShowEvent *event) override;
    void openFolder(int index);
    void openComicFromContinueReadingList(int index);
    void requestedFolderContextMenu(QPoint point, int index);
    void requestedContinueReadingComicContextMenu(QPoint point, int index);

protected:
    QQuickWidget *view;
    QModelIndex parent;

    std::unique_ptr<ComicModel> comicModel;
    FolderModel *folderModel;

    // Drop to import
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private:
    QSettings *settings;
    QToolBar *toolbar;
    YACReaderToolBarStretch *toolBarStretch;
    QAction *toolBarStretchAction;
    QWidget *coverSizeSliderWidget;
    QSlider *coverSizeSlider;
    QAction *coverSizeSliderAction;
    QAction *showInfoAction;
    QAction *showInfoSeparatorAction;
};

#endif // FOLDERCONTENTVIEW_H
