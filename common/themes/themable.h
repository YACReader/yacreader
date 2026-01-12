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
                             applyTheme();
                         });

        applyTheme();
    }

    virtual void applyTheme() = 0;
};

#endif // THEMABLE_H
