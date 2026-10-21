#include "new_list_folder_dialog.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QVBoxLayout>

NewListFolderDialog::NewListFolderDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("New folder"));

    nameEdit = new QLineEdit;
    readingListsButton = new QRadioButton(tr("Reading lists"));
    smartListsButton = new QRadioButton(tr("Smart lists"));
    readingListsButton->setChecked(true);

    auto sections = new QVBoxLayout;
    sections->setContentsMargins(0, 0, 0, 0);
    sections->addWidget(readingListsButton);
    sections->addWidget(smartListsButton);

    auto form = new QFormLayout;
    form->addRow(tr("Folder name:"), nameEdit);
    form->addRow(tr("For:"), sections);

    auto buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    createButton = buttons->button(QDialogButtonBox::Ok);
    createButton->setEnabled(false);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(nameEdit, &QLineEdit::textChanged, this, [this] { createButton->setEnabled(!name().isEmpty()); });

    auto layout = new QVBoxLayout(this);
    layout->addLayout(form);
    layout->addWidget(buttons);
}

QString NewListFolderDialog::name() const
{
    return nameEdit->text().trimmed();
}

ListFolderKind NewListFolderDialog::section() const
{
    return smartListsButton->isChecked() ? ListFolderKind::SmartList : ListFolderKind::ReadingList;
}
