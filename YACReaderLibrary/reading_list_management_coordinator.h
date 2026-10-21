#ifndef READING_LIST_MANAGEMENT_COORDINATOR_H
#define READING_LIST_MANAGEMENT_COORDINATOR_H

#include "library_ids.h"

#include <QModelIndex>
#include <QObject>

#include <functional>

class ComicModel;
class QSettings;
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
                                     LibraryPathProvider libraryPathProvider,
                                     QSettings *settings);

public slots:
    void addReadingList();
    void importReadingLists();
    void deleteCurrentList();
    void addLabel();
    void renameCurrentList();
    void renameList(const QModelIndex &list);
    // in the folder or section of the current list, else asks the section
    void addListFolder();
    // inside `list` when it is a folder, else at the top level of its section
    void addListFolderAt(const QModelIndex &list);
    // a folder or a list (not a child list) into a folder of its section, or to its top level
    void moveToFolder(const QModelIndex &list);
    // the dialog starts with `query`, for example the current search
    void addSmartList(const QString &query = QString());
    void editSmartList(const QModelIndex &list);
    void deleteList(const QModelIndex &list);
    // every entry of the reading list (with its child lists) or label
    void matchListToLibraryComics(ListKind listKind, qulonglong listId, const QString &name);

signals:
    void currentListReselectionRequested();
    // the entries of lists changed, maybe also the ones on screen
    void listEntriesChanged();
    // while a smart list dialog is open, the library searches its text; when it closes, the library searches the text
    // of its own search field again
    void searchRequested(const QString &query);
    void searchFinished();

private:
    QWidget *dialogParent;
    ReadingListModel *listsModel;
    CurrentListProvider currentListProvider;
    LibraryPathProvider libraryPathProvider;
    QSettings *settings;
};

#endif // READING_LIST_MANAGEMENT_COORDINATOR_H
