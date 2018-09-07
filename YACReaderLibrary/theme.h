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
        } else {
            t.isMacosNative = false;

            t.disableClassicViewCollapsing = false;

            t.comicsViewTransitionBackground = "#2A2A2A";
        }
    }

    bool isMacosNative;

    //
    bool disableClassicViewCollapsing;

    //
    QString comicsViewTransitionBackground;
};

#endif // THEME_H
