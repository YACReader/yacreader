#include "appearance_tab_widget.h"

#include "appearance_configuration.h"
#include "theme_editor_dialog.h"

#include <QButtonGroup>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QPushButton>
#include <QToolButton>
#include <QVBoxLayout>

AppearanceTabWidget::AppearanceTabWidget(
        AppearanceConfiguration *config,
        std::function<QJsonObject()> currentThemeJson,
        std::function<void(const QJsonObject &)> applyTheme,
        QWidget *parent)
    : QWidget(parent), config(config), currentThemeJson(std::move(currentThemeJson)), applyTheme(std::move(applyTheme))
{
    // Color scheme selector
    auto *modeBox = new QGroupBox(tr("Color scheme"), this);
    auto *modeLayout = new QHBoxLayout();

    auto *sysBtn = new QToolButton();
    auto *lightBtn = new QToolButton();
    auto *darkBtn = new QToolButton();

    sysBtn->setText(tr("System"));
    lightBtn->setText(tr("Light"));
    darkBtn->setText(tr("Dark"));

    sysBtn->setIcon(QIcon(":/images/appearance_config/theme-mode-system.svg"));
    lightBtn->setIcon(QIcon(":/images/appearance_config/theme-mode-light.svg"));
    darkBtn->setIcon(QIcon(":/images/appearance_config/theme-mode-dark.svg"));

    for (auto *btn : { sysBtn, lightBtn, darkBtn }) {
        btn->setCheckable(true);
        btn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        btn->setIconSize(QSize(93, 58));
        btn->setMinimumSize(115, 90);
    }

    auto *modeGroup = new QButtonGroup(this);
    modeGroup->addButton(sysBtn, static_cast<int>(ThemeMode::FollowSystem));
    modeGroup->addButton(lightBtn, static_cast<int>(ThemeMode::Light));
    modeGroup->addButton(darkBtn, static_cast<int>(ThemeMode::Dark));
    modeGroup->setExclusive(true);

    if (this->config) {
        const auto mode = this->config->selection().mode;
        if (auto *btn = modeGroup->button(static_cast<int>(mode)))
            btn->setChecked(true);
    }

    connect(modeGroup, &QButtonGroup::idClicked, this, [this](int id) {
        if (this->config)
            this->config->setMode(static_cast<ThemeMode>(id));
    });

    modeLayout->addStretch();
    modeLayout->addWidget(sysBtn);
    modeLayout->addWidget(lightBtn);
    modeLayout->addWidget(darkBtn);
    modeLayout->addStretch();
    modeBox->setLayout(modeLayout);

    // Theme editor
    auto *themeEditorBox = new QGroupBox(tr("Theme editor"), this);
    auto *themeEditorLayout = new QVBoxLayout();
    auto *openBtn = new QPushButton(tr("Open Theme Editor..."));
    themeEditorLayout->addWidget(openBtn);
    themeEditorBox->setLayout(themeEditorLayout);

    connect(openBtn, &QPushButton::clicked, this, [this]() {
        if (!themeEditor) {
            QJsonObject json = this->currentThemeJson();
            if (json.isEmpty()) {
                QMessageBox::critical(this,
                                      tr("Theme editor error"),
                                      tr("The current theme JSON could not be loaded."));
                return;
            }
            themeEditor = new ThemeEditorDialog(json, this);
            themeEditor->setAttribute(Qt::WA_DeleteOnClose);
            connect(themeEditor, &ThemeEditorDialog::themeJsonChanged, this,
                    [this](const QJsonObject &json) { this->applyTheme(json); });
        }
        themeEditor->show();
        themeEditor->raise();
        themeEditor->activateWindow();
    });

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(modeBox);
    layout->addWidget(themeEditorBox);
    layout->addStretch();
}
