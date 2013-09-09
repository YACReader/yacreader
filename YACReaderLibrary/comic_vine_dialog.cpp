#include "comic_vine_dialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <QRadioButton>

ComicVineDialog::ComicVineDialog(QWidget *parent) :
	QDialog(parent)
{
	doLayout();
	doStackedWidgets();
	doConnections();
}

void ComicVineDialog::doLayout()
{
	setStyleSheet(""
		"QDialog {background-color: #404040; }"
		"");

	QString dialogButtonsStyleSheet = "QPushButton {border: 1px solid #242424; background: #2e2e2e; color:white; padding: 5px 26px 5px 26px; font-size:12px;font-family:Arial; font-weight:bold;}";

	QLabel * mainTitleLabel = new QLabel(tr("SEARCH"));
	QLabel * subTitleLabel = new QLabel(tr("%1 comics selected"));

	mainTitleLabel->setStyleSheet("QLabel {color:white; font-size:18px;font-family:Arial;}");
	subTitleLabel->setStyleSheet("QLabel {color:white; font-size:12px;font-family:Arial;}");

	nextButton = new QPushButton(tr("next"));
	closeButton = new QPushButton(tr("close"));

	nextButton->setStyleSheet(dialogButtonsStyleSheet);
	closeButton->setStyleSheet(dialogButtonsStyleSheet);

	content = new QStackedWidget(this);
	//

	QVBoxLayout * mainLayout = new QVBoxLayout;
	QHBoxLayout * titleLayout = new QHBoxLayout;
	QVBoxLayout * titleLabelsLayout = new QVBoxLayout;
	QHBoxLayout * buttonLayout = new QHBoxLayout;

	titleLabelsLayout->addWidget(mainTitleLabel);
	titleLabelsLayout->addWidget(subTitleLabel);
	titleLabelsLayout->setSpacing(0);

	titleLayout->addLayout(titleLabelsLayout);

	buttonLayout->addStretch();
	buttonLayout->addWidget(nextButton);
	buttonLayout->addWidget(closeButton);
	buttonLayout->setContentsMargins(0,0,0,0);

	mainLayout->addLayout(titleLayout);
	mainLayout->addWidget(content);
	mainLayout->addStretch();
	mainLayout->addLayout(buttonLayout);

	mainLayout->setContentsMargins(26,16,26,11);

	setLayout(mainLayout);
	setFixedSize(672,529);
}

void ComicVineDialog::doStackedWidgets()
{
	doSeriesQuestion();
}
void ComicVineDialog::doSeriesQuestion()
{
	QWidget * w = new QWidget;
	QVBoxLayout * l = new QVBoxLayout;

	QLabel * questionLabel = new QLabel(tr("You are trying to get information for various comics at once, are they part of the same series?"));
	questionLabel->setStyleSheet("QLabel {color:white; font-size:12px;font-family:Arial;}");
	QRadioButton * yes = new QRadioButton(tr("yes"));
	QRadioButton * no = new QRadioButton(tr("no"));

	QString rbStyle = "QRadioButton {margin-left:27px; margin-top:5px; color:white;font-size:12px;font-family:Arial;}"
		"QRadioButton::indicator {width:11px;height:11px;}"
		"QRadioButton::indicator::unchecked {image : url(:/images/comic_vine/radioUnchecked.png);}"
		"QRadioButton::indicator::checked {image : url(:/images/comic_vine/radioChecked.png);}";
	yes->setStyleSheet(rbStyle);
	no->setStyleSheet(rbStyle);

	yes->setChecked(true);

	l->addSpacing(35);
	l->addWidget(questionLabel);
	l->addWidget(yes);
	l->addWidget(no);
	l->addStretch();

	l->setContentsMargins(0,0,0,0);
	w->setLayout(l);
	w->setContentsMargins(0,0,0,0);
	content->addWidget(w);
}

void ComicVineDialog::doConnections()
{
	connect(closeButton,SIGNAL(pressed()),this,SLOT(close()));
}