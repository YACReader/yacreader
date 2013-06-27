#include "add_library_dialog.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QGridLayout>


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
	connect(path,SIGNAL(textChanged(QString)),this,SLOT(pathSetted(QString)));

	nameLabel = new QLabel(tr("Library Name : "));
	nameEdit = new QLineEdit;
	nameLabel->setBuddy(nameEdit);
	connect(nameEdit,SIGNAL(textChanged(QString)),this,SLOT(nameSetted(QString)));

	accept = new QPushButton(tr("Add"));
	accept->setDisabled(true);
	connect(accept,SIGNAL(clicked()),this,SLOT(add()));

	cancel = new QPushButton(tr("Cancel"));
	connect(cancel,SIGNAL(clicked()),this,SLOT(close()));

	find = new QPushButton(QIcon(":/images/comicFolder.png"),"");
	connect(find,SIGNAL(clicked()),this,SLOT(findPath()));

	QGridLayout * content = new QGridLayout;

	content->addWidget(nameLabel,0,0);
	content->addWidget(nameEdit,0,1);

	content->addWidget(textLabel,1,0);
	content->addWidget(path,1,1);
	content->addWidget(find,1,2);
	content->setColumnStretch(2,0);

	QHBoxLayout *bottomLayout = new QHBoxLayout;
	bottomLayout->addStretch();
	bottomLayout->addWidget(accept);
	bottomLayout->addWidget(cancel);

	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addLayout(content);
	mainLayout->addStretch();
	mainLayout->addLayout(bottomLayout);

	QHBoxLayout * imgMainLayout = new QHBoxLayout;
	QLabel * imgLabel = new QLabel(this);
	QPixmap p(":/images/openLibrary.png");
	imgLabel->setPixmap(p);
	imgMainLayout->addWidget(imgLabel);//,0,Qt::AlignTop);
	imgMainLayout->addLayout(mainLayout);
	
	setLayout(imgMainLayout);

	setModal(true);
	setWindowTitle(tr("Add an existing library"));
}

void AddLibraryDialog::add()
{
	//accept->setEnabled(false);
	emit(addLibrary(QDir::cleanPath(path->text()),nameEdit->text()));
}

void AddLibraryDialog::nameSetted(const QString & text)
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

void AddLibraryDialog::pathSetted(const QString & text)
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

void AddLibraryDialog::findPath()
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

void AddLibraryDialog::close()
{
	path->clear();
	nameEdit->clear();
	accept->setEnabled(false);
	QDialog::close();
}