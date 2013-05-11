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

	QLabel * text = new QLabel("<font color=\"#495252\">"+tr("You don't have any librarires yet")+"</font>");
	text->setStyleSheet("QLabel {font-size:25px;font-weight:bold;}");
	QLabel * textDescription = new QLabel("<font color=\"#565959\">"+tr("Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.")+"</font>");
	textDescription->setWordWrap(true);
	textDescription->setMaximumWidth(330);

	QPushButton * createButton = new QPushButton(tr("create your first library"));
	createButton->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
	QPushButton * addButton = new QPushButton(tr("add an existing one"));
	addButton->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);

	QVBoxLayout * layout = new QVBoxLayout(this);
	QHBoxLayout * buttonLayout = new QHBoxLayout();
	QHBoxLayout * topLayout = new QHBoxLayout();
	QVBoxLayout * textLayout = new QVBoxLayout();

	QWidget * topWidget = new QWidget();
	topWidget->setFixedSize(650,160);
	textLayout->addSpacing(12);
	textLayout->addWidget(text);
	textLayout->addSpacing(12);
	textLayout->addWidget(textDescription);
	textLayout->addStretch();

	topLayout->addStretch();
	topLayout->addWidget(iconLabel,0,Qt::AlignVCenter);
	topLayout->addSpacing(30);
	topLayout->addLayout(textLayout,1);
	topLayout->addStretch();
	topLayout->setMargin(0);

	topWidget->setLayout(topLayout);

	layout->setAlignment(Qt::AlignHCenter);

	buttonLayout->addSpacing(125);
	buttonLayout->addWidget(createButton);
	layout->addSpacing(25);
	buttonLayout->addWidget(addButton);
	buttonLayout->addSpacing(125);

	layout->addStretch();
	layout->addWidget(topWidget);
	layout->addSpacing(20);
	layout->addWidget(lineLabel,0,Qt::AlignHCenter);
	layout->addSpacing(10);
	layout->addLayout(buttonLayout,0);
	layout->addSpacing(150);
	layout->addStretch();

	connect(createButton,SIGNAL(clicked()),this,SIGNAL(createNewLibrary()));
	connect(addButton,SIGNAL(clicked()),this,SIGNAL(addExistingLibrary()));
	
	
}
