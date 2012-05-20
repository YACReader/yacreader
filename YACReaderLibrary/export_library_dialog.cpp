#include "export_library_dialog.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileDialog>
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

	find = new QPushButton(QIcon(":/images/comicFolder.png"),"");
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

	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addLayout(libraryLayout);
	mainLayout->addWidget(progress=new QLabel());
	mainLayout->addStretch();
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
	t.setInterval(500);
	connect(&t,SIGNAL(timeout()),this,SLOT(updateProgress()));
}

void ExportLibraryDialog::exportLibrary()
{
	accept->setEnabled(false);
	emit exportPath(QDir::cleanPath(path->text()));
	t.start();
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
	accept->setEnabled(false);
	t.stop();
	progressCount=0;
	progress->setText("");
	QDialog::close();
}

void ExportLibraryDialog::run()
{

}

void ExportLibraryDialog::updateProgress()
{
	if(progressCount == 0)
		progress->setText(tr("Creating package ."));
	else
		progress->setText(progress->text()+" .");
	progressCount++;
	if(progressCount == 15)
		progressCount = 0;
	
}
