#include "move_to_folder_dialog.h"

#include "reading_list_model.h"

#include <QDialogButtonBox>
#include <QLabel>
#include <QPushButton>
#include <QTreeWidget>
#include <QVBoxLayout>

#include <functional>

MoveToFolderDialog::MoveToFolderDialog(ReadingListModel *model, const QModelIndex &item, QWidget *parent)
    : QDialog(parent), model(model), item(item)
{
    setWindowTitle(tr("Move to folder"));

    tree = new QTreeWidget;
    tree->setHeaderHidden(true);

    // the places it cannot go to (its current place, a folder itself and its folders) are shown, but cannot be chosen
    const auto disabledText = palette().brush(QPalette::Disabled, QPalette::Text);
    const auto addPlace = [&](QTreeWidgetItem *parentItem, const QString &name, const QIcon &icon, const QModelIndex &folder) {
        auto *place = parentItem == nullptr ? new QTreeWidgetItem(tree, { name }) : new QTreeWidgetItem(parentItem, { name });
        place->setIcon(0, icon);
        place->setData(0, Qt::UserRole, QVariant::fromValue(QPersistentModelIndex(folder)));
        if (!model->canMoveToFolder(item, folder)) {
            place->setFlags(place->flags() & ~Qt::ItemIsSelectable);
            place->setForeground(0, disabledText);
        }
        return place;
    };

    const auto section = model->section(item);
    std::function<void(QTreeWidgetItem *, const QModelIndex &)> addFolders = [&](QTreeWidgetItem *parentItem, const QModelIndex &parentIndex) {
        for (int row = 0; row < model->rowCount(parentIndex); ++row) {
            const auto child = model->index(row, 0, parentIndex);
            const auto id = model->listFolder(child);
            if (id && section && id->kind == *section)
                addFolders(addPlace(parentItem, model->name(child), child.data(Qt::DecorationRole).value<QIcon>(), child), child);
        }
    };
    addFolders(addPlace(nullptr, tr("Top level"), QIcon(), QModelIndex()), QModelIndex());
    tree->expandAll();

    auto buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    moveButton = buttons->button(QDialogButtonBox::Ok);
    moveButton->setEnabled(false);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(tree, &QTreeWidget::itemSelectionChanged, this, [this] {
        moveButton->setEnabled(!tree->selectedItems().isEmpty() && this->model->canMoveToFolder(this->item, folder()));
    });

    auto layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel(tr("Move \"%1\" to:").arg(model->name(item))));
    layout->addWidget(tree);
    layout->addWidget(buttons);
    resize(360, 420);
}

QModelIndex MoveToFolderDialog::folder() const
{
    const auto selected = tree->selectedItems();
    if (selected.isEmpty())
        return { };
    return selected.constFirst()->data(0, Qt::UserRole).value<QPersistentModelIndex>();
}
