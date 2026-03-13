#ifndef APP_LANGUAGE_UTILS_H
#define APP_LANGUAGE_UTILS_H

#include "yacreader_global_gui.h"

#include <QCoreApplication>
#include <QDir>
#include <QLocale>
#include <QRegularExpression>
#include <QSet>
#include <QTranslator>

#include <algorithm>

namespace YACReader::UiLanguage {

struct LanguageOption {
    QString code;
    QString displayName;
};

inline QStringList translationDirectories()
{
    QStringList directories;
#if defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
    directories << QString(DATADIR) + "/yacreader/languages";
#endif
    directories << QCoreApplication::applicationDirPath() + "/languages";
    directories.removeDuplicates();
    return directories;
}

inline bool loadFromLocale(QTranslator &translator, const QString &prefix, const QLocale &locale)
{
    const auto dirs = translationDirectories();
    for (const auto &dir : dirs) {
        if (translator.load(locale, prefix, "_", dir))
            return true;
    }

    return false;
}

inline bool loadTranslator(QTranslator &translator, const QString &prefix, const QString &languageOverride = QString())
{
    auto selectedLanguage = languageOverride.trimmed();
    if (!selectedLanguage.isEmpty()) {
        selectedLanguage.replace('-', '_');

        if (loadFromLocale(translator, prefix, QLocale(selectedLanguage)))
            return true;

        auto baseLanguage = selectedLanguage.section('_', 0, 0);
        if (baseLanguage != selectedLanguage && loadFromLocale(translator, prefix, QLocale(baseLanguage)))
            return true;
    }

    return loadFromLocale(translator, prefix, QLocale());
}

inline QTranslator &appTranslator()
{
    static QTranslator translator;
    return translator;
}

inline bool applyLanguage(const QString &prefix, const QString &languageOverride = QString())
{
    auto &translator = appTranslator();
    QCoreApplication::removeTranslator(&translator);
    const auto loaded = loadTranslator(translator, prefix, languageOverride);
    QCoreApplication::installTranslator(&translator);
    return loaded;
}

inline QString languageDisplayName(const QString &languageCode)
{
    QLocale locale(languageCode);

    QString languageName = locale.nativeLanguageName();
    if (languageName.isEmpty())
        languageName = QLocale::languageToString(locale.language());

    if (languageName.isEmpty())
        languageName = languageCode;

    QString territoryName;
    if (locale.territory() != QLocale::AnyTerritory)
        territoryName = locale.nativeTerritoryName();

    if (!territoryName.isEmpty())
        return QString("%1 (%2)").arg(languageName, territoryName);

    return languageName;
}

inline QList<LanguageOption> availableLanguages(const QString &prefix)
{
    QSet<QString> languageCodes;
    const auto pattern = QRegularExpression(
            "^" + QRegularExpression::escape(prefix) + "_(.+)\\.qm$");

    const auto dirs = translationDirectories();
    for (const auto &dirPath : dirs) {
        QDir dir(dirPath);
        if (!dir.exists())
            continue;

        const auto files = dir.entryList(
                QStringList { prefix + "_*.qm" }, QDir::Files, QDir::Name);
        for (const auto &file : files) {
            auto match = pattern.match(file);
            if (match.hasMatch())
                languageCodes.insert(match.captured(1));
        }
    }

    QList<LanguageOption> options;
    for (const auto &languageCode : languageCodes) {
        options.append({ languageCode, languageDisplayName(languageCode) });
    }

    std::sort(options.begin(), options.end(), [](const LanguageOption &lhs, const LanguageOption &rhs) {
        return lhs.displayName.localeAwareCompare(rhs.displayName) < 0;
    });

    return options;
}

} // namespace YACReader::UiLanguage

#endif // APP_LANGUAGE_UTILS_H
