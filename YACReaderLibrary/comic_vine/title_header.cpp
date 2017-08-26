#include "title_header.h"

#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>

TitleHeader::TitleHeader(QWidget * parent )
	:QWidget(parent)
{
	mainTitleLabel = new QLabel();
	subTitleLabel = new QLabel();

	mainTitleLabel->setStyleSheet("QLabel {color:white; font-size:18px;font-family:Arial;}");
	subTitleLabel->setStyleSheet("QLabel {color:white; font-size:12px;font-family:Arial;}");

	QHBoxLayout * titleLayout = new QHBoxLayout;
	QVBoxLayout * titleLabelsLayout = new QVBoxLayout;

	titleLabelsLayout->addWidget(mainTitleLabel);
	titleLabelsLayout->addWidget(subTitleLabel);
	titleLabelsLayout->setSpacing(0);

	titleLayout->addLayout(titleLabelsLayout);
	titleLayout->setContentsMargins(0,0,0,0);

	setLayout(titleLayout);

	setContentsMargins(0,0,0,0);

	setTitle(tr("SEARCH"));
}

void TitleHeader::setTitle(const QString & title)
{
	mainTitleLabel->setText(title);
}

void TitleHeader::setSubTitle(const QString & title)
{
	subTitleLabel->setText(title);
}

void TitleHeader::showButtons(bool show)
{
	if(show)
	{

	}
	else
	{

	}
}
