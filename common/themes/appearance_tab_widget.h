#ifndef APPEARANCE_TAB_WIDGET_H
#define APPEARANCE_TAB_WIDGET_H

#include "theme_variant.h"

#include <QJsonObject>
#include <QPointer>
#include <QWidget>
#include <functional>
#include <optional>

class AppearanceConfiguration;
class QComboBox;
class QPushButton;
class ThemeEditorDialog;
class ThemeRepository;

class AppearanceTabWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AppearanceTabWidget(
            AppearanceConfiguration *config,
            ThemeRepository *repository,
            std::function<QJsonObject()> currentThemeJson,
            std::function<void(const QJsonObject &)> applyTheme,
            QWidget *parent = nullptr);

private:
    AppearanceConfiguration *config;
    ThemeRepository *repository;
    std::function<QJsonObject()> currentThemeJson;
    std::function<void(const QJsonObject &)> applyTheme;
    QPointer<ThemeEditorDialog> themeEditor;

    // One row per picker; shown/hidden based on active mode
    QWidget *lightRow = nullptr;
    QWidget *darkRow = nullptr;
    QWidget *customRow = nullptr;

    QComboBox *lightCombo = nullptr;
    QComboBox *darkCombo = nullptr;
    QComboBox *customCombo = nullptr;

    QPushButton *lightDeleteBtn = nullptr;
    QPushButton *darkDeleteBtn = nullptr;
    QPushButton *customDeleteBtn = nullptr;

    // Populate a combo with themes, filtered strictly by variant (or all if nullopt).
    void populateCombo(QComboBox *combo, std::optional<ThemeVariant> variantFilter, const QString &selectedId);
    void repopulateCombos();
    void updateDeleteButton(QComboBox *combo, QPushButton *btn);
    void updateModeRows();
    void importTheme();
    void deleteTheme(QComboBox *combo, QPushButton *deleteBtn);
};

#endif // APPEARANCE_TAB_WIDGET_H
