#include "theme.h"
#include "theme_json_utils.h"
#include "theme_repository.h"

#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QTemporaryDir>
#include <QtTest>

namespace {

QJsonObject readJson(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return { };
    return QJsonDocument::fromJson(file.readAll()).object();
}

QJsonObject referenceTheme()
{
    return readJson(QDir(THEMES_DIR).absoluteFilePath("builtin_dark.json"));
}

QStringList builtinFiles()
{
    return QDir(THEMES_DIR).entryList({ "builtin_*.json" }, QDir::Files, QDir::Name);
}

QPair<QString, QString> firstColorKey(const QJsonObject &theme)
{
    for (auto section = theme.constBegin(); section != theme.constEnd(); ++section) {
        if (section.key() == "meta" || !section.value().isObject())
            continue;
        const auto keys = section.value().toObject();
        for (auto key = keys.constBegin(); key != keys.constEnd(); ++key) {
            if (isThemeHexColorString(key.value().toString()))
                return { section.key(), key.key() };
        }
    }
    return { };
}

QJsonObject withValue(QJsonObject theme, const QString &section, const QString &key, const QJsonValue &value)
{
    auto object = theme[section].toObject();
    if (value.isUndefined())
        object.remove(key);
    else
        object[key] = value;
    theme[section] = object;
    return theme;
}

QJsonObject withVersion(QJsonObject theme, const QString &version)
{
    auto meta = theme["meta"].toObject();
    meta["version"] = version;
    theme["meta"] = meta;
    return theme;
}

}

class ThemeSchemaTest : public QObject
{
    Q_OBJECT

private slots:
    void builtinsHaveTheReferenceKeys();
    void builtinsAreCompatible();
    void missingKeyIsReported();
    void missingSectionReportsEachKey();
    void wrongTypeIsReported();
    void extraKeyIsNotReported();
    void minimumVersionIsCompatible();
    void olderVersionIsIncompatible();
    void missingVersionIsIncompatible();
    void incompatibleUserThemeIsListed();
    void referenceKeysDropExtraKeys();
    void referenceKeysDropWrongTypes();
    void referenceKeysKeepMeta();
};

void ThemeSchemaTest::builtinsHaveTheReferenceKeys()
{
    const auto reference = referenceTheme();
    QVERIFY(!reference.isEmpty());

    const auto files = builtinFiles();
    QVERIFY(!files.isEmpty());
    for (const auto &file : files) {
        const auto theme = readJson(QDir(THEMES_DIR).absoluteFilePath(file));
        QVERIFY2(missingThemeKeys(reference, theme).isEmpty(), qPrintable(file + ": " + missingThemeKeys(reference, theme).join(", ")));
        QVERIFY2(missingThemeKeys(theme, reference).isEmpty(), qPrintable(file + " has extra keys: " + missingThemeKeys(theme, reference).join(", ")));
    }
}

void ThemeSchemaTest::builtinsAreCompatible()
{
    QTemporaryDir userDir;
    ThemeRepository repository(THEMES_DIR, userDir.path(), "test", minimumThemeVersion);
    const auto themes = repository.availableThemes();
    QCOMPARE(themes.size(), builtinFiles().size());
    for (const auto &theme : themes)
        QVERIFY2(repository.isCompatible(theme.id), qPrintable(theme.id));
}

void ThemeSchemaTest::missingKeyIsReported()
{
    QTemporaryDir userDir;
    ThemeRepository repository(THEMES_DIR, userDir.path(), "test", minimumThemeVersion);
    const auto reference = referenceTheme();
    const auto [section, key] = firstColorKey(reference);
    QVERIFY(!key.isEmpty());

    QCOMPARE(repository.missingKeys(withValue(reference, section, key, QJsonValue::Undefined)), QStringList { section + "." + key });
}

void ThemeSchemaTest::missingSectionReportsEachKey()
{
    QTemporaryDir userDir;
    ThemeRepository repository(THEMES_DIR, userDir.path(), "test", minimumThemeVersion);
    auto theme = referenceTheme();
    const auto [section, key] = firstColorKey(theme);
    const int keyCount = theme[section].toObject().size();
    theme.remove(section);

    const auto missing = repository.missingKeys(theme);
    QVERIFY(missing.size() >= keyCount);
    QVERIFY(missing.contains(section + "." + key));
}

void ThemeSchemaTest::wrongTypeIsReported()
{
    QTemporaryDir userDir;
    ThemeRepository repository(THEMES_DIR, userDir.path(), "test", minimumThemeVersion);
    const auto reference = referenceTheme();
    const auto [section, key] = firstColorKey(reference);
    const QStringList expected { section + "." + key };

    QCOMPARE(repository.missingKeys(withValue(reference, section, key, "not a color")), expected);
    QCOMPARE(repository.missingKeys(withValue(reference, section, key, 42)), expected);
}

void ThemeSchemaTest::extraKeyIsNotReported()
{
    QTemporaryDir userDir;
    ThemeRepository repository(THEMES_DIR, userDir.path(), "test", minimumThemeVersion);
    const auto reference = referenceTheme();
    const QString section = firstColorKey(reference).first;
    QVERIFY(repository.missingKeys(withValue(reference, section, "unknownColor", "#FFFFFF")).isEmpty());
}

void ThemeSchemaTest::minimumVersionIsCompatible()
{
    QTemporaryDir userDir;
    ThemeRepository repository(THEMES_DIR, userDir.path(), "test", minimumThemeVersion);
    const QString id = repository.saveUserTheme(withVersion(referenceTheme(), minimumThemeVersion.toString()));
    QVERIFY(repository.isCompatible(id));
}

void ThemeSchemaTest::olderVersionIsIncompatible()
{
    QTemporaryDir userDir;
    ThemeRepository repository(THEMES_DIR, userDir.path(), "test", minimumThemeVersion);
    const QString id = repository.saveUserTheme(withVersion(referenceTheme(), "9.99.0"));
    QVERIFY(repository.contains(id));
    QVERIFY(!repository.isCompatible(id));
}

void ThemeSchemaTest::missingVersionIsIncompatible()
{
    QTemporaryDir userDir;
    ThemeRepository repository(THEMES_DIR, userDir.path(), "test", minimumThemeVersion);
    auto theme = referenceTheme();
    auto meta = theme["meta"].toObject();
    meta.remove("version");
    theme["meta"] = meta;
    const QString id = repository.saveUserTheme(theme);
    QVERIFY(repository.contains(id));
    QVERIFY(!repository.isCompatible(id));
}

void ThemeSchemaTest::incompatibleUserThemeIsListed()
{
    QTemporaryDir userDir;
    ThemeRepository repository(THEMES_DIR, userDir.path(), "test", minimumThemeVersion);
    const QString id = repository.saveUserTheme(withVersion(referenceTheme(), "9.99.0"));

    bool listed = false;
    for (const auto &theme : repository.availableThemes())
        listed = listed || theme.id == id;
    QVERIFY(listed);
}

void ThemeSchemaTest::referenceKeysDropExtraKeys()
{
    const auto reference = referenceTheme();
    const QString section = firstColorKey(reference).first;
    auto theme = withValue(reference, section, "unknownColor", "#FFFFFF");
    theme["unknownSection"] = QJsonObject { { "color", "#FFFFFF" } };

    QCOMPARE(referenceThemeKeys(reference, theme), reference);
}

void ThemeSchemaTest::referenceKeysDropWrongTypes()
{
    const auto reference = referenceTheme();
    const auto [section, key] = firstColorKey(reference);
    const auto expected = withValue(reference, section, key, QJsonValue::Undefined);

    QCOMPARE(referenceThemeKeys(reference, withValue(reference, section, key, "not a color")), expected);
    QCOMPARE(referenceThemeKeys(reference, withValue(reference, section, key, 42)), expected);
}

void ThemeSchemaTest::referenceKeysKeepMeta()
{
    auto theme = withVersion(referenceTheme(), "9.99.0");
    auto meta = theme["meta"].toObject();
    meta["custom"] = "value";
    theme["meta"] = meta;

    QCOMPARE(referenceThemeKeys(referenceTheme(), theme)["meta"].toObject(), meta);
}

QTEST_GUILESS_MAIN(ThemeSchemaTest)

#include "main.moc"
