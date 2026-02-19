#include "theme_manager.h"

#include "theme.h"
#include "theme_factory.h"

#include <QGuiApplication>
#include <QPalette>
#include <QStyleHints>

// TODO: add API to force color scheme     //styleHints->setColorScheme(Qt::ColorScheme::Dark);

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
    // QStyleHints::colorScheme is only 6.5+
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    auto *styleHints = qGuiApp->styleHints();

    auto colorScheme = styleHints->colorScheme();

    // TODO: settings are needed to decide what theme to use
    auto applyColorScheme = [this](Qt::ColorScheme scheme) {
        setTheme(scheme == Qt::ColorScheme::Dark ? ThemeId::Dark : ThemeId::Light);
    };

    applyColorScheme(colorScheme);

    connect(styleHints, &QStyleHints::colorSchemeChanged, this, applyColorScheme, Qt::QueuedConnection);
#else
    auto applyPalette = [this](const QPalette &palette) {
        setTheme(palette.color(QPalette::Window).lightness() < 128 ? ThemeId::Dark : ThemeId::Light);
    };

    applyPalette(qGuiApp->palette());

    connect(qGuiApp, &QGuiApplication::paletteChanged, this, applyPalette, Qt::QueuedConnection);
#endif
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
