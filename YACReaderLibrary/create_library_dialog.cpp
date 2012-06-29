#include "create_library_dialog.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileDialog>



CreateLibraryDialog::CreateLibraryDialog(QWidget * parent)
:QDialog(parent)
{
	setupUI();
}

void CreateLibraryDialog::setupUI()
{
	textLabel = new QLabel(tr("Comics folder : "));
	path = new QLineEdit;
	textLabel->setBuddy(path);

	nameLabel = new QLabel(tr("Library Name : "));
	nameEdit = new QLineEdit;
	nameLabel->setBuddy(nameEdit);

	accept = new QPushButton(tr("Create"));
	accept->setDisabled(true);
	connect(accept,SIGNAL(clicked()),this,SLOT(create()));

	cancel = new QPushButton(tr("Cancel"));
	connect(cancel,SIGNAL(clicked()),this,SIGNAL(cancelCreate()));
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

	QHBoxLayout *middleLayout = new QHBoxLayout;

	progressBar = new QProgressBar(this);
	progressBar->setMinimum(0);
	progressBar->setMaximum(0);
	progressBar->setTextVisible(false);
	progressBar->hide();

	currentFileLabel = new QLabel("");
	middleLayout->addWidget(currentFileLabel);
	middleLayout->addStretch();

	QHBoxLayout *bottomLayout = new QHBoxLayout;
	bottomLayout->addStretch();
	bottomLayout->addWidget(accept);
	bottomLayout->addWidget(cancel);

	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addLayout(nameLayout);
	mainLayout->addLayout(libraryLayout);
	mainLayout->addLayout(middleLayout);
	mainLayout->addStretch();
	mainLayout->addWidget(progressBar);
	mainLayout->addLayout(bottomLayout);

	QHBoxLayout * imgMainLayout = new QHBoxLayout;
	QLabel * imgLabel = new QLabel(this);
	QPixmap p(":/images/new.png");
	imgLabel->setPixmap(p);
	imgMainLayout->addWidget(imgLabel);
	imgMainLayout->addLayout(mainLayout);
	
	setLayout(imgMainLayout);

	setModal(true);
	setWindowTitle(tr("Create new library"));
}

void CreateLibraryDialog::create()
{
	progressBar->show();
	accept->setEnabled(false);
	emit(createLibrary(QDir::cleanPath(path->text()),QDir::cleanPath(path->text())+"/.yacreaderlibrary",nameEdit->text()));
}

void CreateLibraryDialog::findPath()
{
	QString s = QFileDialog::getExistingDirectory(0,"Comics directory",".");
	if(!s.isEmpty())
	{
		path->setText(s);
		accept->setEnabled(true);
	}
}

void CreateLibraryDialog::showCurrentFile(QString file)
{
	currentFileLabel->setText(file);
	currentFileLabel->update();
	this->update();
}
void CreateLibraryDialog::close()
{
	progressBar->hide();
	path->clear();
	nameEdit->clear();
	currentFileLabel->setText("");
	accept->setEnabled(true);
	QDialog::close();
}
//-----------------------------------------------------------------------------
// UpdateLibraryDialog
//-----------------------------------------------------------------------------
UpdateLibraryDialog::UpdateLibraryDialog(QWidget * parent)
:QDialog(parent)
{
	QVBoxLayout * mainLayout = new QVBoxLayout;
	mainLayout->addWidget(message = new QLabel(tr("Updating....")));
	mainLayout->addWidget(currentFileLabel = new QLabel(""));

	QHBoxLayout * bottom = new QHBoxLayout;
	bottom->addStretch();
	bottom->addWidget(cancel = new QPushButton(tr("Cancel")));

	connect(cancel,SIGNAL(clicked()),this,SIGNAL(cancelUpdate()));
	connect(cancel,SIGNAL(clicked()),this,SLOT(close()));
	
	mainLayout->addStretch();
	
	progressBar = new QProgressBar(this);
	progressBar->setMinimum(0);
	progressBar->setMaximum(0);
	progressBar->setTextVisible(false);

	mainLayout->addWidget(progressBar);
	mainLayout->addLayout(bottom);

	QHBoxLayout * imgMainLayout = new QHBoxLayout;
	QLabel * imgLabel = new QLabel(this);
	QPixmap p(":/images/updateLibrary.png");
	imgLabel->setPixmap(p);
	imgMainLayout->addWidget(imgLabel);
	imgMainLayout->addLayout(mainLayout);
	
	setLayout(imgMainLayout);

	setModal(true);
}

void UpdateLibraryDialog::showCurrentFile(QString file)
{
	currentFileLabel->setText(file);
	currentFileLabel->update();
	this->update();
}

void UpdateLibraryDialog::close()
{
	currentFileLabel->setText("");
	QDialog::close();
}
