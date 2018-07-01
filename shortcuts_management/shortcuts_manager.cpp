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
#ifdef YACREADER_LIBRARY
    //ACTIONS
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

    //COMMANDS (used in keypressevent)
#else
    defaultShorcuts.insert(OPEN_ACTION_Y, Qt::Key_O);
    defaultShorcuts.insert(OPEN_LATEST_COMIC_Y, Qt::CTRL | Qt::Key_R);
    defaultShorcuts.insert(OPEN_FOLDER_ACTION_Y, Qt::CTRL | Qt::Key_O);
    defaultShorcuts.insert(OPEN_PREVIOUS_COMIC_ACTION_Y, Qt::CTRL | Qt::Key_Left);
    defaultShorcuts.insert(OPEN_NEXT_COMIC_ACTION_Y, Qt::CTRL | Qt::Key_Right);
    defaultShorcuts.insert(PREV_ACTION_Y, Qt::Key_Left);
    defaultShorcuts.insert(NEXT_ACTION_Y, Qt::Key_Right);
    defaultShorcuts.insert(LEFT_ROTATION_ACTION_Y, Qt::Key_L);
    defaultShorcuts.insert(RIGHT_ROTATION_ACTION_Y, Qt::Key_R);
    defaultShorcuts.insert(DOUBLE_PAGE_ACTION_Y, Qt::Key_D);
    defaultShorcuts.insert(DOUBLE_MANGA_PAGE_ACTION_Y, Qt::Key_J);
    defaultShorcuts.insert(GO_TO_PAGE_ACTION_Y, Qt::Key_G);
    defaultShorcuts.insert(OPTIONS_ACTION_Y, Qt::Key_C);
    defaultShorcuts.insert(HELP_ABOUT_ACTION_Y, Qt::Key_F1);
    defaultShorcuts.insert(SHOW_MAGNIFYING_GLASS_ACTION_Y, Qt::Key_Z);
    defaultShorcuts.insert(SET_BOOKMARK_ACTION_Y, Qt::CTRL | Qt::Key_M);
    defaultShorcuts.insert(SHOW_BOOKMARKS_ACTION_Y, Qt::Key_M);
    defaultShorcuts.insert(SHOW_INFO_ACTION_Y, Qt::Key_I);
    defaultShorcuts.insert(CLOSE_ACTION_Y, Qt::Key_Escape);
    defaultShorcuts.insert(SHOW_DICTIONARY_ACTION_Y, Qt::Key_T);
    defaultShorcuts.insert(ALWAYS_ON_TOP_ACTION_Y, Qt::Key_Q); //deprecated
    defaultShorcuts.insert(ADJUST_TO_FULL_SIZE_ACTION_Y, Qt::Key_W);
    defaultShorcuts.insert(SHOW_FLOW_ACTION_Y, Qt::Key_S);
    defaultShorcuts.insert(ZOOM_PLUS_ACTION_Y, Qt::Key_Plus);
    defaultShorcuts.insert(ZOOM_MINUS_ACTION_Y, Qt::Key_Minus);
    defaultShorcuts.insert(RESET_ZOOM_ACTION_Y, Qt::CTRL | Qt::Key_0);

    //main_window_viewer
    defaultShorcuts.insert(TOGGLE_FULL_SCREEN_ACTION_Y, Qt::Key_F);
    defaultShorcuts.insert(TOGGLE_TOOL_BARS_ACTION_Y, Qt::Key_H);
    defaultShorcuts.insert(CHANGE_FIT_ACTION_Y, Qt::Key_A);
    //viewer
    defaultShorcuts.insert(AUTO_SCROLL_FORWARD_ACTION_Y, Qt::Key_Space);
    defaultShorcuts.insert(AUTO_SCROLL_BACKWARD_ACTION_Y, Qt::Key_B);
    defaultShorcuts.insert(MOVE_DOWN_ACTION_Y, Qt::Key_Down);
    defaultShorcuts.insert(MOVE_UP_ACTION_Y, Qt::Key_Up);
    defaultShorcuts.insert(GO_TO_FIRST_PAGE_ACTION_Y, Qt::Key_Home);
    defaultShorcuts.insert(GO_TO_LAST_PAGE_ACTION_Y, Qt::Key_End);
    //mglass
    defaultShorcuts.insert(SIZE_UP_MGLASS_ACTION_Y, Qt::Key_Plus);
    defaultShorcuts.insert(SIZE_DOWN_MGLASS_ACTION_Y, Qt::Key_Minus);
    defaultShorcuts.insert(ZOOM_IN_MGLASS_ACTION_Y, Qt::Key_Asterisk);
    defaultShorcuts.insert(ZOOM_OUT_MGLASS_ACTION_Y, Qt::Key_Underscore);

#endif

}

void ShortcutsManager::resetToDefaults()
{
    //TODO reset to defaults
}

QString ShortcutsManager::getShortcut(const QString &name)
{
#ifdef YACREADER
    QString filePath = "/YACReader.ini";
#else
    QString filePath = "/YACReaderLibrary.ini";
#endif
    QSettings s(YACReader::getSettingsPath()+filePath,QSettings::IniFormat);
    s.beginGroup("shortcuts");

    return s.value(name,defaultShorcuts.value(name)).toString();
}

void ShortcutsManager::saveShortcut(QAction *action)
{
#ifdef YACREADER
    QString filePath = "/YACReader.ini";
#else
    QString filePath = "/YACReaderLibrary.ini";
#endif
    QSettings s(YACReader::getSettingsPath()+filePath,QSettings::IniFormat);
    s.beginGroup("shortcuts");

    return s.setValue(action->data().toString() , action->shortcut().toString());
}

void ShortcutsManager::registerActions(const QList<QAction *> &a)
{
    actions = a;
}

bool ShortcutsManager::checkConflicts(const QKeySequence & shortcut, const QAction *dest)
{
    if(shortcut.isEmpty())
        return false;

    foreach(QAction * action, actions)
    {
        if(action != dest) //if the same shortcut is setted there is no conflict
            if(action->shortcut() == shortcut)
                return true;
    }

    return false;
}
