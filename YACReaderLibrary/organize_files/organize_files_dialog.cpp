#include "organize_files_dialog.h"

#include "organize_files_journal.h"
#include "yacreader_busy_widget.h"
#include "yacreader_global.h"

#include <QAction>
#include <QApplication>
#include <QButtonGroup>
#include <QCheckBox>
#include <QClipboard>
#include <QCloseEvent>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileInfo>
#include <QFontDatabase>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QProgressBar>
#include <QPushButton>
#include <QResizeEvent>
#include <QSettings>
#include <QStackedWidget>
#include <QStyledItemDelegate>
#include <QThread>
#include <QTimer>
#include <QToolButton>
#include <QTreeWidget>
#include <QVBoxLayout>

#include <algorithm>
#include <utility>

using OrganizeFiles::ComicEntry;
using OrganizeFiles::FileFailure;
using OrganizeFiles::FileMove;
using OrganizeFiles::PlannedMove;

namespace {

constexpr int SourceRole = Qt::UserRole + 1;
constexpr int ExtensionRole = Qt::UserRole + 2;
// The path a folder row had before the user renamed it. The new path is read off
// the tree, and the old one is needed to find the moves the rename applies to.
constexpr int FolderPathRole = Qt::UserRole + 3;

// The format field is typed into, so it waits for a pause. Every other input is a
// single discrete act, so it rebuilds on the next turn of the event loop.
constexpr int PatternDebounceMs = 300;
constexpr int ImmediateMs = 0;

constexpr int LoadingPage = 0;
constexpr int PlanPage = 1;
constexpr int WorkingPage = 2;
constexpr int ResultPage = 3;

class SegmentEditDelegate : public QStyledItemDelegate
{
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        if (index.column() != 0)
            return nullptr;
        return QStyledItemDelegate::createEditor(parent, option, index);
    }

    void setEditorData(QWidget *editor, const QModelIndex &index) const override
    {
        auto *lineEdit = qobject_cast<QLineEdit *>(editor);
        if (lineEdit == nullptr) {
            QStyledItemDelegate::setEditorData(editor, index);
            return;
        }

        QString text = index.data(Qt::EditRole).toString();
        const QString extension = index.data(ExtensionRole).toString();
        if (!extension.isEmpty() && text.endsWith(extension))
            text.chop(extension.size());

        lineEdit->setText(text);
    }

    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override
    {
        auto *lineEdit = qobject_cast<QLineEdit *>(editor);
        if (lineEdit == nullptr) {
            QStyledItemDelegate::setModelData(editor, model, index);
            return;
        }

        const QString name = OrganizeFiles::sanitizeSegment(lineEdit->text());
        if (name.isEmpty())
            return;

        model->setData(index, name + index.data(ExtensionRole).toString(), Qt::EditRole);
    }
};

// Qt has no segmented control: two checkable buttons with the border between them
// collapsed. Colours come from the palette, which is what the theme system drives.
QString segmentedStyleSheet()
{
    return QStringLiteral(
            "QPushButton {"
            "  border: 1px solid palette(mid);"
            "  padding: 4px 14px;"
            "  background-color: palette(button);"
            "  color: palette(button-text);"
            "}"
            "QPushButton:hover { background-color: palette(midlight); }"
            "QPushButton:checked {"
            "  background-color: palette(highlight);"
            "  color: palette(highlighted-text);"
            "  border-color: palette(highlight);"
            "}"
            "QPushButton#organizeBaseLeft {"
            "  border-top-left-radius: 4px;"
            "  border-bottom-left-radius: 4px;"
            "  border-right: none;"
            "}"
            "QPushButton#organizeBaseRight {"
            "  border-top-right-radius: 4px;"
            "  border-bottom-right-radius: 4px;"
            "}");
}

bool isExecutable(PlannedMove::Status status)
{
    return status == PlannedMove::Status::Move || status == PlannedMove::Status::Renamed || status == PlannedMove::Status::Incomplete;
}

}

OrganizeFilesDialog::OrganizeFilesDialog(const Context &context, QSettings *settings, QWidget *parent)
    : QDialog(parent), context(context), settings(settings), planThread(nullptr), planWorker(nullptr), moveThread(nullptr), moveWorker(nullptr), undoThread(nullptr), undoWorker(nullptr), generation(0), updatingTree(false), changedLibrary(false), patternIsValid(true), moveRunning(false), undoRunning(false), planIsStale(true)
{
    qRegisterMetaType<QList<OrganizeFiles::PlannedMove>>();
    qRegisterMetaType<OrganizeFiles::Overrides>();

    setupPages();
    setupPlanWorker();

    setModal(true);
    setWindowTitle(renaming() ? tr("Rename files") : tr("Organize files"));
    resize(760, 560);

    pages->setCurrentIndex(LoadingPage);
    updateBasePathLabel();
    startBuild();
}

OrganizeFilesDialog::~OrganizeFilesDialog()
{
    if (planThread != nullptr) {
        planThread->quit();
        planThread->wait();
        delete planWorker;
        planWorker = nullptr;
    }

    if (moveThread != nullptr) {
        moveThread->quit();
        moveThread->wait();
        delete moveWorker;
        moveWorker = nullptr;
    }

    if (undoThread != nullptr) {
        undoThread->quit();
        undoThread->wait();
        delete undoWorker;
        undoWorker = nullptr;
    }
}

void OrganizeFilesDialog::setApplier(Applier applier)
{
    this->applier = std::move(applier);
}

void OrganizeFilesDialog::setUndoer(Undoer undoer)
{
    this->undoer = std::move(undoer);
}

bool OrganizeFilesDialog::libraryChanged() const
{
    return changedLibrary;
}

void OrganizeFilesDialog::setupPages()
{
    pages = new QStackedWidget;
    pages->addWidget(createLoadingPage());
    pages->addWidget(createPlanPage());
    pages->addWidget(createWorkingPage());
    pages->addWidget(createResultPage());

    auto layout = new QVBoxLayout;
    layout->addWidget(pages);
    setLayout(layout);
}

QWidget *OrganizeFilesDialog::createLoadingPage()
{
    auto page = new QWidget;
    auto layout = new QVBoxLayout;

    loadingLabel = new QLabel(tr("Preparing the preview..."));
    loadingLabel->setAlignment(Qt::AlignHCenter);

    layout->addStretch();
    layout->addWidget(new YACReaderBusyWidget, 0, Qt::AlignHCenter);
    layout->addSpacing(12);
    layout->addWidget(loadingLabel);
    layout->addStretch();

    page->setLayout(layout);
    return page;
}

QWidget *OrganizeFilesDialog::createPlanPage()
{
    auto page = new QWidget;
    auto layout = new QVBoxLayout;

    const QString patternKey = renaming() ? QStringLiteral(ORGANIZE_FILES_FILENAME_PATTERN) : QStringLiteral(ORGANIZE_FILES_PATH_PATTERN);
    const QString fallbackPattern = OrganizeFiles::defaultPattern(context.mode);

    auto formatLabel = new QLabel(renaming() ? tr("&Filename format:") : tr("&Path format:"));
    patternEdit = new QLineEdit(settings != nullptr ? settings->value(patternKey, fallbackPattern).toString() : fallbackPattern);
    patternEdit->setAccessibleName(renaming() ? tr("Filename format") : tr("Path format"));
    formatLabel->setBuddy(patternEdit);
    connect(patternEdit, &QLineEdit::textChanged, this, &OrganizeFilesDialog::patternEdited);

    auto presetsButton = new QPushButton(tr("Presets"));
    presetsButton->setAutoDefault(false);
    presetsMenu = new QMenu(presetsButton);
    presetsButton->setMenu(presetsMenu);
    rebuildPresetsMenu();

    auto insertButton = new QPushButton(tr("Insert"));
    insertButton->setAutoDefault(false);
    auto insertMenu = new QMenu(insertButton);

    const auto tokens = OrganizeFiles::knownTokens();
    for (const QString &token : tokens) {
        auto action = insertMenu->addAction(QStringLiteral("{") + token + QStringLiteral("}"));
        connect(action, &QAction::triggered, this, [this, token] {
            patternEdit->insert(QStringLiteral("{") + token + QStringLiteral("}"));
            patternEdit->setFocus();
        });
    }

    insertMenu->addSeparator();

    auto optionalAction = insertMenu->addAction(tr("Optional part < >"));
    optionalAction->setToolTip(tr("Disappears completely when the fields inside it are empty."));
    connect(optionalAction, &QAction::triggered, this, &OrganizeFilesDialog::wrapSelectionInOptionalGroup);

    auto paddedAction = insertMenu->addAction(tr("Padded number {number:000}"));
    connect(paddedAction, &QAction::triggered, this, [this] {
        patternEdit->insert(QStringLiteral("{number:000}"));
        patternEdit->setFocus();
    });

    insertMenu->addSeparator();
    connect(insertMenu->addAction(tr("Format help...")), &QAction::triggered, this, &OrganizeFilesDialog::showFormatHelp);

    insertButton->setMenu(insertMenu);

    auto formatRow = new QHBoxLayout;
    formatRow->addWidget(formatLabel);
    formatRow->addWidget(patternEdit, 1);
    formatRow->addWidget(insertButton);
    formatRow->addWidget(presetsButton);

    patternError = new QLabel;
    patternError->setWordWrap(true);
    patternError->setVisible(false);

    folderBaseButton = new QPushButton(tr("selected folder"));
    folderBaseButton->setObjectName(QStringLiteral("organizeBaseLeft"));
    folderBaseButton->setToolTip(QDir::toNativeSeparators(context.folderPath));

    rootBaseButton = new QPushButton(tr("library root"));
    rootBaseButton->setObjectName(QStringLiteral("organizeBaseRight"));
    rootBaseButton->setToolTip(QDir::toNativeSeparators(context.libraryPath));

    auto baseButtons = new QButtonGroup(this);
    baseButtons->setExclusive(true);
    for (auto *segment : { folderBaseButton, rootBaseButton }) {
        segment->setCheckable(true);
        // A QPushButton inside a QDialog claims the default-button role, which
        // would let Return trigger a setting instead of Move files.
        segment->setAutoDefault(false);
        baseButtons->addButton(segment);
    }

    baseSelector = new QWidget;
    auto segmented = new QHBoxLayout(baseSelector);
    segmented->setSpacing(0);
    segmented->setContentsMargins(0, 0, 0, 0);
    segmented->addWidget(folderBaseButton);
    segmented->addWidget(rootBaseButton);
    baseSelector->setStyleSheet(segmentedStyleSheet());

    basePathLabel = new QLabel;
    basePathLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    basePathLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

    if (context.folderPath.isEmpty()) {
        baseSelector->setVisible(false);
        rootBaseButton->setChecked(true);
    } else {
        const bool relativeToRoot = settings != nullptr ? settings->value(ORGANIZE_FILES_RELATIVE_TO_ROOT, true).toBool() : true;
        rootBaseButton->setChecked(relativeToRoot);
        folderBaseButton->setChecked(!relativeToRoot);
        // One click, not a stream of keystrokes, so there is nothing to wait for.
        connect(rootBaseButton, &QPushButton::toggled, this, [this] {
            updateBasePathLabel();
            markPlanStale(ImmediateMs);
        });
    }

    auto baseRow = new QHBoxLayout;
    baseRow->addWidget(new QLabel(tr("Move into")));
    baseRow->addSpacing(8);
    baseRow->addWidget(baseSelector);
    baseRow->addSpacing(12);
    baseRow->addWidget(basePathLabel, 1);

    overridesBanner = new QLabel;
    overridesBanner->setVisible(false);

    resetButton = new QPushButton(tr("Reset changes"));
    resetButton->setVisible(false);
    connect(resetButton, &QPushButton::clicked, this, &OrganizeFilesDialog::resetOverrides);

    removeButton = new QPushButton(tr("Remove selected"));
    removeButton->setEnabled(false);
    connect(removeButton, &QPushButton::clicked, this, &OrganizeFilesDialog::removeSelectedItems);

    showUnchangedCheck = new QCheckBox(tr("Show unchanged"));
    if (settings != nullptr)
        showUnchangedCheck->setChecked(settings->value(ORGANIZE_FILES_SHOW_UNCHANGED, false).toBool());
    connect(showUnchangedCheck, &QCheckBox::toggled, this, [this] {
        rebuildTree();
        updateStatusLine();
    });

    auto toolbar = new QHBoxLayout;
    toolbar->addWidget(removeButton);
    toolbar->addWidget(showUnchangedCheck);
    toolbar->addStretch();
    toolbar->addWidget(overridesBanner);
    toolbar->addWidget(resetButton);

    tree = new QTreeWidget;
    tree->setColumnCount(3);
    // The second header carries the verb: two neutral nouns side by side never say
    // which way the change runs.
    tree->setHeaderLabels(renaming() ? QStringList { tr("New name"), tr("Renamed from"), QString() }
                                     : QStringList { tr("New location"), tr("Moved from"), QString() });
    tree->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked | QAbstractItemView::EditKeyPressed);
    tree->setItemDelegate(new SegmentEditDelegate(tree));
    tree->setUniformRowHeights(true);
    tree->setAlternatingRowColors(true);
    tree->setSelectionMode(QAbstractItemView::ExtendedSelection);
    tree->header()->setStretchLastSection(false);
    tree->header()->setSectionResizeMode(1, QHeaderView::Stretch);
    tree->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    connect(tree, &QTreeWidget::itemChanged, this, &OrganizeFilesDialog::itemChanged);
    connect(tree, &QTreeWidget::itemSelectionChanged, this, &OrganizeFilesDialog::updateSelectionState);

    auto removeAction = new QAction(tr("Remove from list"), this);
    removeAction->setShortcut(QKeySequence::Delete);
    removeAction->setShortcutContext(Qt::WidgetShortcut);
    connect(removeAction, &QAction::triggered, this, &OrganizeFilesDialog::removeSelectedItems);
    tree->addAction(removeAction);
    tree->setContextMenuPolicy(Qt::ActionsContextMenu);

    statusLabel = new QLabel;
    warningLabel = new QLabel;
    warningLabel->setWordWrap(true);

    moveButton = new QPushButton(renaming() ? tr("Rename files") : tr("Move files"));
    moveButton->setDefault(true);
    connect(moveButton, &QPushButton::clicked, this, &OrganizeFilesDialog::startMove);

    cancelButton = new QPushButton(tr("Cancel"));
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    auto buttons = new QHBoxLayout;
    buttons->addWidget(statusLabel);
    buttons->addStretch();
    buttons->addWidget(moveButton);
    buttons->addWidget(cancelButton);

    layout->addLayout(formatRow);
    layout->addWidget(patternError);
    if (!renaming())
        layout->addLayout(baseRow);
    layout->addLayout(toolbar);
    layout->addWidget(tree, 1);
    layout->addWidget(warningLabel);
    layout->addLayout(buttons);

    page->setLayout(layout);
    return page;
}

QWidget *OrganizeFilesDialog::createWorkingPage()
{
    auto page = new QWidget;
    auto layout = new QVBoxLayout;

    progressBar = new QProgressBar;
    progressLabel = new QLabel;
    progressLabel->setWordWrap(true);
    progressLabel->setAlignment(Qt::AlignHCenter);

    layout->addStretch();
    layout->addWidget(progressBar);
    layout->addWidget(progressLabel);
    layout->addStretch();

    page->setLayout(layout);
    return page;
}

QWidget *OrganizeFilesDialog::createResultPage()
{
    auto page = new QWidget;
    auto layout = new QVBoxLayout;

    resultLabel = new QLabel;
    resultLabel->setWordWrap(true);
    resultLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    resultLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);

    resultTree = new QTreeWidget;
    resultTree->setColumnCount(3);
    resultTree->setRootIsDecorated(false);
    resultTree->setUniformRowHeights(true);
    resultTree->setAlternatingRowColors(true);
    resultTree->setEditTriggers(QAbstractItemView::NoEditTriggers);
    resultTree->setSelectionMode(QAbstractItemView::ExtendedSelection);
    resultTree->header()->setSectionResizeMode(QHeaderView::Interactive);
    resultTree->header()->setStretchLastSection(true);
    resultTree->setVisible(false);

    failureList = new QListWidget;
    failureList->setVisible(false);

    copyFailuresButton = new QPushButton(tr("Copy failure details"));
    copyFailuresButton->setVisible(false);
    connect(copyFailuresButton, &QPushButton::clicked, this, &OrganizeFilesDialog::copyFailures);

    undoButton = new QPushButton(tr("Undo"));
    connect(undoButton, &QPushButton::clicked, this, &OrganizeFilesDialog::undo);

    finishButton = new QPushButton(tr("Finish"));
    finishButton->setDefault(true);
    connect(finishButton, &QPushButton::clicked, this, &QDialog::accept);

    auto buttons = new QHBoxLayout;
    buttons->addWidget(copyFailuresButton);
    buttons->addStretch();
    buttons->addWidget(undoButton);
    buttons->addWidget(finishButton);

    layout->addWidget(resultLabel);
    layout->addWidget(resultTree, 1);
    layout->addWidget(failureList, 1);
    layout->addLayout(buttons);

    page->setLayout(layout);
    return page;
}

void OrganizeFilesDialog::setupPlanWorker()
{
    planThread = new QThread(this);
    planWorker = new OrganizeFiles::PlanWorker(context.entries, currentBase(), context.mode);
    planWorker->moveToThread(planThread);

    connect(this, &OrganizeFilesDialog::buildRequested, planWorker, &OrganizeFiles::PlanWorker::build);
    connect(planWorker, &OrganizeFiles::PlanWorker::built, this, &OrganizeFilesDialog::planBuilt);

    planThread->start();

    buildTimer = new QTimer(this);
    buildTimer->setSingleShot(true);
    connect(buildTimer, &QTimer::timeout, this, &OrganizeFilesDialog::startBuild);
}

void OrganizeFilesDialog::markPlanStale(int delayMs)
{
    planIsStale = true;

    // Dead before the click that follows the interaction is delivered — committing
    // a tree editor by clicking Move files must not run the pre-edit plan.
    moveButton->setEnabled(false);

    // QTimer::start(int) also sets the interval, so the delay is always passed.
    buildTimer->start(delayMs);
}

bool OrganizeFilesDialog::renaming() const
{
    return context.mode == OrganizeFiles::Mode::Rename;
}

QString OrganizeFilesDialog::currentBase() const
{
    if (renaming())
        return context.libraryPath;

    if (context.folderPath.isEmpty() || rootBaseButton == nullptr || rootBaseButton->isChecked())
        return context.libraryPath;

    return context.folderPath;
}

void OrganizeFilesDialog::updateBasePathLabel()
{
    const QString path = QDir::toNativeSeparators(currentBase());

    basePathLabel->setToolTip(path);
    basePathLabel->setText(basePathLabel->fontMetrics().elidedText(path, Qt::ElideMiddle, qMax(120, basePathLabel->width())));
}

QString OrganizeFilesDialog::presetsKey() const
{
    return renaming() ? QStringLiteral(ORGANIZE_FILES_FILENAME_PRESETS) : QStringLiteral(ORGANIZE_FILES_PATH_PRESETS);
}

QList<QPair<QString, QString>> OrganizeFilesDialog::userPresets() const
{
    QList<QPair<QString, QString>> presets;
    if (settings == nullptr)
        return presets;

    const int size = settings->beginReadArray(presetsKey());
    for (int i = 0; i < size; ++i) {
        settings->setArrayIndex(i);
        const QString name = settings->value(QStringLiteral("name")).toString();
        const QString pattern = settings->value(QStringLiteral("pattern")).toString();
        if (!name.isEmpty() && !pattern.isEmpty())
            presets.append({ name, pattern });
    }
    settings->endArray();

    return presets;
}

void OrganizeFilesDialog::saveUserPresets(const QList<QPair<QString, QString>> &presets)
{
    if (settings == nullptr)
        return;

    settings->remove(presetsKey());
    settings->beginWriteArray(presetsKey());
    for (int i = 0; i < presets.size(); ++i) {
        settings->setArrayIndex(i);
        settings->setValue(QStringLiteral("name"), presets.at(i).first);
        settings->setValue(QStringLiteral("pattern"), presets.at(i).second);
    }
    settings->endArray();
}

void OrganizeFilesDialog::rebuildPresetsMenu()
{
    presetsMenu->clear();

    const auto addPattern = [this](const QString &name, const QString &pattern) {
        auto action = presetsMenu->addAction(name);
        connect(action, &QAction::triggered, this, [this, pattern] { patternEdit->setText(pattern); });
    };

    const auto custom = userPresets();
    for (const auto &preset : custom)
        addPattern(preset.first, preset.second);

    if (!custom.isEmpty()) {
        auto removeMenu = presetsMenu->addMenu(tr("Remove preset"));
        for (const auto &preset : custom) {
            const QString name = preset.first;
            connect(removeMenu->addAction(name), &QAction::triggered, this, [this, name] {
                auto presets = userPresets();
                presets.removeIf([&name](const QPair<QString, QString> &preset) { return preset.first == name; });
                saveUserPresets(presets);
                rebuildPresetsMenu();
            });
        }
        presetsMenu->addSeparator();
    }

    const auto builtIn = OrganizeFiles::presets(context.mode);
    for (const auto &preset : builtIn)
        addPattern(preset.first, preset.second);

    presetsMenu->addSeparator();

    auto saveAction = presetsMenu->addAction(tr("Save current format as preset..."));
    saveAction->setEnabled(settings != nullptr);
    connect(saveAction, &QAction::triggered, this, &OrganizeFilesDialog::saveCurrentPatternAsPreset);

    const QString fallback = OrganizeFiles::defaultPattern(context.mode);
    connect(presetsMenu->addAction(tr("Reset to default format")), &QAction::triggered, this, [this, fallback] { patternEdit->setText(fallback); });
}

void OrganizeFilesDialog::saveCurrentPatternAsPreset()
{
    bool accepted = false;
    const QString name = QInputDialog::getText(this, tr("Save preset"), tr("Preset name:"), QLineEdit::Normal, QString(), &accepted).trimmed();
    if (!accepted || name.isEmpty())
        return;

    auto presets = userPresets();
    presets.removeIf([&name](const QPair<QString, QString> &preset) { return preset.first == name; });
    presets.append({ name, patternEdit->text() });
    saveUserPresets(presets);
    rebuildPresetsMenu();
}

void OrganizeFilesDialog::patternEdited()
{
    const auto invalid = OrganizeFiles::invalidTokens(patternEdit->text());

    const bool createsFolders = renaming() && OrganizeFiles::patternCreatesFolders(patternEdit->text());
    patternIsValid = invalid.isEmpty() && !createsFolders;

    if (!patternIsValid) {
        if (createsFolders)
            patternError->setText(tr("A filename format cannot contain \"/\". Use Organize files to move comics into folders."));
        else
            patternError->setText(tr("This format cannot be used: %1").arg(invalid.join(QStringLiteral(" "))));
        patternError->setVisible(true);
        moveButton->setEnabled(false);
        // Bumping the generation drops a build still in flight, so it cannot land
        // and report itself current while the format on screen is invalid.
        planIsStale = true;
        ++generation;
        buildTimer->stop();
        return;
    }

    patternError->setVisible(false);
    scheduleBuild();
}

void OrganizeFilesDialog::scheduleBuild()
{
    markPlanStale(PatternDebounceMs);
}

void OrganizeFilesDialog::startBuild()
{
    buildTimer->stop();
    emit buildRequested(patternEdit->text(), currentBase(), overrides, ++generation);
}

void OrganizeFilesDialog::planBuilt(const QList<OrganizeFiles::PlannedMove> &moves, quint64 buildGeneration)
{
    // An older build finishing after a newer one was asked for; the newer result
    // is still on its way.
    if (buildGeneration != generation)
        return;

    // The user interacted again while this build was in flight (the timer has not
    // fired yet, so the generation still matches). The pending rebuild covers it.
    if (buildTimer->isActive())
        return;

    plan = moves;
    planIsStale = false;

    planDestinations.clear();
    for (const auto &move : std::as_const(plan))
        planDestinations.insert(move.sourceAbsolute, move.destinationRelative);

    rebuildTree();
    updateStatusLine();

    if (pages->currentIndex() == LoadingPage)
        pages->setCurrentIndex(PlanPage);
}

bool OrganizeFilesDialog::isFileItem(QTreeWidgetItem *item) const
{
    return item != nullptr && item->data(0, SourceRole).isValid();
}

void OrganizeFilesDialog::collectFileItems(QTreeWidgetItem *item, QList<QTreeWidgetItem *> &out) const
{
    if (isFileItem(item)) {
        out.append(item);
        return;
    }

    for (int i = 0; i < item->childCount(); ++i)
        collectFileItems(item->child(i), out);
}

QString OrganizeFilesDialog::relativePathForItem(QTreeWidgetItem *item) const
{
    QStringList segments;
    for (QTreeWidgetItem *node = item; node != nullptr; node = node->parent()) {
        const QString clean = OrganizeFiles::sanitizeSegment(node->text(0));
        if (!clean.isEmpty())
            segments.prepend(clean);
    }

    return segments.join(QLatin1Char('/'));
}

void OrganizeFilesDialog::rebuildTree()
{
    updatingTree = true;

    tree->clear();
    newFolderCount = 0;

    const QDir libraryDir(context.libraryPath);
    const QString base = currentBase();
    const bool showUnchanged = showUnchangedCheck->isChecked();

    const QColor mutedColor = tree->palette().color(QPalette::Disabled, QPalette::Text);
    const bool dark = tree->palette().color(QPalette::Base).lightness() < 128;
    const QColor warningColor = dark ? QColor(0xE0, 0xA0, 0x30) : QColor(0xB2, 0x6B, 0x00);
    const QColor errorColor = dark ? QColor(0xE0, 0x6C, 0x5A) : QColor(0xC0, 0x39, 0x2B);

    QFont statusFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    statusFont.setPointSize(tree->font().pointSize());

    const auto setStatus = [&](QTreeWidgetItem *item, const QString &glyph, const QString &text, const QColor &color) {
        item->setText(2, glyph.isEmpty() ? text : glyph + QLatin1Char(' ') + text);
        item->setForeground(2, color);
        item->setFont(2, statusFont);
        item->setTextAlignment(2, Qt::AlignRight | Qt::AlignVCenter);
    };

    QList<PlannedMove> visible;
    for (const auto &move : std::as_const(plan)) {
        if (move.status == PlannedMove::Status::Excluded)
            continue;
        if (move.status == PlannedMove::Status::Unchanged && !showUnchanged)
            continue;
        visible.append(move);
    }

    std::sort(visible.begin(), visible.end(), [](const PlannedMove &a, const PlannedMove &b) {
        return a.destinationRelative.compare(b.destinationRelative, Qt::CaseInsensitive) < 0;
    });

    QHash<QString, QTreeWidgetItem *> folders;

    for (const auto &move : std::as_const(visible)) {
        const auto segments = move.destinationRelative.split(QLatin1Char('/'), Qt::SkipEmptyParts);
        if (segments.isEmpty())
            continue;

        QTreeWidgetItem *parent = nullptr;
        QString cumulative;
        for (int i = 0; i < segments.size() - 1; ++i) {
            cumulative += (cumulative.isEmpty() ? QString() : QStringLiteral("/")) + segments.at(i);

            QTreeWidgetItem *&folderItem = folders[cumulative];
            if (folderItem == nullptr) {
                folderItem = parent != nullptr ? new QTreeWidgetItem(parent) : new QTreeWidgetItem(tree);
                folderItem->setText(0, segments.at(i));
                folderItem->setData(0, FolderPathRole, cumulative);

                QFont folderFont = folderItem->font(0);
                folderFont.setBold(true);
                folderItem->setFont(0, folderFont);

                if (!renaming()) {
                    folderItem->setFlags(folderItem->flags() | Qt::ItemIsEditable);

                    const QString absolute = base + QLatin1Char('/') + cumulative;
                    auto known = folderExistsCache.find(absolute);
                    if (known == folderExistsCache.end())
                        known = folderExistsCache.insert(absolute, QDir(absolute).exists());

                    if (!known.value()) {
                        newFolderCount++;
                        setStatus(folderItem, QString(), tr("new folder"), mutedColor);
                        folderItem->setToolTip(2, tr("This folder does not exist yet. It will be created."));
                    }
                }
            }
            parent = folderItem;
        }

        auto fileItem = parent != nullptr ? new QTreeWidgetItem(parent) : new QTreeWidgetItem(tree);
        fileItem->setText(0, segments.last());
        fileItem->setData(0, SourceRole, move.sourceAbsolute);
        fileItem->setData(0, ExtensionRole, QFileInfo(segments.last()).suffix().isEmpty() ? QString() : QLatin1Char('.') + QFileInfo(segments.last()).suffix());

        // In rename mode the folder part is identical on both sides, so printing
        // the whole path again would only repeat the tree above it.
        fileItem->setText(1, renaming() ? QFileInfo(move.sourceAbsolute).fileName() : libraryDir.relativeFilePath(move.sourceAbsolute));
        fileItem->setForeground(1, mutedColor);
        fileItem->setToolTip(1, QDir::toNativeSeparators(move.sourceAbsolute));

        if (!move.note.isEmpty())
            fileItem->setToolTip(2, move.note);

        // Only the exceptions are marked. A row with no marker is the normal case,
        // and marking that too would bury the rows that need attention.
        switch (move.status) {
        case PlannedMove::Status::Missing:
            setStatus(fileItem, QStringLiteral("x"), tr("file not found"), errorColor);
            fileItem->setToolTip(2, tr("This comic is in the library but not on disk. It is skipped."));
            fileItem->setDisabled(true);
            break;
        case PlannedMove::Status::Renamed:
            setStatus(fileItem, QStringLiteral("!"), tr("name in use"), warningColor);
            break;
        case PlannedMove::Status::Incomplete:
            setStatus(fileItem, QStringLiteral("?"), tr("no metadata"), warningColor);
            break;
        case PlannedMove::Status::Unchanged:
            setStatus(fileItem, QStringLiteral("="), tr("already here"), mutedColor);
            fileItem->setToolTip(2, tr("This file is already in the right place."));
            break;
        default:
            if (move.edited)
                setStatus(fileItem, QString(), tr("edited"), mutedColor);
            break;
        }

        if (move.status != PlannedMove::Status::Missing)
            fileItem->setFlags(fileItem->flags() | Qt::ItemIsEditable);
    }

    tree->expandAll();
    tree->resizeColumnToContents(0);

    // A deeply indented file name sitting flush against a flat path reads as one
    // string, so the first column keeps a gutter and never eats the second one.
    const int viewportWidth = tree->viewport()->width();
    const int widest = viewportWidth > 0 ? viewportWidth * 3 / 5 : 420;
    tree->setColumnWidth(0, qMin(tree->columnWidth(0) + 56, widest));

    updatingTree = false;

    updateSelectionState();
}

void OrganizeFilesDialog::updateStatusLine()
{
    int willMove = 0;
    int unchanged = 0;
    int renamed = 0;
    int excluded = 0;
    int missing = 0;

    for (const auto &move : std::as_const(plan)) {
        switch (move.status) {
        case PlannedMove::Status::Unchanged:
            unchanged++;
            break;
        case PlannedMove::Status::Excluded:
            excluded++;
            break;
        case PlannedMove::Status::Missing:
            missing++;
            break;
        case PlannedMove::Status::Renamed:
            renamed++;
            willMove++;
            break;
        default:
            willMove++;
            break;
        }
    }

    QStringList parts;
    parts << (renaming() ? tr("%n will be renamed", "", willMove) : tr("%n will move", "", willMove));
    parts << tr("%n unchanged", "", unchanged);
    if (renamed > 0)
        parts << tr("%n renamed", "", renamed);
    if (excluded > 0)
        parts << tr("%n removed", "", excluded);
    if (missing > 0)
        parts << tr("%n missing", "", missing);
    if (newFolderCount > 0)
        parts << tr("%n new folder(s)", "", newFolderCount);

    statusLabel->setText(parts.join(QStringLiteral(" · ")));

    const bool hasOverrides = !overrides.isEmpty();
    overridesBanner->setText(tr("%n manual change(s) kept", "", overrides.size()));
    overridesBanner->setVisible(hasOverrides);
    resetButton->setVisible(hasOverrides);

    if (willMove == 0) {
        warningLabel->setText(renaming() ? tr("Nothing would be renamed with this format.")
                                         : tr("Nothing would move with this format."));
    } else if (renaming()) {
        warningLabel->setText(tr("%n file(s) will be renamed. The folders do not change. You can undo it afterwards.", "", willMove));
    } else {
        warningLabel->setText(tr("%n file(s) will move into %1. This changes your files on disk. You can undo it afterwards.", "", willMove)
                                      .arg(QDir::toNativeSeparators(currentBase())));
    }

    // planIsStale keeps the button off while a rebuild is pending or in flight:
    // the tree on screen is the plan from before the last interaction.
    moveButton->setEnabled(willMove > 0 && patternIsValid && !moveRunning && !planIsStale);
}

void OrganizeFilesDialog::updateSelectionState()
{
    removeButton->setEnabled(!tree->selectedItems().isEmpty());
}

void OrganizeFilesDialog::itemChanged(QTreeWidgetItem *item, int column)
{
    if (updatingTree || column != 0)
        return;

    captureOverrides(item);

    // The item delegate is still closing its editor over this item, so the tree
    // cannot be rebuilt before the event loop comes back around.
    markPlanStale(ImmediateMs);
}

void OrganizeFilesDialog::captureOverrides(QTreeWidgetItem *item)
{
    if (isFileItem(item)) {
        const QString source = item->data(0, SourceRole).toString();
        const QString path = relativePathForItem(item);

        if (!path.isEmpty() && path != planDestinations.value(source))
            overrides[source].destinationRelative = path;

        return;
    }

    // A folder rename is applied to the plan, not read off the tree: "Show
    // unchanged" hides rows that belong to the folder just as much.
    const QString oldPath = item->data(0, FolderPathRole).toString();
    const QString newPath = relativePathForItem(item);

    if (oldPath.isEmpty() || newPath.isEmpty() || oldPath == newPath)
        return;

    const QString prefix = oldPath + QLatin1Char('/');

    for (const auto &move : std::as_const(plan)) {
        // A missing comic has its source path here, not a planned destination, and
        // an override on it would do nothing but inflate the count of manual changes.
        if (move.status == PlannedMove::Status::Missing)
            continue;

        if (!move.destinationRelative.startsWith(prefix))
            continue;

        overrides[move.sourceAbsolute].destinationRelative = newPath + QLatin1Char('/') + move.destinationRelative.mid(prefix.size());
    }
}

void OrganizeFilesDialog::removeSelectedItems()
{
    const auto selected = tree->selectedItems();
    if (selected.isEmpty())
        return;

    QList<QTreeWidgetItem *> fileItems;
    for (auto *item : selected)
        collectFileItems(item, fileItems);

    for (auto *fileItem : std::as_const(fileItems))
        overrides[fileItem->data(0, SourceRole).toString()].excluded = true;

    markPlanStale(ImmediateMs);
}

void OrganizeFilesDialog::resetOverrides()
{
    overrides.clear();
    markPlanStale(ImmediateMs);
}

QList<FileMove> OrganizeFilesDialog::movesToExecute() const
{
    const QDir baseDir(currentBase());

    QList<FileMove> moves;
    for (const auto &move : std::as_const(plan)) {
        if (!isExecutable(move.status))
            continue;

        FileMove fileMove;
        fileMove.comicId = move.comicId;
        fileMove.source = move.sourceAbsolute;
        fileMove.destination = QDir::cleanPath(baseDir.absoluteFilePath(move.destinationRelative));
        moves.append(fileMove);
    }

    return moves;
}

void OrganizeFilesDialog::saveSettings()
{
    if (settings == nullptr)
        return;

    if (patternIsValid)
        settings->setValue(renaming() ? ORGANIZE_FILES_FILENAME_PATTERN : ORGANIZE_FILES_PATH_PATTERN, patternEdit->text());
    settings->setValue(ORGANIZE_FILES_SHOW_UNCHANGED, showUnchangedCheck->isChecked());
    if (!renaming() && !context.folderPath.isEmpty())
        settings->setValue(ORGANIZE_FILES_RELATIVE_TO_ROOT, rootBaseButton->isChecked());
}

void OrganizeFilesDialog::startMove()
{
    // The button is disabled in all of these cases. This is the second lock: a click
    // that was already on its way when the state changed must not get through.
    if (planIsStale || !patternIsValid || moveRunning)
        return;

    const auto moves = movesToExecute();
    if (moves.isEmpty())
        return;

    saveSettings();
    lastRequestedMoves = moves;

    moveRunning = true;
    moveButton->setEnabled(false);

    progressBar->setRange(0, moves.size());
    progressBar->setValue(0);
    progressLabel->clear();
    pages->setCurrentIndex(WorkingPage);

    moveWorker = new OrganizeFiles::MoveWorker(context.libraryPath, currentBase(), moves, !renaming());
    // The database phase runs on the worker thread too; on the GUI thread it froze
    // the window while the progress bar stood at 100%.
    moveWorker->setApplier(applier);
    moveThread = new QThread(this);
    moveWorker->moveToThread(moveThread);

    connect(moveThread, &QThread::started, moveWorker, &OrganizeFiles::MoveWorker::process);
    connect(moveWorker, &OrganizeFiles::MoveWorker::progress, this, &OrganizeFilesDialog::moveProgress);
    connect(moveWorker, &OrganizeFiles::MoveWorker::updatingLibrary, this, &OrganizeFilesDialog::showUpdatingLibrary);
    connect(moveWorker, &OrganizeFiles::MoveWorker::finished, this, &OrganizeFilesDialog::moveFinished);

    moveThread->start();
}

void OrganizeFilesDialog::moveProgress(int done, int total, const QString &currentFile)
{
    progressBar->setRange(0, total);
    progressBar->setValue(done);
    progressLabel->setText(tr("Moving %1 of %2\n%3").arg(done).arg(total).arg(QDir::toNativeSeparators(currentFile)));
}

void OrganizeFilesDialog::showUpdatingLibrary()
{
    progressBar->setRange(0, 0);
    progressLabel->setText(tr("Updating the library..."));
}

void OrganizeFilesDialog::showFailures(const QList<FileFailure> &failures)
{
    failureList->clear();
    for (const auto &failure : failures)
        failureList->addItem(QDir::toNativeSeparators(failure.path) + QStringLiteral(" — ") + failure.reason);

    // The failures are presented in the result table. This hidden list remains the
    // source for Copy the list, so detailed diagnostics are still easy to share.
    failureList->setVisible(false);
    copyFailuresButton->setVisible(!failures.isEmpty());
}

void OrganizeFilesDialog::showCompletedMoves(const QList<FileMove> &moves, const QList<FileFailure> &failures, bool restored)
{
    resultTree->clear();

    if (renaming()) {
        resultTree->setHeaderLabels(restored ? QStringList { tr("Restored name"), tr("Moved back from"), tr("Status") }
                                             : QStringList { tr("Final name"), tr("Previous name"), tr("Status") });
    } else {
        resultTree->setHeaderLabels(restored ? QStringList { tr("Restored location"), tr("Moved back from"), tr("Status") }
                                             : QStringList { tr("Final location"), tr("Previous location"), tr("Status") });
    }

    const QDir libraryDir(context.libraryPath);
    const auto displayPath = [this, &libraryDir](const QString &path) {
        return renaming() ? QFileInfo(path).fileName()
                          : QDir::toNativeSeparators(libraryDir.relativeFilePath(path));
    };
    const auto addRow = [this, &displayPath](const QString &finalPath, const QString &previousPath, const QString &status, const QColor &color = QColor()) {
        auto *item = new QTreeWidgetItem(resultTree);
        item->setText(0, displayPath(finalPath));
        item->setText(1, displayPath(previousPath));
        item->setText(2, status);
        item->setToolTip(0, QDir::toNativeSeparators(finalPath));
        item->setToolTip(1, QDir::toNativeSeparators(previousPath));
        item->setToolTip(2, status);
        if (color.isValid()) {
            for (int column = 0; column < resultTree->columnCount(); ++column)
                item->setForeground(column, color);
        }
    };

    QHash<QString, QString> failureReasons;
    for (const auto &failure : failures)
        failureReasons.insert(QDir::cleanPath(failure.path), failure.reason);

    QList<FileMove> sortedMoves = moves;
    std::sort(sortedMoves.begin(), sortedMoves.end(), [restored](const FileMove &a, const FileMove &b) {
        const QString &aFinal = restored ? a.source : a.destination;
        const QString &bFinal = restored ? b.source : b.destination;
        return aFinal.compare(bFinal, Qt::CaseInsensitive) < 0;
    });

    for (const auto &move : std::as_const(sortedMoves)) {
        const QString failureKey = QDir::cleanPath(restored ? move.destination : move.source);
        if (failureReasons.contains(failureKey))
            continue;

        const QString finalPath = restored ? move.source : move.destination;
        const QString previousPath = restored ? move.destination : move.source;
        addRow(finalPath, previousPath, restored ? tr("Restored") : (renaming() ? tr("Renamed") : tr("Moved")));
    }

    const bool dark = resultTree->palette().color(QPalette::Base).lightness() < 128;
    const QColor errorColor = dark ? QColor(0xFF, 0x7B, 0x72) : QColor(0xC0, 0x39, 0x2B);
    for (const auto &failure : failures) {
        const auto requested = std::find_if(lastRequestedMoves.cbegin(), lastRequestedMoves.cend(), [&failure, restored](const FileMove &move) {
            const QString relevantPath = restored ? move.destination : move.source;
            return QDir::cleanPath(relevantPath) == QDir::cleanPath(failure.path);
        });

        const QString finalPath = requested == lastRequestedMoves.cend()
                ? failure.path
                : (restored ? requested->source : requested->destination);
        const QString previousPath = requested == lastRequestedMoves.cend()
                ? failure.path
                : (restored ? requested->destination : requested->source);
        addRow(finalPath, previousPath, restored ? tr("Undo failed: %1").arg(failure.reason) : tr("Failed: %1").arg(failure.reason),
               errorColor);
    }

    resultTree->setVisible(!moves.isEmpty() || !failures.isEmpty());
    if (resultTree->isVisible()) {
        const int availableWidth = qMax(resultTree->viewport()->width(), width() - 48);
        resultTree->setColumnWidth(0, availableWidth * 2 / 5);
        resultTree->setColumnWidth(1, availableWidth * 2 / 5);
    }
}

void OrganizeFilesDialog::moveFinished()
{
    moveThread->quit();
    moveThread->wait();

    const auto completed = moveWorker->completedMoves();
    const auto failures = moveWorker->failures();
    const auto removedDirectories = moveWorker->removedDirectories();
    const QString journalPath = moveWorker->journalPath();
    const QString startError = moveWorker->startError();
    const QString recordError = moveWorker->recordError();
    const int notAttempted = moveWorker->notAttempted();
    const bool databaseUpdated = moveWorker->databaseUpdated();
    const QString databaseError = moveWorker->databaseError();

    QStringList lines;

    if (!startError.isEmpty()) {
        // Nothing was touched: the run refuses to start without a record.
        lines << tr("Nothing was moved.")
              << tr("The record this run could be undone from could not be written, so the run did not start: %1").arg(startError);
    } else {
        lines << (renaming() ? tr("%n file(s) renamed.", "", completed.size())
                             : tr("%n file(s) moved into %1.", "", completed.size()).arg(QDir::toNativeSeparators(currentBase())));

        if (!recordError.isEmpty()) {
            lines << tr("The record of this run stopped early, so the run stopped with it: %1").arg(recordError);
            if (notAttempted > 0)
                lines << tr("%n file(s) were not moved.", "", notAttempted);
        }
    }

    if (!completed.isEmpty()) {
        if (databaseUpdated)
            changedLibrary = true;
        else
            lines << tr("The library database could not be updated: %1").arg(databaseError)
                  << tr("Use Undo to move the files back, or update the library to make it match the files.");
    }

    if (!removedDirectories.isEmpty())
        lines << tr("%n empty folder(s) were removed.", "", removedDirectories.size());

    if (!failures.isEmpty())
        lines << tr("%n file(s) could not be moved.", "", failures.size());

    showFailures(failures);
    showCompletedMoves(completed, failures);

    resultLabel->setText(lines.join(QStringLiteral("\n")));

    lastJournalPath = journalPath;
    lastCompletedMoves = completed;
    undoButton->setEnabled(!journalPath.isEmpty() && !completed.isEmpty() && static_cast<bool>(undoer));

    // Deleted directly: a deferred delete posted to a stopped thread never runs.
    delete moveWorker;
    moveWorker = nullptr;
    moveThread->deleteLater();
    moveThread = nullptr;
    moveRunning = false;

    pages->setCurrentIndex(ResultPage);
}

void OrganizeFilesDialog::undo()
{
    const QString journalPath = lastJournalPath;
    if (journalPath.isEmpty() || !undoer || undoRunning || moveRunning)
        return;

    undoRunning = true;
    undoButton->setEnabled(false);

    progressBar->setRange(0, 0);
    progressBar->setValue(0);
    progressLabel->setText(tr("Moving the files back..."));
    pages->setCurrentIndex(WorkingPage);

    // Same treatment as the run it reverses: worker thread and progress page.
    const Undoer runner = undoer;
    undoWorker = new OrganizeFiles::UndoWorker(
            [runner, journalPath](QList<FileFailure> *failures, QString *error,
                                  const std::function<void(int, int, const QString &)> &fileProgress,
                                  const std::function<void()> &databasePhase) {
                return runner(journalPath, failures, error, fileProgress, databasePhase);
            });
    undoThread = new QThread(this);
    undoWorker->moveToThread(undoThread);

    connect(undoThread, &QThread::started, undoWorker, &OrganizeFiles::UndoWorker::process);
    connect(undoWorker, &OrganizeFiles::UndoWorker::progress, this, &OrganizeFilesDialog::undoProgress);
    connect(undoWorker, &OrganizeFiles::UndoWorker::updatingLibrary, this, &OrganizeFilesDialog::showUpdatingLibrary);
    connect(undoWorker, &OrganizeFiles::UndoWorker::finished, this, &OrganizeFilesDialog::undoFinished);

    undoThread->start();
}

void OrganizeFilesDialog::undoProgress(int done, int total, const QString &currentFile)
{
    progressBar->setRange(0, total);
    progressBar->setValue(done);
    progressLabel->setText(tr("Moving back %1 of %2\n%3").arg(done).arg(total).arg(QDir::toNativeSeparators(currentFile)));
}

void OrganizeFilesDialog::undoFinished()
{
    undoThread->quit();
    undoThread->wait();

    const bool success = undoWorker->succeeded();
    const auto failures = undoWorker->failures();
    const QString error = undoWorker->errorString();

    delete undoWorker;
    undoWorker = nullptr;
    undoThread->deleteLater();
    undoThread = nullptr;
    undoRunning = false;

    // Even a failed undo has moved files and touched the database.
    changedLibrary = true;

    if (success) {
        resultLabel->setText(tr("Everything was moved back."));
        showCompletedMoves(lastCompletedMoves, { }, true);
        showFailures({ });
        undoButton->setEnabled(false);
    } else {
        resultLabel->setText(tr("The undo did not finish: %1").arg(error));
        showFailures(failures);
        showCompletedMoves(lastCompletedMoves, failures, true);
        // The journal survives a failed undo so it can be retried, and this button
        // is the only way to reach it.
        undoButton->setEnabled(true);
    }

    pages->setCurrentIndex(ResultPage);
}

void OrganizeFilesDialog::reject()
{
    if (moveRunning || undoRunning)
        return;

    QDialog::reject();
}

void OrganizeFilesDialog::done(int result)
{
    saveSettings();
    QDialog::done(result);
}

void OrganizeFilesDialog::resizeEvent(QResizeEvent *event)
{
    QDialog::resizeEvent(event);
    updateBasePathLabel();
}

void OrganizeFilesDialog::closeEvent(QCloseEvent *event)
{
    if (moveRunning || undoRunning) {
        event->ignore();
        return;
    }

    QDialog::closeEvent(event);
}

void OrganizeFilesDialog::wrapSelectionInOptionalGroup()
{
    QString text = patternEdit->text();
    int start = patternEdit->selectionStart();

    if (start < 0) {
        start = patternEdit->cursorPosition();
        text.insert(start, QStringLiteral("<>"));
        patternEdit->setText(text);
        patternEdit->setCursorPosition(start + 1);
    } else {
        const int length = patternEdit->selectedText().size();
        text.insert(start + length, QLatin1Char('>'));
        text.insert(start, QLatin1Char('<'));
        patternEdit->setText(text);
        patternEdit->setCursorPosition(start + length + 2);
    }

    patternEdit->setFocus();
}

void OrganizeFilesDialog::showFormatHelp()
{
    auto help = new QDialog(this);
    help->setAttribute(Qt::WA_DeleteOnClose);
    help->setWindowTitle(tr("Format help"));

    auto layout = new QVBoxLayout(help);

    const auto section = [&](const QString &title, const QString &description, const QString &example) {
        auto group = new QGroupBox(title, help);
        group->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
        auto groupLayout = new QVBoxLayout(group);

        auto text = new QLabel(description, group);
        text->setWordWrap(true);
        groupLayout->addWidget(text);

        auto code = new QLabel(example, group);
        code->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
        code->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
        code->setMargin(6);
        code->setTextInteractionFlags(Qt::TextSelectableByMouse);
        groupLayout->addWidget(code);

        layout->addWidget(group);
    };

    const QChar lineBreak = QChar::LineFeed;
    const QChar quote = QLatin1Char('"');
    const auto quoted = [quote](const QString &text) { return quote + text + quote; };

    section(tr("Fields"),
            tr("Every field is written between braces and is replaced by the metadata of the comic. "
               "The Insert menu lists all of them."),
            OrganizeFiles::knownTokens().join(QStringLiteral("  ")) + lineBreak + tr("{series} gives %1").arg(quoted(QStringLiteral("The Amazing Spider-Man"))));

    section(tr("Optional parts"),
            tr("A part written between the signs < and > disappears completely when every field inside it is empty. "
               "Use it for punctuation that belongs to a field, such as brackets or a leading number sign. "
               "Text at the start or the end of a name is trimmed without it."),
            tr("{series} ({year})     with no year gives %1").arg(quoted(QStringLiteral("Series ()"))) + lineBreak + tr("{series}< ({year})>   with no year gives %1").arg(quoted(QStringLiteral("Series"))));

    section(tr("Numbers"),
            tr("Write a colon and some zeros to pad the issue number. "
               "This keeps the issues in order in a file browser."),
            QStringLiteral("{number}      ") + quoted(QStringLiteral("42")) + lineBreak + QStringLiteral("{number:000}  ") + quoted(QStringLiteral("042")));

    if (renaming()) {
        section(tr("Folders"),
                tr("A filename format cannot contain a slash. Every comic keeps its current folder. "
                   "Use Organize into folders to move comics."),
                QStringLiteral("{series} #{number:000}"));
    } else {
        section(tr("Folders"),
                tr("Each part separated by a slash becomes a folder. The last part becomes the file name. "
                   "The original extension is always kept."),
                QStringLiteral("{publisher}/{series}/{number:000}"));
    }

    auto buttons = new QDialogButtonBox(QDialogButtonBox::Close, help);
    connect(buttons, &QDialogButtonBox::rejected, help, &QDialog::reject);
    layout->addWidget(buttons);

    help->resize(540, help->sizeHint().height());
    help->open();
}

void OrganizeFilesDialog::copyFailures()
{
    QStringList lines;
    for (int i = 0; i < failureList->count(); ++i)
        lines << failureList->item(i)->text();

    QApplication::clipboard()->setText(lines.join(QStringLiteral("\n")));
}
