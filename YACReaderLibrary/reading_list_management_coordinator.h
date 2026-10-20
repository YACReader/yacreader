#ifndef READING_LIST_MANAGEMENT_COORDINATOR_H
#define READING_LIST_MANAGEMENT_COORDINATOR_H

#include "library_ids.h"

#include <QModelIndex>
#include <QObject>

#include <functional>

class ComicModel;
class ReadingListModel;
class QWidget;

class ReadingListManagementCoordinator : public QObject
{
    Q_OBJECT

public:
    using CurrentListProvider = std::function<QModelIndex()>;
    using LibraryPathProvider = std::function<QString()>;

    ReadingListManagementCoordinator(QWidget *dialogParent,
                                     ReadingListModel *listsModel,
                                     ComicModel *comicsModel,
                                     CurrentListProvider currentListProvider,
                                     LibraryPathProvider libraryPathProvider);

public slots:
    void addReadingList();
    void importReadingLists();
    void deleteCurrentList();
    void addLabel();
    void renameCurrentList();
    // every entry of the reading list (with its child lists) or label
    void matchListToLibraryComics(ListKind listKind, qulonglong listId, const QString &name);

signals:
    void currentListReselectionRequested();
    // the entries of lists changed, maybe also the ones on screen
    void listEntriesChanged();

private:
    QWidget *dialogParent;
    ReadingListModel *listsModel;
    CurrentListProvider currentListProvider;
    LibraryPathProvider libraryPathProvider;
};

#endif // READING_LIST_MANAGEMENT_COORDINATOR_H
