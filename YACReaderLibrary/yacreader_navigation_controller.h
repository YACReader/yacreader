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
    //reading lists
    void selectedList(const QModelIndex & mi);
    //history navigation
    void selectedIndexFromHistory(const QModelIndex & mi);
    //empty subfolder
    void selectSubfolder(const QModelIndex &sourceMI, int child);

    void loadFolderInfo(const QModelIndex & modelIndex);
    void loadPreviousStatus();

private:

    void setupConnections();
    LibraryWindow * libraryWindow;

    //convenience methods
    qulonglong folderModelIndexToID(const QModelIndex & mi);
};

#endif // YACREADER_NAVIGATION_CONTROLLER_H
