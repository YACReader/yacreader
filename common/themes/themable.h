#ifndef THEMABLE_H
#define THEMABLE_H

#include "theme_manager.h"

class Themable
{
protected:
    void initTheme(QObject *owner)
    {
        QObject::connect(&ThemeManager::instance(),
                         &ThemeManager::themeChanged,
                         owner,
                         [this]() {
                             theme = ThemeManager::instance().getCurrentTheme();
                             applyTheme(theme);
                         });

        applyTheme(ThemeManager::instance().getCurrentTheme());
    }

    virtual void applyTheme(const Theme &theme) = 0;

    Theme theme;
};

#endif // THEMABLE_H
