#include "appearance_configuration.h"

#include <QSettings>

static constexpr auto kGroup = "Appearance";
static constexpr auto kMode = "ThemeMode";
static constexpr auto kLightId = "LightThemeId";
static constexpr auto kDarkId = "DarkThemeId";
static constexpr auto kFixedId = "FixedThemeId";

static QString themeModeToString(ThemeMode mode)
{
    switch (mode) {
    case ThemeMode::FollowSystem:
        return "FollowSystem";
    case ThemeMode::Light:
        return "Light";
    case ThemeMode::Dark:
        return "Dark";
    case ThemeMode::ForcedTheme:
        return "ForcedTheme";
    }
    return "FollowSystem";
}

static ThemeMode themeModeFromString(const QString &s)
{
    if (s == "Light")
        return ThemeMode::Light;
    if (s == "Dark")
        return ThemeMode::Dark;
    if (s == "ForcedTheme")
        return ThemeMode::ForcedTheme;
    return ThemeMode::FollowSystem;
}

AppearanceConfiguration::AppearanceConfiguration(const QString &settingsFilePath, QObject *parent)
    : QObject(parent), path(settingsFilePath)
{
    load();
}

void AppearanceConfiguration::load()
{
    QSettings s(path, QSettings::IniFormat);
    s.beginGroup(kGroup);
    sel.mode = themeModeFromString(s.value(kMode, "FollowSystem").toString());
    sel.lightThemeId = s.value(kLightId, sel.lightThemeId).toString();
    sel.darkThemeId = s.value(kDarkId, sel.darkThemeId).toString();
    sel.fixedThemeId = s.value(kFixedId, sel.fixedThemeId).toString();
    s.endGroup();
}

void AppearanceConfiguration::write(const QString &key, const QString &value)
{
    QSettings s(path, QSettings::IniFormat);
    s.beginGroup(kGroup);
    s.setValue(key, value);
    s.endGroup();
}

void AppearanceConfiguration::setMode(ThemeMode mode)
{
    if (sel.mode == mode)
        return;
    sel.mode = mode;
    write(kMode, themeModeToString(mode));
    emit selectionChanged();
}

void AppearanceConfiguration::setLightThemeId(const QString &id)
{
    if (sel.lightThemeId == id)
        return;
    sel.lightThemeId = id;
    write(kLightId, id);
    emit selectionChanged();
}

void AppearanceConfiguration::setDarkThemeId(const QString &id)
{
    if (sel.darkThemeId == id)
        return;
    sel.darkThemeId = id;
    write(kDarkId, id);
    emit selectionChanged();
}

void AppearanceConfiguration::setFixedThemeId(const QString &id)
{
    if (sel.fixedThemeId == id)
        return;
    sel.fixedThemeId = id;
    write(kFixedId, id);
    emit selectionChanged();
}
