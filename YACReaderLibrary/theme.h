#ifndef THEME_H
#define THEME_H

#include <QtCore>

class Theme
{
public:
    Theme();

    static Theme currentTheme() {
        Theme t;

        if (true) { //native macos theme
            t.isMacosNative = true;

            t.disableClassicViewCollapsing = true;

            t.comicsViewTransitionBackground = "#FFFFFF";

            t.itemLibrarySelectedStyleSheet = "color: white; background-color:#91c4f4; border-bottom:1px solid #91c4f4;";
            t.itemLibraryNoSelectedStyleSheet = "background-color:transparent;";

            t.useNativeFolderIcons = true;

            t.noComicsContentBackgroundColor = "#FFFFFF";
            t.noComicsContentTitleLabelStyle = "QLabel {color:#888888; font-size:24px;font-family:Arial;font-weight:bold;}";
        } else {
            t.isMacosNative = false;

            t.disableClassicViewCollapsing = false;

            t.comicsViewTransitionBackground = "#2A2A2A";

            t.itemLibrarySelectedStyleSheet = "color: white; background-color:#2E2E2E; font-weight:bold;";
            t.itemLibraryNoSelectedStyleSheet = "background-color:transparent; color:#DDDFDF;";

            t.useNativeFolderIcons = false;

            t.noComicsContentBackgroundColor = "#2A2A2A";
            t.noComicsContentTitleLabelStyle = "QLabel {color:#CCCCCC; font-size:24px;font-family:Arial;font-weight:bold;}";
        }

        return t;
    }

    bool isMacosNative;

    //
    bool disableClassicViewCollapsing;

    //
    QString comicsViewTransitionBackground;

    //library item
    QString itemLibrarySelectedStyleSheet;
    QString itemLibraryNoSelectedStyleSheet;

    //tree folders
    bool useNativeFolderIcons;

    //EmptyContainerInfo & NoSearchResultsWidget
    QString noComicsContentBackgroundColor;
    QString noComicsContentTitleLabelStyle;
};

#endif // THEME_H
