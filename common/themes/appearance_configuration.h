#ifndef APPEARANCE_CONFIGURATION_H
#define APPEARANCE_CONFIGURATION_H

#include <QObject>
#include <QString>

enum class ThemeMode {
    FollowSystem,
    Light,
    Dark,
    ForcedTheme,
};

struct ThemeSelection {
    ThemeMode mode = ThemeMode::FollowSystem;
    QString lightThemeId = "builtin/light";
    QString darkThemeId = "builtin/dark";
    QString fixedThemeId = "builtin/classic";
};

// Persists theme selection settings to a QSettings INI file under the
// [Appearance] group. All access is on-demand (no persistent QSettings handle)
// so the caller does not need to manage a QSettings lifetime.
class AppearanceConfiguration : public QObject
{
    Q_OBJECT
public:
    explicit AppearanceConfiguration(const QString &settingsFilePath, QObject *parent = nullptr);

    ThemeSelection selection() const { return sel; }

    void setMode(ThemeMode mode);
    void setLightThemeId(const QString &id);
    void setDarkThemeId(const QString &id);
    void setFixedThemeId(const QString &id);

signals:
    void selectionChanged();

private:
    QString path;
    ThemeSelection sel;

    void load();
    void write(const QString &key, const QString &value);
};

#endif // APPEARANCE_CONFIGURATION_H
