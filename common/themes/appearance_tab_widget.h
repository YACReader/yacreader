#ifndef APPEARANCE_TAB_WIDGET_H
#define APPEARANCE_TAB_WIDGET_H

#include <QJsonObject>
#include <QPointer>
#include <QWidget>
#include <functional>

class AppearanceConfiguration;
class ThemeEditorDialog;

class AppearanceTabWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AppearanceTabWidget(
            AppearanceConfiguration *config,
            std::function<QJsonObject()> currentThemeJson,
            std::function<void(const QJsonObject &)> applyTheme,
            QWidget *parent = nullptr);

private:
    AppearanceConfiguration *config;
    std::function<QJsonObject()> currentThemeJson;
    std::function<void(const QJsonObject &)> applyTheme;
    QPointer<ThemeEditorDialog> themeEditor;
};

#endif // APPEARANCE_TAB_WIDGET_H
