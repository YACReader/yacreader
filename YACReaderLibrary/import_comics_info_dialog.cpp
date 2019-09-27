#include "import_comics_info_dialog.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QProgressBar>

#include "data_base_management.h"

ImportComicsInfoDialog::ImportComicsInfoDialog(QWidget *parent)
    : QDialog(parent)
{
    setModal(true);
    setWindowTitle(tr("Import comics info"));

    textLabel = new QLabel(tr("Info database location : "));
    path = new QLineEdit;
    textLabel->setBuddy(path);

    accept = new QPushButton(tr("Import"));
    accept->setDisabled(true);
    connect(accept, SIGNAL(clicked()), this, SLOT(import()));

    cancel = new QPushButton(tr("Cancel"));
    connect(cancel, SIGNAL(clicked()), this, SLOT(close()));
    //connect(cancel,SIGNAL(clicked()),this,SIGNAL(rejected()));

    find = new QPushButton(QIcon(":/images/find_folder.png"), "");
    connect(find, SIGNAL(clicked()), this, SLOT(findPath()));

    auto libraryLayout = new QHBoxLayout;

    libraryLayout->addWidget(textLabel);
    libraryLayout->addWidget(path);
    libraryLayout->addWidget(find);
    libraryLayout->setStretchFactor(find, 0); //TODO

    progressBar = new QProgressBar(this);
    progressBar->setMinimum(0);
    progressBar->setMaximum(0);
    progressBar->setTextVisible(false);
    progressBar->hide();
    connect(accept, SIGNAL(clicked()), progressBar, SLOT(show()));

    auto bottomLayout = new QHBoxLayout;
    bottomLayout->addStretch();
    bottomLayout->addWidget(accept);
    bottomLayout->addWidget(cancel);

    auto mainLayout = new QVBoxLayout;
    mainLayout->addLayout(libraryLayout);
    mainLayout->addStretch();
    mainLayout->addWidget(progressBar);
    mainLayout->addLayout(bottomLayout);

    auto imgMainLayout = new QHBoxLayout;
    QLabel *imgLabel = new QLabel(this);
    QPixmap p(":/images/importComicsInfo.png");
    imgLabel->setPixmap(p);
    imgMainLayout->addWidget(imgLabel);
    imgMainLayout->addLayout(mainLayout);

    setLayout(imgMainLayout);

    setModal(true);
}

ImportComicsInfoDialog::~ImportComicsInfoDialog()
{
}

void ImportComicsInfoDialog::findPath()
{
    QString s = QFileDialog::getOpenFileName(0, "Comics Info", ".", tr("Comics info file (*.ydb)"));
    if (!s.isEmpty()) {
        path->setText(s);
        accept->setEnabled(true);
    }
}

void ImportComicsInfoDialog::import()
{
    progressBar->show();

    auto importer = new Importer();
    importer->source = path->text();
    importer->dest = dest;
    connect(importer, SIGNAL(finished()), this, SLOT(close()));
    connect(importer, SIGNAL(finished()), this, SLOT(hide()));
    importer->start();
}

void ImportComicsInfoDialog::close()
{
    path->clear();
    progressBar->hide();
    accept->setDisabled(true);
    QDialog::close();
    emit(finished(0));
}

void Importer::run()
{
    DataBaseManagement::importComicsInfo(source, dest);
}
