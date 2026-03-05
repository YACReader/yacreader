#include "theme_repository.h"

#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUuid>

ThemeRepository::ThemeRepository(const QString &qrcPrefix, const QString &userThemesDir)
    : qrcPrefix(qrcPrefix), userThemesDir(userThemesDir)
{
    scanBuiltins();
    scanUserThemes();
}

QList<ThemeListEntry> ThemeRepository::availableThemes() const
{
    QList<ThemeListEntry> result;
    result.reserve(builtins.size() + userThemes.size());

    for (const auto &b : builtins)
        result.append({ b.meta.id, b.meta.displayName, b.meta.variant, true });

    for (const auto &u : userThemes)
        result.append({ u.meta.id, u.meta.displayName, u.meta.variant, false });

    return result;
}

bool ThemeRepository::contains(const QString &themeId) const
{
    for (const auto &b : builtins)
        if (b.id == themeId)
            return true;

    for (const auto &u : userThemes)
        if (u.id == themeId)
            return true;

    return false;
}

QJsonObject ThemeRepository::loadThemeJson(const QString &themeId) const
{
    for (const auto &b : builtins)
        if (b.id == themeId)
            return readJsonFile(b.resourcePath);

    for (const auto &u : userThemes)
        if (u.id == themeId)
            return readJsonFile(u.filePath);

    return { };
}

QString ThemeRepository::saveUserTheme(QJsonObject themeJson)
{
    QDir().mkpath(userThemesDir);

    auto metaObj = themeJson["meta"].toObject();
    QString id = metaObj["id"].toString();

    if (id.isEmpty() || id.startsWith("builtin/")) {
        const QString uuid = QUuid::createUuid().toString(QUuid::WithoutBraces);
        id = "user/" + uuid;
        metaObj["id"] = id;
        themeJson["meta"] = metaObj;
    }

    // Extract uuid from "user/<uuid>"
    const QString uuid = id.mid(5); // skip "user/"
    const QString filePath = filePathForUserTheme(uuid);

    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(themeJson).toJson(QJsonDocument::Indented));
        file.close();
    }

    // Update cache
    refresh();

    return id;
}

bool ThemeRepository::deleteUserTheme(const QString &themeId)
{
    if (themeId.startsWith("builtin/"))
        return false;

    for (const auto &u : userThemes) {
        if (u.id == themeId) {
            const bool removed = QFile::remove(u.filePath);
            if (removed)
                refresh();
            return removed;
        }
    }

    return false;
}

QString ThemeRepository::importThemeFromFile(const QString &filePath)
{
    QJsonObject json = readJsonFile(filePath);
    if (json.isEmpty())
        return { };

    // Force a new user id regardless of what the file contains
    auto metaObj = json["meta"].toObject();
    const QString uuid = QUuid::createUuid().toString(QUuid::WithoutBraces);
    const QString id = "user/" + uuid;
    metaObj["id"] = id;
    json["meta"] = metaObj;

    return saveUserTheme(json);
}

void ThemeRepository::refresh()
{
    scanUserThemes();
}

// --- Private helpers ---

void ThemeRepository::scanBuiltins()
{
    builtins.clear();

    static const QStringList builtinNames = { "classic", "light", "dark" };

    for (const auto &name : builtinNames) {
        const QString resourcePath = qrcPrefix + "/builtin_" + name + ".json";
        const QJsonObject json = readJsonFile(resourcePath);
        if (json.isEmpty())
            continue;

        BuiltinEntry entry;
        entry.id = "builtin/" + name;
        entry.resourcePath = resourcePath;
        entry.meta = extractMeta(json);
        // Ensure the id matches the canonical form
        entry.meta.id = entry.id;
        builtins.append(entry);
    }
}

void ThemeRepository::scanUserThemes()
{
    userThemes.clear();

    QDir dir(userThemesDir);
    if (!dir.exists())
        return;

    const auto entries = dir.entryList({ "*.json" }, QDir::Files);
    for (const auto &fileName : entries) {
        const QString filePath = dir.absoluteFilePath(fileName);
        const QJsonObject json = readJsonFile(filePath);
        if (json.isEmpty())
            continue;

        ThemeMeta meta = extractMeta(json);
        if (meta.id.isEmpty()) {
            // Derive id from filename (strip .json extension)
            const QString baseName = fileName.chopped(5); // remove ".json"
            meta.id = "user/" + baseName;
        }

        UserEntry entry;
        entry.id = meta.id;
        entry.filePath = filePath;
        entry.meta = meta;
        userThemes.append(entry);
    }
}

ThemeMeta ThemeRepository::extractMeta(const QJsonObject &json)
{
    const auto meta = json["meta"].toObject();
    return ThemeMeta {
        meta["id"].toString(),
        meta["displayName"].toString(),
        (meta["variant"].toString() == "light") ? ThemeVariant::Light : ThemeVariant::Dark
    };
}

QJsonObject ThemeRepository::readJsonFile(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return { };

    const QByteArray data = file.readAll();
    QJsonParseError error;
    const QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    if (error.error != QJsonParseError::NoError)
        return { };

    return doc.object();
}

QString ThemeRepository::filePathForUserTheme(const QString &uuid) const
{
    return userThemesDir + "/" + uuid + ".json";
}
