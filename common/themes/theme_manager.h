#ifndef THEME_MANAGER_H
#define THEME_MANAGER_H

#include "appearance_configuration.h"
#include "theme.h"

#include <QtCore>

class ThemeRepository;

class ThemeManager : public QObject
{
    Q_OBJECT
public:
    static ThemeManager &instance();

    ThemeManager(const ThemeManager &) = delete;
    ThemeManager &operator=(const ThemeManager &) = delete;
    ThemeManager(ThemeManager &&) = delete;
    ThemeManager &operator=(ThemeManager &&) = delete;

    void initialize(AppearanceConfiguration *config, ThemeRepository *repository);

    void setTheme(const Theme &theme);
    const Theme &getCurrentTheme() const { return currentTheme; }

    AppearanceConfiguration *getAppearanceConfiguration() const { return config; }
    ThemeRepository *getRepository() const { return repository; }

signals:
    void themeChanged();

private:
    explicit ThemeManager();

    AppearanceConfiguration *config = nullptr;
    ThemeRepository *repository = nullptr;
    Theme currentTheme;

    Theme themeFromId(const QString &id, ThemeVariant fallbackVariant);

private slots:
    void resolveTheme();
};

#endif // THEME_MANAGER_H
