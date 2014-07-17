#include "shortcuts_manager.h"

#include <QSettings>
#include <QAction>
#include "yacreader_global.h"

ShortcutsManager::ShortcutsManager()
{
    initDefaultShorcuts();
}

void ShortcutsManager::initDefaultShorcuts()
{
    defaultShorcuts.insert(CREATE_LIBRARY_ACTION_YL,Qt::Key_A);
    defaultShorcuts.insert(OPEN_LIBRARY_ACTION_YL,Qt::Key_O);
    defaultShorcuts.insert(UPDATE_LIBRARY_ACTION_YL,Qt::Key_U);
    defaultShorcuts.insert(RENAME_LIBRARY_ACTION_YL,Qt::Key_R);
    defaultShorcuts.insert(OPEN_COMIC_ACTION_YL,Qt::Key_Return);
    defaultShorcuts.insert(SHOW_HIDE_MARKS_ACTION_YL,Qt::Key_M);
    defaultShorcuts.insert(TOGGLE_FULL_SCREEN_ACTION_YL,Qt::Key_F);
    defaultShorcuts.insert(HELP_ABOUT_ACTION_YL,Qt::Key_F1);
    defaultShorcuts.insert(SET_ROOT_INDEX_ACTION_YL,Qt::Key_0);
    defaultShorcuts.insert(EXPAND_ALL_NODES_ACTION_YL,Qt::Key_Plus);
    defaultShorcuts.insert(COLAPSE_ALL_NODES_ACTION_YL,Qt::Key_Minus);
    defaultShorcuts.insert(OPTIONS_ACTION_YL,Qt::Key_C);
    defaultShorcuts.insert(SERVER_CONFIG_ACTION_YL,Qt::Key_S);
    defaultShorcuts.insert(TOGGLE_COMICS_VIEW_ACTION_YL,Qt::Key_V);
}

void ShortcutsManager::resetToDefaults()
{
    //TODO reset to defaults
}

QString ShortcutsManager::getShortcut(const QString &name)
{
    QSettings s(YACReader::getSettingsPath()+"/YACReaderLibrary.ini",QSettings::IniFormat);
    s.beginGroup("shortcuts");

    return s.value(name,defaultShorcuts.value(name)).toString();
}

void ShortcutsManager::saveShortcut(QAction *action)
{
    QSettings s(YACReader::getSettingsPath()+"/YACReaderLibrary.ini",QSettings::IniFormat);
    s.beginGroup("shortcuts");

    return s.setValue(action->data().toString() , action->shortcut().toString());
}
