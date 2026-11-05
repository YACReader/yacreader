#include "header_model.h"

HeaderModel::HeaderModel(const HeaderActions &actions, QObject *parent)
    : QObject(parent), headerActions(actions)
{
}

void HeaderModel::setFolderName(const QString &name)
{
    if (name == currentFolderName)
        return;
    currentFolderName = name;
    emit folderNameChanged(name);
}

void HeaderModel::setEnabled(bool enabled)
{
    if (enabled == this->enabled)
        return;
    this->enabled = enabled;
    emit enabledChanged(enabled);
}

void HeaderModel::setSearchText(const QString &text, bool notify)
{
    if (text == currentSearchText)
        return;
    currentSearchText = text;
    emit searchTextSet(text);
    if (notify)
        emit searchChanged(text);
}

void HeaderModel::setSearchEnabled(bool enabled)
{
    if (enabled == searchEnabled)
        return;
    searchEnabled = enabled;
    emit searchEnabledChanged(enabled);
}

void HeaderModel::setSearchMenu(QMenu *menu)
{
    this->menu = menu;
    emit searchMenuChanged(menu);
}

void HeaderModel::focusSearch()
{
    emit searchFocusRequested();
}

void HeaderModel::editSearch(const QString &text)
{
    currentSearchText = text;
    emit searchChanged(text);
}
