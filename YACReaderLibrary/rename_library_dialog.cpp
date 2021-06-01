#include "rename_library_dialog.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileDialog>

RenameLibraryDialog::RenameLibraryDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUI();
}

void RenameLibraryDialog::setupUI()
{
    newNameLabel = new QLabel(tr("New Library Name : "));
    newNameEdit = new QLineEdit;
    newNameLabel->setBuddy(newNameEdit);
    connect(newNameEdit, &QLineEdit::textChanged, this, &RenameLibraryDialog::nameSetted);

    accept = new QPushButton(tr("Rename"));
    accept->setDisabled(true);
    connect(accept, &QAbstractButton::clicked, this, &RenameLibraryDialog::rename);

    cancel = new QPushButton(tr("Cancel"));
    connect(cancel, SIGNAL(clicked()), this, SLOT(close()));

    auto nameLayout = new QHBoxLayout;

    nameLayout->addWidget(newNameLabel);
    nameLayout->addWidget(newNameEdit);

    auto bottomLayout = new QHBoxLayout;
    bottomLayout->addStretch();
    bottomLayout->addWidget(accept);
    bottomLayout->addWidget(cancel);

    auto mainLayout = new QVBoxLayout;
    mainLayout->addLayout(nameLayout);
    mainLayout->addStretch();
    mainLayout->addLayout(bottomLayout);

    auto imgMainLayout = new QHBoxLayout;
    QLabel *imgLabel = new QLabel(this);
    QPixmap p(":/images/edit.png");
    imgLabel->setPixmap(p);
    imgMainLayout->addWidget(imgLabel);
    imgMainLayout->addLayout(mainLayout);

    setLayout(imgMainLayout);

    setModal(true);
    setWindowTitle(tr("Rename current library"));
}

void RenameLibraryDialog::rename()
{
    //accept->setEnabled(false);
    emit(renameLibrary(newNameEdit->text()));
}

void RenameLibraryDialog::nameSetted(const QString &text)
{
    if (!text.isEmpty())
        accept->setEnabled(true);
    else
        accept->setEnabled(false);
}

void RenameLibraryDialog::close()
{
    newNameEdit->clear();
    //accept->setEnabled(false);
    QDialog::close();
}