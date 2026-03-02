#include "theme_editor_dialog.h"

#include <cmath>
#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QColorDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QHeaderView>
#include <QPixmap>
#include <QJsonArray>
#include <QJsonDocument>
#include <QApplication>
#include <QFileDialog>
#include <QFile>
#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QUuid>

// Role used to store the JSON path (QStringList) on each leaf item.
static const int PathRole = Qt::UserRole;
// Role used to distinguish color items from others.
static const int IsColorRole = Qt::UserRole + 1;
// Role used to distinguish boolean items.
static const int IsBoolRole = Qt::UserRole + 2;
// Role used to distinguish numeric items.
static const int IsNumberRole = Qt::UserRole + 3;

static bool isColorString(const QString &s)
{
    // Accepts #RGB, #RRGGBB, #AARRGGBB
    if (!s.startsWith('#'))
        return false;
    const int len = s.length();
    return len == 4 || len == 7 || len == 9;
}

ThemeEditorDialog::ThemeEditorDialog(const QJsonObject &params, QWidget *parent)
    : QDialog(parent), params(params)
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

    populate(nullptr, params, {});
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
    auto *saveBtn = new QPushButton(tr("Save to file..."), this);
    auto *loadBtn = new QPushButton(tr("Load from file..."), this);
    auto *closeBtn = new QPushButton(tr("Close"), this);
    connect(saveBtn, &QPushButton::clicked, this, &ThemeEditorDialog::saveToFile);
    connect(loadBtn, &QPushButton::clicked, this, &ThemeEditorDialog::loadFromFile);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::close);
    auto *buttons = new QHBoxLayout();
    buttons->addWidget(saveBtn);
    buttons->addWidget(loadBtn);
    buttons->addStretch();
    buttons->addWidget(closeBtn);

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(toolbar);
    layout->addWidget(metaBox);
    layout->addWidget(tree);
    layout->addLayout(buttons);

    setLayout(layout);
}

void ThemeEditorDialog::populate(QTreeWidgetItem *parent, const QJsonObject &obj, const QStringList &path)
{
    for (auto it = obj.constBegin(); it != obj.constEnd(); ++it) {
        const QString key = it.key();

        // "meta" is handled by the dedicated UI above the tree
        if (path.isEmpty() && key == "meta")
            continue;
        const QJsonValue val = it.value();
        const QStringList childPath = path + QStringList(key);

        if (val.isObject()) {
            // Group row
            QTreeWidgetItem *group = parent ? new QTreeWidgetItem(parent)
                                            : new QTreeWidgetItem(tree);
            QFont bold = group->font(0);
            bold.setBold(true);
            group->setFont(0, bold);
            group->setText(0, key);
            group->setFlags(group->flags() & ~Qt::ItemIsSelectable);
            populate(group, val.toObject(), childPath);
        } else {
            // Leaf row
            QTreeWidgetItem *item = parent ? new QTreeWidgetItem(parent)
                                           : new QTreeWidgetItem(tree);
            item->setText(0, key);
            item->setData(0, PathRole, childPath);

            const QString strVal = val.toString();
            if (val.isString() && isColorString(strVal)) {
                const QColor color(strVal);
                item->setIcon(1, colorIcon(color));
                item->setText(1, strVal);
                item->setData(0, IsColorRole, true);
                item->setToolTip(1, tr("Double-click to edit color"));
            } else if (val.isBool()) {
                item->setText(1, val.toBool() ? tr("true") : tr("false"));
                item->setData(0, IsColorRole, false);
                item->setData(0, IsBoolRole, true);
                item->setToolTip(1, tr("Double-click to toggle"));
            } else if (val.isDouble()) {
                item->setText(1, QString::number(val.toDouble()));
                item->setData(0, IsNumberRole, true);
                item->setToolTip(1, tr("Double-click to edit value"));
            } else {
                item->setText(1, strVal);
                item->setData(0, IsColorRole, false);
            }
        }
    }
}

void ThemeEditorDialog::editColorItem(QTreeWidgetItem *item)
{
    const QColor current(item->text(1));
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
    item->setText(1, hexStr);
    item->setIcon(1, colorIcon(color));

    const QStringList path = item->data(0, PathRole).toStringList();
    setJsonPath(params, path, hexStr);
}

void ThemeEditorDialog::toggleBoolItem(QTreeWidgetItem *item)
{
    const bool newValue = item->text(1) != tr("true");
    item->setText(1, newValue ? tr("true") : tr("false"));
    const QStringList path = item->data(0, PathRole).toStringList();
    setJsonPath(params, path, newValue);
    emit themeJsonChanged(params);
}

void ThemeEditorDialog::editNumberItem(QTreeWidgetItem *item)
{
    const double current = item->text(1).toDouble();
    // Use integer dialog when the stored value has no fractional part
    const bool isInt = (current == std::floor(current));
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
    const QStringList path = item->data(0, PathRole).toStringList();
    setJsonPath(params, path, newValue);
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
    file.write(QJsonDocument(params).toJson(QJsonDocument::Indented));
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

    params = doc.object();
    syncMetaFromParams();
    tree->clear();
    populate(nullptr, params, {});
    tree->expandAll();
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
