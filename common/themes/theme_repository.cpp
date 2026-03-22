#include "theme_repository.h"
#include "theme_json_utils.h"

#include <algorithm>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUuid>

namespace {
QString builtinNameFromFileName(QString fileName)
{
    if (fileName.endsWith(".json"))
        fileName.chop(5);

    if (fileName.startsWith("builtin_"))
        fileName.remove(0, 8);

    return fileName;
}

int builtinSortRank(const QString &name)
{
    if (name == QStringLiteral("classic"))
        return 0;
    if (name == QStringLiteral("light"))
        return 1;
    if (name == QStringLiteral("dark"))
        return 2;
    return 3;
}
}

ThemeRepository::ThemeRepository(const QString &qrcPrefix, const QString &userThemesDir, const QString &targetApp)
    : qrcPrefix(qrcPrefix), userThemesDir(userThemesDir), targetApp(targetApp)
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
    for (const auto &b : builtins) {
        if (b.id != themeId)
            continue;

        QJsonObject json = readJsonFile(b.resourcePath);
        if (json.isEmpty())
            return { };

        auto meta = json["meta"].toObject();
        meta["id"] = b.id;
        json["meta"] = meta;
        return json;
    }

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
    }

    // Always stamp targetApp so saved themes are always identifiable
    if (metaObj["targetApp"].toString().isEmpty())
        metaObj["targetApp"] = targetApp;

    themeJson["meta"] = metaObj;

    // Extract uuid from "user/<uuid>"
    const QString uuid = id.mid(5); // skip "user/"
    const QString filePath = filePathForUserTheme(uuid);

    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(serializeNormalizedThemeJson(themeJson));
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

QString ThemeRepository::importThemeFromFile(const QString &filePath, QString *errorMessage)
{
    QJsonObject json = readJsonFile(filePath);
    if (json.isEmpty()) {
        if (errorMessage)
            *errorMessage = QObject::tr("The file could not be read or is not valid JSON.");
        return { };
    }

    // Check that the theme targets the correct application
    const auto metaIn = json["meta"].toObject();
    const QString themeTargetApp = metaIn["targetApp"].toString();
    if (!themeTargetApp.isEmpty() && themeTargetApp != targetApp) {
        if (errorMessage)
            *errorMessage = QObject::tr("This theme is for %1, not %2.").arg(themeTargetApp, targetApp);
        return { };
    }

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

    QDir dir(qrcPrefix);
    QStringList builtinFiles = dir.entryList({ QStringLiteral("builtin_*.json") }, QDir::Files, QDir::Name);
    std::sort(builtinFiles.begin(), builtinFiles.end(), [](const QString &lhs, const QString &rhs) {
        const QString lhsName = builtinNameFromFileName(lhs);
        const QString rhsName = builtinNameFromFileName(rhs);
        const int lhsRank = builtinSortRank(lhsName);
        const int rhsRank = builtinSortRank(rhsName);
        if (lhsRank != rhsRank)
            return lhsRank < rhsRank;
        return lhsName < rhsName;
    });

    for (const auto &fileName : builtinFiles) {
        const QString name = builtinNameFromFileName(fileName);
        if (name.isEmpty())
            continue;

        const QString resourcePath = dir.absoluteFilePath(fileName);
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
        (meta["variant"].toString() == "light") ? ThemeVariant::Light : ThemeVariant::Dark,
        meta["targetApp"].toString(),
        meta["version"].toString()
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
