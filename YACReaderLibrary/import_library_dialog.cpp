#include "import_library_dialog.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileDialog>



ImportLibraryDialog::ImportLibraryDialog(QWidget * parent)
:QDialog(parent),progressCount(0)
{
	setupUI();
}

void ImportLibraryDialog::setupUI()
{
	nameLabel = new QLabel(tr("Library Name : "));
	nameEdit = new QLineEdit;
	nameLabel->setBuddy(nameEdit);
	connect(nameEdit,SIGNAL(textChanged(QString)),this,SLOT(nameEntered()));

	textLabel = new QLabel(tr("Package location : "));
	path = new QLineEdit;
	textLabel->setBuddy(path);

	destLabel = new QLabel(tr("Destination folder : "));
	destPath = new QLineEdit;
	textLabel->setBuddy(destPath);

	accept = new QPushButton(tr("Unpack"));
	accept->setDisabled(true);
	connect(accept,SIGNAL(clicked()),this,SLOT(add()));

	cancel = new QPushButton(tr("Cancel"));
	connect(cancel,SIGNAL(clicked()),this,SLOT(close()));
	//connect(cancel,SIGNAL(clicked()),this,SIGNAL(rejected()));

	find = new QPushButton(QIcon(":/images/coversPackage.png"),"");
	connect(find,SIGNAL(clicked()),this,SLOT(findPath()));

	findDest = new QPushButton(QIcon(":/images/open.png"),"");
	connect(findDest,SIGNAL(clicked()),this,SLOT(findDestination()));

	QHBoxLayout *nameLayout = new QHBoxLayout;

	nameLayout->addWidget(nameLabel);
	nameLayout->addWidget(nameEdit);

	QHBoxLayout *libraryLayout = new QHBoxLayout;

	libraryLayout->addWidget(textLabel);
	libraryLayout->addWidget(path);
	libraryLayout->addWidget(find);
	libraryLayout->setStretchFactor(find,0); //TODO

	QHBoxLayout *destLayout = new QHBoxLayout;

	destLayout->addWidget(destLabel);
	destLayout->addWidget(destPath);
	destLayout->addWidget(findDest);
	destLayout->setStretchFactor(findDest,0); //TODO


	QHBoxLayout *bottomLayout = new QHBoxLayout;
	bottomLayout->addStretch();
	bottomLayout->addWidget(accept);
	bottomLayout->addWidget(cancel);

	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addLayout(nameLayout);
	mainLayout->addLayout(libraryLayout);
	mainLayout->addLayout(destLayout);
	mainLayout->addWidget(progress = new QLabel());
	mainLayout->addStretch();
	mainLayout->addLayout(bottomLayout);

	QHBoxLayout * imgMainLayout = new QHBoxLayout;
	QLabel * imgLabel = new QLabel(this);
	QPixmap p(":/images/importLibrary.png");
	imgLabel->setPixmap(p);
	imgMainLayout->addWidget(imgLabel);
	imgMainLayout->addLayout(mainLayout);
	
	setLayout(imgMainLayout);

	setModal(true);
	setWindowTitle(tr("Extract a catalog"));

	t.setInterval(500);
	t.stop();
	connect(&t,SIGNAL(timeout()),this,SLOT(updateProgress()));
}

void ImportLibraryDialog::add()
{
	accept->setEnabled(false);
	t.start();
	emit(unpackCLC(QDir::cleanPath(path->text()),QDir::cleanPath(destPath->text()),nameEdit->text()));
}

void ImportLibraryDialog::findPath()
{
	QString s = QFileDialog::getOpenFileName(0,"Covers Package",".",tr("Compresed library covers (*.clc)"));
	if(!s.isEmpty())
	{
		path->setText(s);
		if(!destPath->text().isEmpty() && !nameEdit->text().isEmpty())
		    accept->setEnabled(true);
	}
}


void ImportLibraryDialog::findDestination()
{
	QString s = QFileDialog::getExistingDirectory(0,"Folder",".",QFileDialog::ShowDirsOnly);
	if(!s.isEmpty())
	{
		destPath->setText(s);
		if(!path->text().isEmpty() && !nameEdit->text().isEmpty())
		    accept->setEnabled(true);
	}
}

void ImportLibraryDialog::nameEntered()
{
    if(!nameEdit->text().isEmpty())
    {
	if(!path->text().isEmpty() && !destPath->text().isEmpty())
	    accept->setEnabled(true);
    }
    else
	accept->setEnabled(false);
}

void ImportLibraryDialog::close()
{
	path->clear();
	destPath->clear();
	nameEdit->clear();
	accept->setEnabled(false);
	if(t.isActive())
	{
	    t.stop();
	    emit rejected();
	}
	progress->setText("");
	QDialog::hide();
}

void ImportLibraryDialog::updateProgress()
{
    if(progressCount == 0)
	    progress->setText(tr("Importing package ."));
    else
	    progress->setText(progress->text()+" .");
    progressCount++;
    if(progressCount == 15)
	    progressCount = 0;
}

void ImportLibraryDialog::closeEvent ( QCloseEvent * e )
{
    close();
}
