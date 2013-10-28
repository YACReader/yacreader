#include "export_library_dialog.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>

ExportLibraryDialog::ExportLibraryDialog(QWidget * parent)
:QDialog(parent),progressCount(0)
{
	textLabel = new QLabel(tr("Output folder : "));
	path = new QLineEdit;
	textLabel->setBuddy(path);

	accept = new QPushButton(tr("Create"));
	accept->setDisabled(true);
	connect(accept,SIGNAL(clicked()),this,SLOT(exportLibrary()));

	cancel = new QPushButton(tr("Cancel"));
	connect(cancel,SIGNAL(clicked()),this,SLOT(close()));
	connect(cancel,SIGNAL(clicked()),this,SIGNAL(rejected()));

	find = new QPushButton(QIcon(":/images/find_folder.png"),"");
	connect(find,SIGNAL(clicked()),this,SLOT(findPath()));

	QHBoxLayout *libraryLayout = new QHBoxLayout;

	libraryLayout->addWidget(textLabel);
	libraryLayout->addWidget(path);
	libraryLayout->addWidget(find);
	libraryLayout->setStretchFactor(find,0); //TODO

	QHBoxLayout *bottomLayout = new QHBoxLayout;
	bottomLayout->addStretch();
	bottomLayout->addWidget(accept);
	bottomLayout->addWidget(cancel);

	progressBar = new QProgressBar(this);
	progressBar->setMinimum(0);
	progressBar->setMaximum(0);
	progressBar->setTextVisible(false);
	progressBar->hide();

	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addLayout(libraryLayout);
	mainLayout->addStretch();
	mainLayout->addWidget(progressBar);
	mainLayout->addLayout(bottomLayout);

	QHBoxLayout * imgMainLayout = new QHBoxLayout;
	QLabel * imgLabel = new QLabel(this);
	QPixmap p(":/images/exportLibrary.png");
	imgLabel->setPixmap(p);
	imgMainLayout->addWidget(imgLabel);
	imgMainLayout->addLayout(mainLayout);
	
	setLayout(imgMainLayout);

	setModal(true);
	setWindowTitle(tr("Create covers package"));
}

void ExportLibraryDialog::exportLibrary()
{
	QFileInfo f(path->text());
	if(f.exists() && f.isDir() && f.isWritable())
	{
		progressBar->show();
		accept->setEnabled(false);
		emit exportPath(QDir::cleanPath(path->text()));
	}
	else
		QMessageBox::critical(NULL,tr("Problem found while writing"),tr("The selected path for the output file does not exist or is not a valid path. Be sure that you have write access to this folder"));

}

void ExportLibraryDialog::findPath()
{
	QString s = QFileDialog::getExistingDirectory(0,tr("Destination directory"),".");
	if(!s.isEmpty())
	{
		path->setText(s);
		accept->setEnabled(true);
	}
}

void ExportLibraryDialog::close()
{
	path->clear();
	progressBar->hide();
	accept->setEnabled(false);
	progressCount=0;
	QDialog::close();
}

void ExportLibraryDialog::run()
{

}

