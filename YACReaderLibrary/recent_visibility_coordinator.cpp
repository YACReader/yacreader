
#include "recent_visibility_coordinator.h"

#include "yacreader_global_gui.h"

RecentVisibilityCoordinator::RecentVisibilityCoordinator(QSettings *settings, FolderModel *folderModel, FolderContentView *folderContentView, ComicModel *comicModel)
    : QObject(), settings(settings), folderModel(folderModel), folderContentView(folderContentView), comicModel(comicModel)
{
    updateVisibility();
}

void RecentVisibilityCoordinator::toggleVisibility(bool visibility)
{
    settings->setValue(DISPLAY_RECENTLY_INDICATOR, visibility);

    updateVisibility();
}

void RecentVisibilityCoordinator::setTimeRangeInDays(int days)
{
}

void RecentVisibilityCoordinator::updateVisibility()
{
    auto visibility = settings->value(DISPLAY_RECENTLY_INDICATOR, true).toBool();

    folderModel->setShowRecent(visibility);
    folderContentView->setShowRecent(visibility);
    comicModel->setShowRecent(visibility);
}
