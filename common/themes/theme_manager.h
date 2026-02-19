#ifndef THEME_MANAGER_H
#define THEME_MANAGER_H

#include "theme.h"
#include "theme_id.h"

#include <QtCore>

class ThemeManager : public QObject
{
    Q_OBJECT
public:
    static ThemeManager &instance();

    ThemeManager(const ThemeManager &) = delete;
    ThemeManager &operator=(const ThemeManager &) = delete;
    ThemeManager(ThemeManager &&) = delete;
    ThemeManager &operator=(ThemeManager &&) = delete;

    void initialize();

    void setTheme(ThemeId themeId);

    const Theme &getCurrentTheme() const { return currentTheme; }

signals:
    void themeChanged();

private:
    explicit ThemeManager();
    ThemeId themeId = ThemeId::Classic;
    Theme currentTheme;

    void updateCurrentTheme();
};

#endif // THEME_MANAGER_H
