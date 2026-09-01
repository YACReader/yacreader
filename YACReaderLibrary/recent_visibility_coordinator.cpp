
#include "recent_visibility_coordinator.h"

#include "yacreader_global_gui.h"

RecentVisibilityCoordinator::RecentVisibilityCoordinator(QSettings *settings, FolderModel *folderModel, ComicModel *comicModel)
    : QObject(), settings(settings), folderModel(folderModel), comicModel(comicModel)
{
    updateVisibility();
    updateTimeRange();
}

void RecentVisibilityCoordinator::toggleVisibility(bool visibility)
{
    settings->setValue(DISPLAY_RECENTLY_INDICATOR, visibility);

    updateVisibility();
}

void RecentVisibilityCoordinator::updateTimeRange()
{
    auto days = settings->value(NUM_DAYS_TO_CONSIDER_RECENT, 1).toInt();
    folderModel->setRecentRange(days);
    comicModel->setRecentRange(days);
}

void RecentVisibilityCoordinator::updateVisibility()
{
    auto visibility = settings->value(DISPLAY_RECENTLY_INDICATOR, true).toBool();

    folderModel->setShowRecent(visibility);
    comicModel->setShowRecent(visibility);
}
