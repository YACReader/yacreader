#include "theme_manager.h"

#include "appearance_configuration.h"
#include "theme.h"
#include "theme_factory.h"
#include "theme_repository.h"

#include <QGuiApplication>
#include <QPalette>
#include <QStyleHints>

ThemeVariant themeVariantFromColorScheme(Qt::ColorScheme colorScheme);

ThemeManager::ThemeManager()
{
}

ThemeManager &ThemeManager::instance()
{
    static ThemeManager instance;
    return instance;
}

void ThemeManager::initialize(AppearanceConfiguration *config, ThemeRepository *repository)
{
    this->config = config;
    this->repository = repository;

    auto *styleHints = qGuiApp->styleHints();

    // Re-resolve when OS color scheme changes (relevant for FollowSystem mode)
    connect(styleHints, &QStyleHints::colorSchemeChanged, this, &ThemeManager::resolveTheme, Qt::QueuedConnection);

    // Re-resolve when the user changes any theme setting
    connect(config, &AppearanceConfiguration::selectionChanged, this, &ThemeManager::resolveTheme);

    resolveTheme();
}

void ThemeManager::setTheme(const Theme &theme)
{
    currentTheme = theme;
    emit themeChanged();
}

Theme ThemeManager::themeFromId(const QString &id, ThemeVariant fallbackVariant)
{
    // Try the repository first (handles both builtin and user themes via JSON)
    if (repository && repository->contains(id)) {
        QJsonObject json = repository->loadThemeJson(id);
        if (!json.isEmpty())
            return makeTheme(json);
    }

    // Fallback to the builtin that matches the current dark/light intent.
    const QString fallbackId = (fallbackVariant == ThemeVariant::Dark)
            ? QStringLiteral("builtin/dark")
            : QStringLiteral("builtin/light");
    if (repository && repository->contains(fallbackId)) {
        QJsonObject json = repository->loadThemeJson(fallbackId);
        if (!json.isEmpty())
            return makeTheme(json);
    }

    return { };
}

void ThemeManager::resolveTheme()
{
    if (!config)
        return;

    const auto &sel = config->selection();
    const ThemeVariant systemVariant = themeVariantFromColorScheme(qGuiApp->styleHints()->colorScheme());

    QString id;
    ThemeVariant fallbackVariant;
    switch (sel.mode) {
    case ThemeMode::FollowSystem: {
        const bool isDark = (systemVariant == ThemeVariant::Dark);
        id = isDark ? sel.darkThemeId : sel.lightThemeId;
        fallbackVariant = systemVariant;
        break;
    }
    case ThemeMode::Light:
        id = sel.lightThemeId;
        fallbackVariant = ThemeVariant::Light;
        break;
    case ThemeMode::Dark:
        id = sel.darkThemeId;
        fallbackVariant = ThemeVariant::Dark;
        break;
    case ThemeMode::ForcedTheme:
        id = sel.fixedThemeId;
        fallbackVariant = systemVariant;
        break;
    }

    const Theme theme = themeFromId(id, fallbackVariant);

    // Sync Qt's application-level color scheme so native widgets (menus, scrollbars,
    // standard dialogs) use the correct palette before themeChanged() is emitted.
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
    Qt::ColorScheme scheme;
    switch (sel.mode) {
    case ThemeMode::FollowSystem:
        scheme = Qt::ColorScheme::Unknown; // delegate to OS
        break;
    case ThemeMode::Light:
        scheme = Qt::ColorScheme::Light;
        break;
    case ThemeMode::Dark:
        scheme = Qt::ColorScheme::Dark;
        break;
    case ThemeMode::ForcedTheme:
        scheme = (theme.meta.variant == ThemeVariant::Dark)
                ? Qt::ColorScheme::Dark
                : Qt::ColorScheme::Light;
        break;
    }
    qGuiApp->styleHints()->setColorScheme(scheme);
#endif

    setTheme(theme);
}

bool paletteRolePairLooksDark(const QPalette &palette, QPalette::ColorRole backgroundRole, QPalette::ColorRole foregroundRole)
{
    const QColor background = palette.color(QPalette::Active, backgroundRole);
    const QColor foreground = palette.color(QPalette::Active, foregroundRole);

    return background.lightness() < foreground.lightness();
}

bool paletteLooksDark()
{
    const QPalette palette = qGuiApp->palette();

    return paletteRolePairLooksDark(palette, QPalette::Window, QPalette::WindowText) || paletteRolePairLooksDark(palette, QPalette::Base, QPalette::Text);
}

ThemeVariant themeVariantFromColorScheme(Qt::ColorScheme colorScheme)
{
    if (colorScheme == Qt::ColorScheme::Dark)
        return ThemeVariant::Dark;

    if (colorScheme == Qt::ColorScheme::Unknown && paletteLooksDark())
        return ThemeVariant::Dark;

    return ThemeVariant::Light;
}
