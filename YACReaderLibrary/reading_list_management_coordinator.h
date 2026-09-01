#ifndef READING_LIST_MANAGEMENT_COORDINATOR_H
#define READING_LIST_MANAGEMENT_COORDINATOR_H

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

    ReadingListManagementCoordinator(QWidget *dialogParent,
                                     ReadingListModel *listsModel,
                                     ComicModel *comicsModel,
                                     CurrentListProvider currentListProvider);

public slots:
    void addReadingList();
    void deleteCurrentList();
    void addLabel();
    void renameCurrentList();

signals:
    void currentListReselectionRequested();

private:
    QWidget *dialogParent;
    ReadingListModel *listsModel;
    CurrentListProvider currentListProvider;
};

#endif // READING_LIST_MANAGEMENT_COORDINATOR_H
