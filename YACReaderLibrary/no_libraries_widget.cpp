#include "no_libraries_widget.h"
#include <QLabel>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>

NoLibrariesWidget::NoLibrariesWidget(QWidget *parent) :
    QWidget(parent)
{
	setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

	QPalette p(palette());
    p.setColor(QPalette::Background, QColor(250,250,250));
    setAutoFillBackground(true);
    setPalette(p);

	QPixmap icon(":/images/noLibrariesIcon.png");
	QLabel * iconLabel  = new QLabel();
	iconLabel->setPixmap(icon);

	QPixmap line(":/images/noLibrariesLine.png");
	QLabel * lineLabel = new QLabel();
	lineLabel->setPixmap(line);

	QLabel * text = new QLabel(tr("<font color=\"#565959\">You don't have any librarires yet</font>"));
	text->setStyleSheet("QLabel {font-size:25px;font-weight:bold;}");

	QPushButton * createButton = new QPushButton(tr("create your first library"));
	createButton->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
	QPushButton * addButton = new QPushButton(tr("add an existing one"));
	addButton->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);

	QVBoxLayout * layout = new QVBoxLayout(this);
	QHBoxLayout * buttonLayout = new QHBoxLayout();
	//QHBoxLayout * topLayout = new QHBoxLayout();
	//QVBoxLayout * textLayout = new QVBoxLayout();

	layout->setAlignment(Qt::AlignHCenter);

	buttonLayout->addSpacing(100);
	buttonLayout->addWidget(createButton);
	layout->addSpacing(25);
	buttonLayout->addWidget(addButton);
	buttonLayout->addSpacing(100);

	layout->addStretch();
	layout->addWidget(iconLabel,0,Qt::AlignHCenter);
	layout->addSpacing(10);
	layout->addWidget(text,0,Qt::AlignHCenter);
	layout->addSpacing(10);
	layout->addWidget(lineLabel,0,Qt::AlignHCenter);
	layout->addSpacing(10);
	layout->addLayout(buttonLayout,0);
	layout->addSpacing(150);
	layout->addStretch();

	connect(createButton,SIGNAL(clicked()),this,SIGNAL(createNewLibrary()));
	connect(addButton,SIGNAL(clicked()),this,SIGNAL(addExistingLibrary()));
	
	
}
