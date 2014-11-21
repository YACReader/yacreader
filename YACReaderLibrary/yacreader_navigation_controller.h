#ifndef YACREADER_NAVIGATION_CONTROLLER_H
#define YACREADER_NAVIGATION_CONTROLLER_H

#include <QObject>
class LibraryWindow;


class YACReaderNavigationController : public QObject
{
    Q_OBJECT
public:

    explicit YACReaderNavigationController(LibraryWindow * parent);

signals:

public slots:
    //info origins
    //folders view
    void selectedFolder(const QModelIndex & mi);
    void reselectCurrentFolder();
    //reading lists
    void selectedList(const QModelIndex & mi);
    void reselectCurrentList();
    //history navigation
    void selectedIndexFromHistory(const QModelIndex & mi);
    //empty subfolder
    void selectSubfolder(const QModelIndex &sourceMI, int child);

    void loadEmptyFolderInfo(const QModelIndex & modelIndex);

    void loadFolderInfo(const QModelIndex & modelIndex);
    void loadListInfo(const QModelIndex & modelIndex);
    void loadSpecialListInfo(const QModelIndex & modelIndex);
    void loadLabelInfo(const QModelIndex & modelIndex);
    void loadReadingListInfo(const QModelIndex & modelIndex);

    void loadPreviousStatus();

private:

    void setupConnections();
    LibraryWindow * libraryWindow;

    //convenience methods
    qulonglong folderModelIndexToID(const QModelIndex & mi);
};

#endif // YACREADER_NAVIGATION_CONTROLLER_H
