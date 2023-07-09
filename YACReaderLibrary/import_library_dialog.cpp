#include "import_library_dialog.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QGridLayout>
#include <QCloseEvent>

ImportLibraryDialog::ImportLibraryDialog(QWidget *parent)
    : QDialog(parent), progressCount(0)
{
    setupUI();
}

void ImportLibraryDialog::setupUI()
{
    nameLabel = new QLabel(tr("Library Name : "));
    nameEdit = new QLineEdit;
    nameLabel->setBuddy(nameEdit);
    connect(nameEdit, &QLineEdit::textChanged, this, &ImportLibraryDialog::nameEntered);

    textLabel = new QLabel(tr("Package location : "));
    path = new QLineEdit;
    textLabel->setBuddy(path);

    destLabel = new QLabel(tr("Destination folder : "));
    destPath = new QLineEdit;
    textLabel->setBuddy(destPath);

    accept = new QPushButton(tr("Unpack"));
    accept->setDisabled(true);
    connect(accept, &QAbstractButton::clicked, this, &ImportLibraryDialog::add);

    cancel = new QPushButton(tr("Cancel"));
    connect(cancel, &QAbstractButton::clicked, this, &ImportLibraryDialog::close);
    // connect(cancel,SIGNAL(clicked()),this,SIGNAL(rejected()));

    find = new QPushButton(QIcon(":/images/find_folder.png"), "");
    connect(find, &QAbstractButton::clicked, this, &ImportLibraryDialog::findPath);

    findDest = new QPushButton(QIcon(":/images/find_folder.png"), "");
    connect(findDest, &QAbstractButton::clicked, this, &ImportLibraryDialog::findDestination);

    auto content = new QGridLayout;

    content->addWidget(nameLabel, 0, 0);
    content->addWidget(nameEdit, 0, 1);

    content->addWidget(textLabel, 1, 0);
    content->addWidget(path, 1, 1);
    content->addWidget(find, 1, 2);
    content->setColumnStretch(2, 0); // TODO

    content->addWidget(destLabel, 2, 0);
    content->addWidget(destPath, 2, 1);
    content->addWidget(findDest, 2, 2);
    // destLayout->setStretchFactor(findDest,0); //TODO

    auto bottomLayout = new QHBoxLayout;
    bottomLayout->addStretch();
    bottomLayout->addWidget(accept);
    bottomLayout->addWidget(cancel);

    progressBar = new QProgressBar(this);
    progressBar->setMinimum(0);
    progressBar->setMaximum(0);
    progressBar->setTextVisible(false);
    progressBar->hide();

    auto mainLayout = new QVBoxLayout;
    mainLayout->addLayout(content);
    // mainLayout->addWidget(progress = new QLabel());
    mainLayout->addStretch();
    mainLayout->addWidget(progressBar);
    mainLayout->addLayout(bottomLayout);

    auto imgMainLayout = new QHBoxLayout;
    QLabel *imgLabel = new QLabel(this);
    QPixmap p(":/images/importLibrary.png");
    imgLabel->setPixmap(p);
    imgMainLayout->addWidget(imgLabel);
    imgMainLayout->addLayout(mainLayout);

    setLayout(imgMainLayout);

    setModal(true);
    setWindowTitle(tr("Extract a catalog"));
}
void ImportLibraryDialog::open(const YACReaderLibraries &libs)
{
    libraries = libs;
    QDialog::open();
}

void ImportLibraryDialog::add()
{
    if (!libraries.contains(nameEdit->text())) {
        accept->setEnabled(false);
        progressBar->show();
        emit unpackCLC(QDir::cleanPath(path->text()), QDir::cleanPath(destPath->text()), nameEdit->text());
    } else {
        emit libraryExists(nameEdit->text());
    }
}

void ImportLibraryDialog::findPath()
{
    QString s = QFileDialog::getOpenFileName(0, "Covers Package", ".", tr("Compresed library covers (*.clc)"));
    if (!s.isEmpty()) {
        path->setText(s);
        if (!destPath->text().isEmpty() && !nameEdit->text().isEmpty())
            accept->setEnabled(true);
    }
}

void ImportLibraryDialog::findDestination()
{
    QString s = QFileDialog::getExistingDirectory(0, "Folder", ".", QFileDialog::ShowDirsOnly);
    if (!s.isEmpty()) {
        destPath->setText(s);
        if (!path->text().isEmpty() && !nameEdit->text().isEmpty())
            accept->setEnabled(true);
    }
}

void ImportLibraryDialog::nameEntered()
{
    if (!nameEdit->text().isEmpty()) {
        if (!path->text().isEmpty() && !destPath->text().isEmpty())
            accept->setEnabled(true);
    } else
        accept->setEnabled(false);
}

void ImportLibraryDialog::close()
{
    path->clear();
    destPath->clear();
    nameEdit->clear();
    accept->setEnabled(false);
    progressBar->hide();
    QDialog::hide();
}

void ImportLibraryDialog::closeEvent(QCloseEvent *e)
{
    close();
    e->accept();
}
