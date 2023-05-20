
#ifndef RECENT_VISIBILITY_COORDINATOR_H
#define RECENT_VISIBILITY_COORDINATOR_H

#include <QtCore>

#include "folder_model.h"
#include "comic_model.h"
#include "folder_content_view.h"

class RecentVisibilityCoordinator : public QObject
{
    Q_OBJECT
public:
    explicit RecentVisibilityCoordinator(QSettings *settings, FolderModel *folderModel, FolderContentView *folderContentView, ComicModel *comicModel);

public slots:
    void toggleVisibility(bool visibility);
    void updateTimeRange();

private:
    QSettings *settings;
    FolderModel *folderModel;
    FolderContentView *folderContentView;
    ComicModel *comicModel;

    void updateVisibility();
};

#endif // RECENT_VISIBILITY_COORDINATOR_H
