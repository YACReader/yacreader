#include "theme_manager.h"

#include "theme.h"
#include "theme_factory.h"

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

void ThemeManager::initialize()
{
    auto *styleHints = qGuiApp->styleHints();

    auto colorScheme = styleHints->colorScheme();

    // TODO: settings are needed to decide what theme to use
    auto applyColorScheme = [this](Qt::ColorScheme scheme) {
        setTheme(scheme == Qt::ColorScheme::Dark ? ThemeId::Dark : ThemeId::Light);
    };

    applyColorScheme(colorScheme);

    connect(styleHints, &QStyleHints::colorSchemeChanged, this, applyColorScheme, Qt::QueuedConnection);
}

void ThemeManager::setTheme(ThemeId themeId)
{
    if (this->themeId == themeId) {
        return;
    }

    this->themeId = themeId;

    updateCurrentTheme();

    emit themeChanged();
}

void ThemeManager::updateCurrentTheme()
{
    currentTheme = makeTheme(themeId);
}
