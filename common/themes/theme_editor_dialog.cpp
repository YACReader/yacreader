#include "theme_editor_dialog.h"

#include "theme_json_utils.h"
#include "version.h"

#include <QApplication>
#include <QColorDialog>
#include <QComboBox>
#include <QFile>
#include <QFileDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPixmap>
#include <QPushButton>
#include <QStyle>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QUuid>
#include <QVBoxLayout>

#include <cmath>

// Role used to store the JSON path (QStringList) on each leaf item.
static const int PathRole = Qt::UserRole;
// Role used to distinguish color items from others.
static const int IsColorRole = Qt::UserRole + 1;
// Role used to distinguish boolean items.
static const int IsBoolRole = Qt::UserRole + 2;
// Role used to distinguish numeric items.
static const int IsNumberRole = Qt::UserRole + 3;
static const int IsMissingRole = Qt::UserRole + 4;

// Converts a camelCase JSON key to a human-readable display string.
// "metadataScraperDialog" → "Metadata scraper dialog"
// "navigationTreeQSS"     → "Navigation tree qss"
// Consecutive uppercase letters (acronyms) are kept together as one word.
static QString displayKey(const QString &key)
{
    QString result;
    for (int i = 0; i < key.size(); ++i) {
        const bool isUpper = key[i].isUpper();
        const bool prevIsLower = (i > 0) && key[i - 1].isLower();
        const bool nextIsLower = (i + 1 < key.size()) && key[i + 1].isLower();
        if (i > 0 && isUpper && (prevIsLower || nextIsLower))
            result += ' ';
        result += result.isEmpty() ? key[i].toUpper() : key[i].toLower();
    }
    return result;
}

ThemeEditorDialog::ThemeEditorDialog(const QJsonObject &json, const QJsonObject &reference, QWidget *parent)
    : QDialog(parent), reference(reference), params(referenceThemeKeys(reference, normalizeThemeJson(json)))
{
    setWindowTitle(tr("Theme Editor"));
    resize(520, 700);

    // --- top toolbar ---
    auto *expandBtn = new QPushButton(tr("+"), this);
    auto *collapseBtn = new QPushButton(tr("-"), this);
    auto *identifyBtn = new QPushButton(tr("i"), this);
    expandBtn->setFixedWidth(28);
    collapseBtn->setFixedWidth(28);
    identifyBtn->setFixedWidth(28);
    expandBtn->setToolTip(tr("Expand all"));
    collapseBtn->setToolTip(tr("Collapse all"));
    identifyBtn->setToolTip(tr("Hold to flash the selected value in the UI (magenta / toggled / 0↔10). Releases restore the original."));
    // NoFocus so clicking the button doesn't steal the tree's current item
    identifyBtn->setFocusPolicy(Qt::NoFocus);

    searchEdit = new QLineEdit(this);
    searchEdit->setPlaceholderText(tr("Search…"));
    searchEdit->setClearButtonEnabled(true);

    auto *toolbar = new QHBoxLayout();
    toolbar->addWidget(expandBtn);
    toolbar->addWidget(collapseBtn);
    toolbar->addWidget(identifyBtn);
    toolbar->addStretch();
    toolbar->addWidget(searchEdit);

    connect(identifyBtn, &QPushButton::pressed, this, &ThemeEditorDialog::identifyPressed);
    connect(identifyBtn, &QPushButton::released, this, &ThemeEditorDialog::identifyReleased);

    // --- meta section ---
    idLabel = new QLabel(this);
    idLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    nameEdit = new QLineEdit(this);
    variantCombo = new QComboBox(this);
    variantCombo->addItem(tr("Light"), "light");
    variantCombo->addItem(tr("Dark"), "dark");

    auto *metaForm = new QFormLayout();
    metaForm->addRow(tr("ID:"), idLabel);
    metaForm->addRow(tr("Display name:"), nameEdit);
    metaForm->addRow(tr("Variant:"), variantCombo);

    auto *metaBox = new QGroupBox(tr("Theme info"), this);
    metaBox->setLayout(metaForm);

    syncMetaFromParams();

    connect(nameEdit, &QLineEdit::textEdited, this, [this](const QString &text) {
        auto meta = this->params["meta"].toObject();
        meta["displayName"] = text;
        this->params["meta"] = meta;
    });
    connect(nameEdit, &QLineEdit::editingFinished, this, [this]() {
        emit themeJsonChanged(this->params);
    });
    connect(variantCombo, &QComboBox::currentIndexChanged, this, [this](int index) {
        auto meta = this->params["meta"].toObject();
        meta["variant"] = variantCombo->itemData(index).toString();
        this->params["meta"] = meta;
        emit themeJsonChanged(this->params);
    });

    // --- tree ---
    tree = new QTreeWidget(this);
    tree->setColumnCount(2);
    tree->setHeaderLabels({ tr("Parameter"), tr("Value") });
    tree->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    tree->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    tree->setRootIsDecorated(true);
    tree->setUniformRowHeights(true);
    tree->setAlternatingRowColors(true);

    populate(nullptr, reference, { });
    tree->expandAll();

    connect(expandBtn, &QPushButton::clicked, tree, &QTreeWidget::expandAll);
    connect(collapseBtn, &QPushButton::clicked, tree, &QTreeWidget::collapseAll);
    connect(searchEdit, &QLineEdit::textChanged, this, &ThemeEditorDialog::filterTree);

    connect(tree, &QTreeWidget::itemDoubleClicked, this, [this](QTreeWidgetItem *item, int) {
        if (item->data(0, IsColorRole).toBool())
            editColorItem(item);
        else if (item->data(0, IsBoolRole).toBool())
            toggleBoolItem(item);
        else if (item->data(0, IsNumberRole).toBool())
            editNumberItem(item);
    });

    // --- bottom buttons ---
    missingLabel = new QLabel(this);
    saveLibBtn = new QPushButton(tr("Save and apply"), this);
    exportBtn = new QPushButton(tr("Export to file..."), this);
    auto *loadBtn = new QPushButton(tr("Load from file..."), this);
    auto *closeBtn = new QPushButton(tr("Close"), this);
    connect(saveLibBtn, &QPushButton::clicked, this, [this]() {
        emit saveToLibraryRequested(paramsToSave());
    });
    connect(exportBtn, &QPushButton::clicked, this, &ThemeEditorDialog::saveToFile);
    connect(loadBtn, &QPushButton::clicked, this, &ThemeEditorDialog::loadFromFile);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::close);
    auto *buttons = new QHBoxLayout();
    buttons->addWidget(saveLibBtn);
    buttons->addWidget(exportBtn);
    buttons->addWidget(loadBtn);
    buttons->addStretch();
    buttons->addWidget(missingLabel);
    buttons->addWidget(closeBtn);

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(toolbar);
    layout->addWidget(metaBox);
    layout->addWidget(tree);
    layout->addLayout(buttons);

    setLayout(layout);

    updateSaveState();
}

void ThemeEditorDialog::populate(QTreeWidgetItem *parent, const QJsonObject &referenceObj, const QStringList &path)
{
    for (auto it = referenceObj.constBegin(); it != referenceObj.constEnd(); ++it) {
        const QString key = it.key();

        // "meta" is handled by the dedicated UI above the tree
        if (path.isEmpty() && key == "meta")
            continue;
        const QJsonValue referenceVal = it.value();
        const QStringList childPath = path + QStringList(key);

        if (referenceVal.isObject()) {
            // Group row
            QTreeWidgetItem *group = parent ? new QTreeWidgetItem(parent)
                                            : new QTreeWidgetItem(tree);
            QFont bold = group->font(0);
            bold.setBold(true);
            group->setFont(0, bold);
            group->setText(0, displayKey(key));
            group->setFlags(group->flags() & ~Qt::ItemIsSelectable);
            populate(group, referenceVal.toObject(), childPath);
        } else {
            // Leaf row
            QTreeWidgetItem *item = parent ? new QTreeWidgetItem(parent)
                                           : new QTreeWidgetItem(tree);
            item->setText(0, displayKey(key));
            item->setData(0, PathRole, childPath);

            const QJsonValue val = jsonPathValue(params, childPath);
            if (referenceVal.isString() && isThemeHexColorString(referenceVal.toString())) {
                item->setData(0, IsColorRole, true);
                item->setToolTip(1, tr("Double-click to edit color"));
                if (!val.isUndefined()) {
                    item->setIcon(1, colorIcon(QColor(val.toString())));
                    item->setText(1, val.toString());
                }
            } else if (referenceVal.isBool()) {
                item->setData(0, IsBoolRole, true);
                item->setToolTip(1, tr("Double-click to toggle"));
                if (!val.isUndefined())
                    item->setText(1, val.toBool() ? tr("true") : tr("false"));
            } else if (referenceVal.isDouble()) {
                item->setData(0, IsNumberRole, true);
                item->setToolTip(1, tr("Double-click to edit value"));
                if (!val.isUndefined())
                    item->setText(1, QString::number(val.toDouble()));
            } else if (!val.isUndefined()) {
                item->setText(1, val.toString());
            }
            if (val.isUndefined())
                setMissing(item);
        }
    }
}

void ThemeEditorDialog::setMissing(QTreeWidgetItem *item)
{
    item->setData(0, IsMissingRole, true);
    item->setText(1, tr("Missing"));
    item->setIcon(1, style()->standardIcon(QStyle::SP_MessageBoxWarning));
}

void ThemeEditorDialog::updateSaveState()
{
    const int missing = missingThemeKeys(reference, params).size();
    missingLabel->setText(tr("%n value(s) missing", "", missing));
    missingLabel->setVisible(missing > 0);
    saveLibBtn->setEnabled(missing == 0);
    exportBtn->setEnabled(missing == 0);
}

QJsonObject ThemeEditorDialog::paramsToSave()
{
    auto meta = params["meta"].toObject();
    meta["version"] = QStringLiteral(VERSION);
    params["meta"] = meta;
    return params;
}

void ThemeEditorDialog::editColorItem(QTreeWidgetItem *item)
{
    const bool wasMissing = item->data(0, IsMissingRole).toBool();
    const QColor current = wasMissing ? QColor() : QColor(item->text(1));
    QColorDialog dialog(current, this);
    dialog.setOption(QColorDialog::ShowAlphaChannel, true);
    dialog.setWindowTitle(tr("Edit: %1").arg(item->text(0)));

    // Live update as user drags the picker
    connect(&dialog, &QColorDialog::currentColorChanged, this, [this, item](const QColor &color) {
        applyColorToItem(item, color);
        emit themeJsonChanged(params);
    });

    if (dialog.exec() == QDialog::Accepted) {
        applyColorToItem(item, dialog.selectedColor());
    } else if (wasMissing) {
        removeJsonPath(params, item->data(0, PathRole).toStringList());
        setMissing(item);
        updateSaveState();
    } else {
        // Revert to original if cancelled
        applyColorToItem(item, current);
    }
    emit themeJsonChanged(params);
}

void ThemeEditorDialog::applyColorToItem(QTreeWidgetItem *item, const QColor &color)
{
    const QString hexStr = color.alpha() < 255
            ? color.name(QColor::HexArgb)
            : color.name(QColor::HexRgb);
    const QString normalizedHexStr = hexStr.toUpper();
    item->setText(1, normalizedHexStr);
    item->setIcon(1, colorIcon(color));

    item->setData(0, IsMissingRole, false);

    const QStringList path = item->data(0, PathRole).toStringList();
    setJsonPath(params, path, normalizedHexStr);
    updateSaveState();
}

void ThemeEditorDialog::toggleBoolItem(QTreeWidgetItem *item)
{
    const bool newValue = item->text(1) != tr("true");
    item->setText(1, newValue ? tr("true") : tr("false"));
    item->setIcon(1, QIcon());
    item->setData(0, IsMissingRole, false);
    const QStringList path = item->data(0, PathRole).toStringList();
    setJsonPath(params, path, newValue);
    updateSaveState();
    emit themeJsonChanged(params);
}

void ThemeEditorDialog::editNumberItem(QTreeWidgetItem *item)
{
    const QStringList path = item->data(0, PathRole).toStringList();
    const bool wasMissing = item->data(0, IsMissingRole).toBool();
    const double current = wasMissing ? 0.0 : item->text(1).toDouble();
    // Use integer dialog when the stored value has no fractional part
    const double typeSample = wasMissing ? jsonPathValue(reference, path).toDouble() : current;
    const bool isInt = (typeSample == std::floor(typeSample));
    bool ok = false;
    double newValue;
    if (isInt) {
        const int result = QInputDialog::getInt(
                this, tr("Edit: %1").arg(item->text(0)), item->text(0),
                static_cast<int>(current), INT_MIN, INT_MAX, 1, &ok);
        newValue = result;
    } else {
        newValue = QInputDialog::getDouble(
                this, tr("Edit: %1").arg(item->text(0)), item->text(0),
                current, -1e9, 1e9, 4, &ok);
    }
    if (!ok)
        return;
    item->setText(1, isInt ? QString::number(static_cast<int>(newValue)) : QString::number(newValue));
    item->setIcon(1, QIcon());
    item->setData(0, IsMissingRole, false);
    setJsonPath(params, path, newValue);
    updateSaveState();
    emit themeJsonChanged(params);
}

// Returns true if the item or any of its descendants should be visible.
static bool applyFilter(QTreeWidgetItem *item, const QString &query)
{
    if (query.isEmpty()) {
        item->setHidden(false);
        for (int i = 0; i < item->childCount(); ++i)
            applyFilter(item->child(i), query);
        return true;
    }

    const bool selfMatch = item->text(0).contains(query, Qt::CaseInsensitive);

    if (item->childCount() == 0) {
        // Leaf: match on key name or value text
        const bool match = selfMatch || item->text(1).contains(query, Qt::CaseInsensitive);
        item->setHidden(!match);
        return match;
    }

    // Group: if the group name itself matches, show all children
    bool anyChildVisible = false;
    for (int i = 0; i < item->childCount(); ++i) {
        if (selfMatch) {
            item->child(i)->setHidden(false);
            anyChildVisible = true;
        } else {
            if (applyFilter(item->child(i), query))
                anyChildVisible = true;
        }
    }
    item->setHidden(!anyChildVisible);
    return anyChildVisible;
}

void ThemeEditorDialog::filterTree(const QString &query)
{
    for (int i = 0; i < tree->topLevelItemCount(); ++i)
        applyFilter(tree->topLevelItem(i), query);

    // Keep visible results expanded so they're reachable
    if (!query.isEmpty())
        tree->expandAll();
}

QIcon ThemeEditorDialog::colorIcon(const QColor &color)
{
    const int size = qApp->style()->pixelMetric(QStyle::PM_SmallIconSize);
    QPixmap pix(size, size);
    pix.fill(color);
    return QIcon(pix);
}

QJsonValue ThemeEditorDialog::jsonPathValue(const QJsonObject &root, const QStringList &path)
{
    QJsonValue value = root;
    for (const auto &key : path)
        value = value.toObject().value(key);
    return value;
}

void ThemeEditorDialog::setJsonPath(QJsonObject &root, const QStringList &path, const QJsonValue &value)
{
    if (path.isEmpty())
        return;
    if (path.size() == 1) {
        root[path[0]] = value;
        return;
    }
    QJsonObject sub = root[path[0]].toObject();
    setJsonPath(sub, path.mid(1), value);
    root[path[0]] = sub;
}

void ThemeEditorDialog::removeJsonPath(QJsonObject &root, const QStringList &path)
{
    if (path.isEmpty())
        return;
    if (path.size() == 1) {
        root.remove(path[0]);
        return;
    }
    QJsonObject sub = root[path[0]].toObject();
    removeJsonPath(sub, path.mid(1));
    root[path[0]] = sub;
}

void ThemeEditorDialog::syncMetaToParams()
{
    auto meta = params["meta"].toObject();
    meta["displayName"] = nameEdit->text();
    meta["variant"] = variantCombo->currentData().toString();
    params["meta"] = meta;
}

void ThemeEditorDialog::syncMetaFromParams()
{
    const auto meta = params["meta"].toObject();
    idLabel->setText(meta["id"].toString());
    nameEdit->setText(meta["displayName"].toString());
    const QString variant = meta["variant"].toString("dark");
    variantCombo->setCurrentIndex(variant == "light" ? 0 : 1);
}

void ThemeEditorDialog::updateSavedId(const QString &id)
{
    auto meta = params["meta"].toObject();
    meta["id"] = id;
    params["meta"] = meta;
    idLabel->setText(id);
}

void ThemeEditorDialog::saveToFile()
{
    // Assign a user-scoped UUID if the current id is builtin or empty
    auto meta = params["meta"].toObject();
    const QString currentId = meta["id"].toString();
    if (currentId.isEmpty() || currentId.startsWith("builtin/")) {
        const QString newId = "user/" + QUuid::createUuid().toString(QUuid::WithoutBraces);
        meta["id"] = newId;
        params["meta"] = meta;
        idLabel->setText(newId);
    }

    const QString path = QFileDialog::getSaveFileName(
            this, tr("Save theme"), QString(), tr("JSON files (*.json);;All files (*)"));
    if (path.isEmpty())
        return;

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Save failed"), tr("Could not open file for writing:\n%1").arg(path));
        return;
    }
    file.write(serializeNormalizedThemeJson(paramsToSave()));
}

void ThemeEditorDialog::loadFromFile()
{
    const QString path = QFileDialog::getOpenFileName(
            this, tr("Load theme"), QString(), tr("JSON files (*.json);;All files (*)"));
    if (path.isEmpty())
        return;

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Load failed"), tr("Could not open file:\n%1").arg(path));
        return;
    }

    QJsonParseError err;
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &err);
    if (doc.isNull()) {
        QMessageBox::warning(this, tr("Load failed"), tr("Invalid JSON:\n%1").arg(err.errorString()));
        return;
    }
    if (!doc.isObject()) {
        QMessageBox::warning(this, tr("Load failed"), tr("Expected a JSON object."));
        return;
    }

    params = referenceThemeKeys(reference, normalizeThemeJson(doc.object()));
    syncMetaFromParams();
    tree->clear();
    populate(nullptr, reference, { });
    tree->expandAll();
    filterTree(searchEdit->text());
    updateSaveState();
    emit themeJsonChanged(params);
}

void ThemeEditorDialog::identifyPressed()
{
    QTreeWidgetItem *item = tree->currentItem();
    if (!item)
        return;

    const QStringList path = item->data(0, PathRole).toStringList();
    if (path.isEmpty())
        return; // group row, not a leaf
    if (item->data(0, IsMissingRole).toBool())
        return;

    if (item->data(0, IsColorRole).toBool()) {
        identifySnapshot = QJsonValue(item->text(1));
        identifyItem = item;
        identifyPath = path;
        applyColorToItem(item, QColor(0xFA00FA));
    } else if (item->data(0, IsBoolRole).toBool()) {
        const bool current = (item->text(1) == tr("true"));
        identifySnapshot = QJsonValue(current);
        identifyItem = item;
        identifyPath = path;
        const bool flipped = !current;
        item->setText(1, flipped ? tr("true") : tr("false"));
        setJsonPath(params, path, flipped);
    } else if (item->data(0, IsNumberRole).toBool()) {
        const double current = item->text(1).toDouble();
        identifySnapshot = QJsonValue(current);
        identifyItem = item;
        identifyPath = path;
        const bool isInt = (current == std::floor(current));
        const double highlight = (current > 0.0) ? 0.0 : 10.0;
        item->setText(1, isInt ? QString::number(static_cast<int>(highlight)) : QString::number(highlight));
        setJsonPath(params, path, highlight);
    } else {
        return; // non-editable leaf (plain string), nothing to flash
    }
    emit themeJsonChanged(params);
}

void ThemeEditorDialog::identifyReleased()
{
    if (!identifyItem)
        return;

    QTreeWidgetItem *item = identifyItem;
    const QStringList path = identifyPath;
    identifyItem = nullptr;
    identifyPath.clear();

    if (item->data(0, IsColorRole).toBool()) {
        applyColorToItem(item, QColor(identifySnapshot.toString()));
    } else if (item->data(0, IsBoolRole).toBool()) {
        const bool restored = identifySnapshot.toBool();
        item->setText(1, restored ? tr("true") : tr("false"));
        setJsonPath(params, path, restored);
    } else if (item->data(0, IsNumberRole).toBool()) {
        const double restored = identifySnapshot.toDouble();
        const bool isInt = (restored == std::floor(restored));
        item->setText(1, isInt ? QString::number(static_cast<int>(restored)) : QString::number(restored));
        setJsonPath(params, path, restored);
    }
    identifySnapshot = QJsonValue();
    emit themeJsonChanged(params);
}
