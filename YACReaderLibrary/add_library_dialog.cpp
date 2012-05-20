#include "add_library_dialog.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileDialog>



AddLibraryDialog::AddLibraryDialog(QWidget * parent)
:QDialog(parent)
{
	setupUI();
}

void AddLibraryDialog::setupUI()
{
	textLabel = new QLabel(tr("Comics folder : "));
	path = new QLineEdit;
	textLabel->setBuddy(path);

	nameLabel = new QLabel(tr("Library Name : "));
	nameEdit = new QLineEdit;
	nameLabel->setBuddy(nameEdit);

	accept = new QPushButton(tr("Add"));
	accept->setDisabled(true);
	connect(accept,SIGNAL(clicked()),this,SLOT(add()));

	cancel = new QPushButton(tr("Cancel"));
	connect(cancel,SIGNAL(clicked()),this,SLOT(close()));

	find = new QPushButton(QIcon(":/images/comicFolder.png"),"");
	connect(find,SIGNAL(clicked()),this,SLOT(findPath()));

	QHBoxLayout *nameLayout = new QHBoxLayout;

	nameLayout->addWidget(nameLabel);
	nameLayout->addWidget(nameEdit);

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
	mainLayout->addLayout(nameLayout);
	mainLayout->addLayout(libraryLayout);
	mainLayout->addStretch();
	mainLayout->addLayout(bottomLayout);

	QHBoxLayout * imgMainLayout = new QHBoxLayout;
	QLabel * imgLabel = new QLabel(this);
	QPixmap p(":/images/openLibrary.png");
	imgLabel->setPixmap(p);
	imgMainLayout->addWidget(imgLabel);
	imgMainLayout->addLayout(mainLayout);
	
	setLayout(imgMainLayout);

	setModal(true);
	setWindowTitle(tr("Add an existing library"));
}

void AddLibraryDialog::add()
{
	//accept->setEnabled(false);
	emit(addLibrary(QDir::cleanPath(path->text()),nameEdit->text()));
	close();
}

void AddLibraryDialog::findPath()
{
	QString s = QFileDialog::getExistingDirectory(0,"Comics directory",".");
	if(!s.isEmpty())
	{
		path->setText(s);
		accept->setEnabled(true);
	}
}

void AddLibraryDialog::close()
{
	path->clear();
	nameEdit->clear();
	accept->setEnabled(false);
	QDialog::close();
}