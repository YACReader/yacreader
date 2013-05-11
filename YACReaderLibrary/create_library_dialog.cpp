#include "create_library_dialog.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QSizePolicy>
#include <QMessageBox>

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
	connect(path,SIGNAL(textChanged(QString)),this,SLOT(pathSetted(QString)));

	nameLabel = new QLabel(tr("Library Name : "));
	nameEdit = new QLineEdit;
	nameLabel->setBuddy(nameEdit);
	connect(nameEdit,SIGNAL(textChanged(QString)),this,SLOT(nameSetted(QString)));

	accept = new QPushButton(tr("Create"));
	accept->setDisabled(true);
	connect(accept,SIGNAL(clicked()),this,SLOT(create()));

	cancel = new QPushButton(tr("Cancel"));
	connect(cancel,SIGNAL(clicked()),this,SIGNAL(cancelCreate()));
	connect(cancel,SIGNAL(clicked()),this,SLOT(close()));

	find = new QPushButton(QIcon(":/images/comicFolder.png"),"");
	connect(find,SIGNAL(clicked()),this,SLOT(findPath()));

	QGridLayout * content = new QGridLayout;

	//QHBoxLayout *nameLayout = new QHBoxLayout;

	content->addWidget(nameLabel,0,0);
	content->addWidget(nameEdit,0,1);

	//QHBoxLayout *libraryLayout = new QHBoxLayout;

	content->addWidget(textLabel,1,0);
	content->addWidget(path,1,1);
	content->addWidget(find,1,2);
	content->setColumnMinimumWidth(2,0); //TODO

	QHBoxLayout *middleLayout = new QHBoxLayout;

	progressBar = new QProgressBar(this);
	progressBar->setMinimum(0);
	progressBar->setMaximum(0);
	progressBar->setTextVisible(false);
	progressBar->hide();

	currentFileLabel = new QLabel("");
	currentFileLabel->setWordWrap(true);
	middleLayout->addWidget(currentFileLabel);
	middleLayout->addStretch();
	middleLayout->setSizeConstraint(QLayout::SetMaximumSize);


	QHBoxLayout *bottomLayout = new QHBoxLayout;
	bottomLayout->addWidget(message = new QLabel(tr("Create a library could take several minutes. You can stop the process and update the library later for completing the task.")));
	message->setWordWrap(true);
	//message->hide();
	bottomLayout->addStretch();
	bottomLayout->addWidget(accept);
	bottomLayout->addWidget(cancel);

	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addLayout(content);

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

	QFileInfo f(path->text());
	if(f.exists() && f.isDir() && f.isWritable())
	{
		progressBar->show();
		message->show();
		currentFileLabel->setText("Importing : \n\n\n\n\n");
		this->adjustSize();
		accept->setEnabled(false);
		emit(createLibrary(QDir::cleanPath(path->text()),QDir::cleanPath(path->text())+"/.yacreaderlibrary",nameEdit->text()));
		close();
	}
	else
		QMessageBox::critical(NULL,tr("Path not found"),tr("The selected path does not exist or is not a valid path. Be sure that you have write access to this folder"));
}

void CreateLibraryDialog::showCurrentFile(QString file)
{
	currentFileLabel->setText(tr("Importing : \n") + file);
	currentFileLabel->update();
	//this->adjustSize();
	//is->update();
}

void CreateLibraryDialog::nameSetted(const QString & text)
{
	if(!text.isEmpty())
	{
		if(!path->text().isEmpty())
		{
			QFileInfo fi(path->text());
			if(fi.isDir())
				accept->setEnabled(true);
			else
				accept->setEnabled(false);
		}
	}
	else
		accept->setEnabled(false);
}

void CreateLibraryDialog::pathSetted(const QString & text)
{
	QFileInfo fi(text);
	if(fi.isDir())
	{
		if(!nameEdit->text().isEmpty())
			accept->setEnabled(true);
	}
	else
		accept->setEnabled(false);
}

void CreateLibraryDialog::findPath()
{
	QString s = QFileDialog::getExistingDirectory(0,"Comics directory",".");
	if(!s.isEmpty())
	{
		path->setText(s);
		if(!nameEdit->text().isEmpty())
			accept->setEnabled(true);
	}
	else
		accept->setEnabled(false);
}

void CreateLibraryDialog::close()
{
	progressBar->hide();
	path->clear();
	nameEdit->clear();
	currentFileLabel->setText("");
	this->adjustSize();
	accept->setEnabled(false);
	QDialog::close();
}

void CreateLibraryDialog::setDataAndStart(QString name, QString path)
{
	this->path->setText(path);
	this->nameEdit->setText(name);
	show();
	create();
}
//-----------------------------------------------------------------------------
// UpdateLibraryDialog
//-----------------------------------------------------------------------------
UpdateLibraryDialog::UpdateLibraryDialog(QWidget * parent)
:QDialog(parent)
{
	QVBoxLayout * mainLayout = new QVBoxLayout;
	mainLayout->addWidget(message = new QLabel(tr("Updating....")));
	mainLayout->addWidget(currentFileLabel = new QLabel("\n\n\n\n"));
	currentFileLabel->setWordWrap(true);

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
	setWindowTitle(tr("Update library"));
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
	this->adjustSize();
	QDialog::close();
}
