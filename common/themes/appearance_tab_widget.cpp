#include "appearance_tab_widget.h"

#include "appearance_configuration.h"
#include "theme_editor_dialog.h"
#include "theme_repository.h"

#include <QButtonGroup>
#include <QComboBox>
#include <QFileDialog>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QToolButton>
#include <QVBoxLayout>

// Select the item in combo whose UserRole data matches id (no-op if not found).
static void selectInCombo(QComboBox *combo, const QString &id)
{
    for (int i = 0; i < combo->count(); ++i) {
        if (combo->itemData(i).toString() == id) {
            combo->setCurrentIndex(i);
            return;
        }
    }
}

AppearanceTabWidget::AppearanceTabWidget(
        AppearanceConfiguration *config,
        ThemeRepository *repository,
        std::function<QJsonObject()> currentThemeJson,
        std::function<void(const QJsonObject &)> applyTheme,
        QWidget *parent)
    : QWidget(parent), config(config), repository(repository), currentThemeJson(std::move(currentThemeJson)), applyTheme(std::move(applyTheme))
{
    // --- Color scheme selector ---
    auto *modeBox = new QGroupBox(tr("Color scheme"), this);
    auto *modeLayout = new QHBoxLayout();

    auto *sysBtn = new QToolButton();
    auto *lightBtn = new QToolButton();
    auto *darkBtn = new QToolButton();
    auto *customBtn = new QToolButton();

    sysBtn->setText(tr("System"));
    lightBtn->setText(tr("Light"));
    darkBtn->setText(tr("Dark"));
    customBtn->setText(tr("Custom"));

    sysBtn->setIcon(QIcon(":/images/appearance_config/theme-mode-system.svg"));
    lightBtn->setIcon(QIcon(":/images/appearance_config/theme-mode-light.svg"));
    darkBtn->setIcon(QIcon(":/images/appearance_config/theme-mode-dark.svg"));
    customBtn->setIcon(QIcon(":/images/appearance_config/theme-mode-custom.svg"));

    for (auto *btn : { sysBtn, lightBtn, darkBtn, customBtn }) {
        btn->setCheckable(true);
        btn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        btn->setIconSize(QSize(93, 58));
        btn->setMinimumSize(115, 90);
    }

    auto *modeGroup = new QButtonGroup(this);
    modeGroup->addButton(sysBtn, static_cast<int>(ThemeMode::FollowSystem));
    modeGroup->addButton(lightBtn, static_cast<int>(ThemeMode::Light));
    modeGroup->addButton(darkBtn, static_cast<int>(ThemeMode::Dark));
    modeGroup->addButton(customBtn, static_cast<int>(ThemeMode::ForcedTheme));
    modeGroup->setExclusive(true);

    if (this->config) {
        const auto mode = this->config->selection().mode;
        if (auto *btn = modeGroup->button(static_cast<int>(mode)))
            btn->setChecked(true);
    }

    connect(modeGroup, &QButtonGroup::idClicked, this, [this](int id) {
        if (this->config) {
            this->config->setMode(static_cast<ThemeMode>(id));
            updateModeRows();
        }
    });

    modeLayout->addStretch();
    modeLayout->addWidget(sysBtn);
    modeLayout->addWidget(lightBtn);
    modeLayout->addWidget(darkBtn);
    modeLayout->addWidget(customBtn);
    modeLayout->addStretch();
    modeBox->setLayout(modeLayout);

    // --- Theme selection ---
    // Each row: [label fixed] [combo expanding] [Remove btn]
    // Rows are shown/hidden based on mode — only relevant ones are visible.
    auto makeRow = [](const QString &label, QComboBox *&combo, QPushButton *&deleteBtn) {
        auto *row = new QWidget();
        auto *hl = new QHBoxLayout(row);
        hl->setContentsMargins(0, 0, 0, 0);

        auto *lbl = new QLabel(label);
        lbl->setFixedWidth(52);

        combo = new QComboBox();
        combo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

        deleteBtn = new QPushButton(tr("Remove"));
        deleteBtn->setEnabled(false);
        deleteBtn->setToolTip(tr("Remove this user-imported theme"));

        hl->addWidget(lbl);
        hl->addWidget(combo);
        hl->addWidget(deleteBtn);
        return row;
    };

    lightRow = makeRow(tr("Light:"), lightCombo, lightDeleteBtn);
    darkRow = makeRow(tr("Dark:"), darkCombo, darkDeleteBtn);
    customRow = makeRow(tr("Custom:"), customCombo, customDeleteBtn);

    auto *importBtn = new QPushButton(tr("Import theme..."));

    auto *themeSelBox = new QGroupBox(tr("Theme"), this);
    auto *themeSelLayout = new QVBoxLayout();
    themeSelLayout->addWidget(lightRow);
    themeSelLayout->addWidget(darkRow);
    themeSelLayout->addWidget(customRow);
    themeSelLayout->addWidget(importBtn, 0, Qt::AlignRight);
    themeSelBox->setLayout(themeSelLayout);

    // Populate combos and set initial row visibility
    if (this->config && this->repository) {
        const auto &sel = this->config->selection();
        populateCombo(lightCombo, ThemeVariant::Light, sel.lightThemeId);
        populateCombo(darkCombo, ThemeVariant::Dark, sel.darkThemeId);
        populateCombo(customCombo, std::nullopt, sel.fixedThemeId);
        updateDeleteButton(lightCombo, lightDeleteBtn);
        updateDeleteButton(darkCombo, darkDeleteBtn);
        updateDeleteButton(customCombo, customDeleteBtn);
        updateModeRows();
    }

    // Combo selection → update config (live theme preview via selectionChanged chain)
    connect(lightCombo, &QComboBox::currentIndexChanged, this, [this](int) {
        if (!this->config)
            return;
        this->config->setLightThemeId(lightCombo->currentData().toString());
        updateDeleteButton(lightCombo, lightDeleteBtn);
    });
    connect(darkCombo, &QComboBox::currentIndexChanged, this, [this](int) {
        if (!this->config)
            return;
        this->config->setDarkThemeId(darkCombo->currentData().toString());
        updateDeleteButton(darkCombo, darkDeleteBtn);
    });
    connect(customCombo, &QComboBox::currentIndexChanged, this, [this](int) {
        if (!this->config)
            return;
        this->config->setFixedThemeId(customCombo->currentData().toString());
        updateDeleteButton(customCombo, customDeleteBtn);
    });

    // Delete buttons
    connect(lightDeleteBtn, &QPushButton::clicked, this,
            [this]() { deleteTheme(lightCombo, lightDeleteBtn); });
    connect(darkDeleteBtn, &QPushButton::clicked, this,
            [this]() { deleteTheme(darkCombo, darkDeleteBtn); });
    connect(customDeleteBtn, &QPushButton::clicked, this,
            [this]() { deleteTheme(customCombo, customDeleteBtn); });

    // Import
    connect(importBtn, &QPushButton::clicked, this, &AppearanceTabWidget::importTheme);

    // --- Theme editor ---
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
    layout->addWidget(themeSelBox);
    layout->addWidget(themeEditorBox);
    layout->addStretch();
}

void AppearanceTabWidget::populateCombo(QComboBox *combo,
                                        std::optional<ThemeVariant> variantFilter,
                                        const QString &selectedId)
{
    QSignalBlocker blocker(combo);
    combo->clear();
    if (!repository)
        return;

    for (const auto &entry : repository->availableThemes()) {
        if (variantFilter && entry.variant != *variantFilter)
            continue;
        combo->addItem(entry.displayName, entry.id);
    }

    // Restore selection; fall back to first item if the saved ID is no longer present
    for (int i = 0; i < combo->count(); ++i) {
        if (combo->itemData(i).toString() == selectedId) {
            combo->setCurrentIndex(i);
            return;
        }
    }
    if (combo->count() > 0)
        combo->setCurrentIndex(0);
}

void AppearanceTabWidget::repopulateCombos()
{
    if (!config)
        return;
    const auto &sel = config->selection();
    populateCombo(lightCombo, ThemeVariant::Light, sel.lightThemeId);
    populateCombo(darkCombo, ThemeVariant::Dark, sel.darkThemeId);
    populateCombo(customCombo, std::nullopt, sel.fixedThemeId);
    updateDeleteButton(lightCombo, lightDeleteBtn);
    updateDeleteButton(darkCombo, darkDeleteBtn);
    updateDeleteButton(customCombo, customDeleteBtn);
}

void AppearanceTabWidget::updateDeleteButton(QComboBox *combo, QPushButton *btn)
{
    const QString id = combo->currentData().toString();
    btn->setEnabled(!id.isEmpty() && id.startsWith(QLatin1String("user/")));
}

void AppearanceTabWidget::updateModeRows()
{
    if (!config)
        return;
    const auto mode = config->selection().mode;
    lightRow->setVisible(mode == ThemeMode::FollowSystem || mode == ThemeMode::Light);
    darkRow->setVisible(mode == ThemeMode::FollowSystem || mode == ThemeMode::Dark);
    customRow->setVisible(mode == ThemeMode::ForcedTheme);
}

void AppearanceTabWidget::importTheme()
{
    const QString path = QFileDialog::getOpenFileName(
            this, tr("Import theme"), QString(), tr("JSON files (*.json);;All files (*)"));
    if (path.isEmpty() || !repository)
        return;

    const QString id = repository->importThemeFromFile(path);
    if (id.isEmpty()) {
        QMessageBox::warning(this, tr("Import failed"),
                             tr("Could not import theme from:\n%1").arg(path));
        return;
    }

    // Detect variant of the imported theme to auto-select it in the right combo
    const QJsonObject json = repository->loadThemeJson(id);
    const bool isLight = (json["meta"].toObject()["variant"].toString() == "light");

    repopulateCombos();

    // Select in the appropriate combo → triggers currentIndexChanged → config update
    if (isLight)
        selectInCombo(lightCombo, id);
    else
        selectInCombo(darkCombo, id);

    // If in Custom mode, also select in customCombo
    if (config && config->selection().mode == ThemeMode::ForcedTheme)
        selectInCombo(customCombo, id);
}

void AppearanceTabWidget::deleteTheme(QComboBox *combo, QPushButton *deleteBtn)
{
    if (!repository)
        return;
    const QString id = combo->currentData().toString();
    if (!id.startsWith(QLatin1String("user/")))
        return;

    repository->deleteUserTheme(id);
    repopulateCombos();

    // repopulateCombos() blocked signals; manually push the new selection into config
    // so the theme resolves correctly (important when the deleted theme was active).
    const QString newId = combo->currentData().toString();
    if (combo == lightCombo && config)
        config->setLightThemeId(newId);
    else if (combo == darkCombo && config)
        config->setDarkThemeId(newId);
    else if (combo == customCombo && config)
        config->setFixedThemeId(newId);

    updateDeleteButton(combo, deleteBtn);
}
