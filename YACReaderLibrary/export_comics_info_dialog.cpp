#include "export_comics_info_dialog.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QDir>
#include <QMessageBox>

#include "data_base_management.h"

ExportComicsInfoDialog::ExportComicsInfoDialog(QWidget *parent)
    : QDialog(parent)
{
    textLabel = new QLabel(tr("Output file : "));
    path = new QLineEdit;
    textLabel->setBuddy(path);

    accept = new QPushButton(tr("Create"));
    accept->setDisabled(true);
    connect(accept, &QAbstractButton::clicked, this, &ExportComicsInfoDialog::exportComicsInfo);

    cancel = new QPushButton(tr("Cancel"));
    connect(cancel, &QAbstractButton::clicked, this, &ExportComicsInfoDialog::close);
    connect(cancel, &QAbstractButton::clicked, this, &QDialog::rejected);

    find = new QPushButton(QIcon(":/images/find_folder.png"), "");
    connect(find, &QAbstractButton::clicked, this, &ExportComicsInfoDialog::findPath);

    auto libraryLayout = new QHBoxLayout;

    libraryLayout->addWidget(textLabel);
    libraryLayout->addWidget(path);
    libraryLayout->addWidget(find);
    libraryLayout->setStretchFactor(find, 0); //TODO

    auto bottomLayout = new QHBoxLayout;
    bottomLayout->addStretch();
    bottomLayout->addWidget(accept);
    bottomLayout->addWidget(cancel);

    auto mainLayout = new QVBoxLayout;
    mainLayout->addLayout(libraryLayout);
    mainLayout->addWidget(progress = new QLabel());
    mainLayout->addStretch();
    mainLayout->addLayout(bottomLayout);

    auto imgMainLayout = new QHBoxLayout;
    QLabel *imgLabel = new QLabel(this);
    QPixmap p(":/images/exportComicsInfo.png");
    imgLabel->setPixmap(p);
    imgMainLayout->addWidget(imgLabel);
    imgMainLayout->addLayout(mainLayout);

    setLayout(imgMainLayout);

    setModal(true);
    setWindowTitle(tr("Export comics info"));
}

ExportComicsInfoDialog::~ExportComicsInfoDialog()
{
}

void ExportComicsInfoDialog::findPath()
{
    QString s = QFileDialog::getSaveFileName(this, tr("Destination database name"), ".", "*.ydb");
    if (!s.isEmpty()) {
        path->setText(s.endsWith(".ydb") ? s : s + ".ydb");
        accept->setEnabled(true);
    }
}

void ExportComicsInfoDialog::exportComicsInfo()
{
    QFileInfo f(path->text());
    QFileInfo fPath(f.absoluteDir().path());
    if (fPath.exists() && fPath.isDir() && fPath.isWritable()) {
        DataBaseManagement::exportComicsInfo(source, path->text());
        close();
    } else
        QMessageBox::critical(NULL, tr("Problem found while writing"), tr("The selected path for the output file does not exist or is not a valid path. Be sure that you have write access to this folder"));
}

void ExportComicsInfoDialog::close()
{
    path->clear();
    QDialog::close();
}
