#include "theme_manager.h"

#include "appearance_configuration.h"
#include "theme.h"
#include "theme_factory.h"
#include "theme_repository.h"

#include <QGuiApplication>
#include <QStyleHints>

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

    return {};
}

void ThemeManager::resolveTheme()
{
    if (!config)
        return;

    const auto &sel = config->selection();

    QString id;
    ThemeVariant fallbackVariant;
    switch (sel.mode) {
    case ThemeMode::FollowSystem: {
        const bool isDark = (qGuiApp->styleHints()->colorScheme() == Qt::ColorScheme::Dark);
        id = isDark ? sel.darkThemeId : sel.lightThemeId;
        fallbackVariant = isDark ? ThemeVariant::Dark : ThemeVariant::Light;
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
        fallbackVariant = (qGuiApp->styleHints()->colorScheme() == Qt::ColorScheme::Dark)
                ? ThemeVariant::Dark
                : ThemeVariant::Light;
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
