#ifndef THEME_REPOSITORY_H
#define THEME_REPOSITORY_H

#include "theme_meta.h"

#include <QJsonObject>
#include <QList>
#include <QString>
#include <QStringList>
#include <QVersionNumber>

struct ThemeListEntry {
    QString id;
    QString displayName;
    ThemeVariant variant;
    bool isBuiltin;
};

class ThemeRepository
{
public:
    explicit ThemeRepository(const QString &qrcPrefix, const QString &userThemesDir, const QString &targetApp, const QVersionNumber &minimumThemeVersion);

    QList<ThemeListEntry> availableThemes() const;
    bool contains(const QString &themeId) const;
    bool isCompatible(const QString &themeId) const;
    QStringList missingKeys(const QJsonObject &themeJson) const;
    QJsonObject referenceThemeJson() const { return referenceJson; }
    QJsonObject loadThemeJson(const QString &themeId) const;

    QString saveUserTheme(QJsonObject themeJson);
    bool deleteUserTheme(const QString &themeId);
    QString importThemeFromFile(const QString &filePath, QString *errorMessage = nullptr);

    void refresh();

private:
    QString qrcPrefix;
    QString userThemesDir;
    QString targetApp;
    QVersionNumber minimumThemeVersion;
    QJsonObject referenceJson;

    struct BuiltinEntry {
        QString id;
        QString resourcePath;
        ThemeMeta meta;
    };
    QList<BuiltinEntry> builtins;

    struct UserEntry {
        QString id;
        QString filePath;
        ThemeMeta meta;
    };
    QList<UserEntry> userThemes;

    void scanBuiltins();
    void scanUserThemes();
    bool isCompatible(const ThemeMeta &meta) const;
    static ThemeMeta extractMeta(const QJsonObject &json);
    static QJsonObject readJsonFile(const QString &path);
    QString filePathForUserTheme(const QString &uuid) const;
};

#endif // THEME_REPOSITORY_H
