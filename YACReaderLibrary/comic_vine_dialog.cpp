#include "comic_vine_dialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <QRadioButton>
#include <QMessageBox>

#include "yacreader_busy_widget.h"
#include "comic_vine_client.h"

ComicVineDialog::ComicVineDialog(QWidget *parent) :
	QDialog(parent),comicVineClient(new ComicVineClient)
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

	nextButton = new QPushButton(tr("next"));
	closeButton = new QPushButton(tr("close"));

	nextButton->setStyleSheet(dialogButtonsStyleSheet);
	closeButton->setStyleSheet(dialogButtonsStyleSheet);

	content = new QStackedWidget(this);

	QVBoxLayout * mainLayout = new QVBoxLayout;

	QHBoxLayout * buttonLayout = new QHBoxLayout;

	buttonLayout->addStretch();
	buttonLayout->addWidget(nextButton);
	buttonLayout->addWidget(closeButton);
	buttonLayout->setContentsMargins(0,0,0,0);

	mainLayout->addWidget(titleHeader = new TitleHeader);
	mainLayout->addWidget(content);
	mainLayout->addStretch();
	mainLayout->addLayout(buttonLayout);

	mainLayout->setContentsMargins(26,16,26,11);

	setLayout(mainLayout);
	setFixedSize(672,529);
}

void ComicVineDialog::doStackedWidgets()
{
	doLoading();
	content->addWidget(seriesQuestion = new SeriesQuestion);
	content->addWidget(searchSingleComic = new SearchSingleComic);
	content->addWidget(searchVolume = new SearchVolume);

}

void ComicVineDialog::doConnections()
{
	connect(closeButton,SIGNAL(pressed()),this,SLOT(close()));
	connect(nextButton,SIGNAL(pressed()),this,SLOT(goNext()));

	connect(comicVineClient,SIGNAL(searchResult(QString)),this,SLOT(debugClientResults(QString)));
}

void ComicVineDialog::goNext()
{
	//
	if(content->currentWidget() == seriesQuestion)
	{
		if(seriesQuestion->getYes())
		{
			content->setCurrentWidget(searchVolume);
		}
		else
		{
			ComicDB comic = comics[currentIndex];
			QString title = comic.getTitleOrPath();
			titleHeader->setSubTitle(tr("comic %1 of %2 - %3").arg(currentIndex+1).arg(comics.length()).arg(title));
			content->setCurrentWidget(searchSingleComic);
		}
	}
	else if (content->currentWidget() == searchSingleComic) {

	}
}

void ComicVineDialog::setComics(const QList<ComicDB> & comics)
{
	this->comics = comics;
}

void ComicVineDialog::show()
{
	QDialog::show();

	currentIndex = 0;

	if(comics.length() == 1)
	{
		ComicDB singleComic = comics[0];
		QString title = singleComic.getTitleOrPath();
		titleHeader->setSubTitle(title);
		content->setCurrentIndex(0);

		comicVineClient->search(title);
	}else if(comics.length()>1)
	{
		titleHeader->setSubTitle(tr("%1 comics selected").arg(comics.length()));
		content->setCurrentWidget(seriesQuestion);
	}

}

void ComicVineDialog::doLoading()
{
	QWidget * w = new QWidget;
	QVBoxLayout * l = new QVBoxLayout;

	YACReaderBusyWidget * bw = new YACReaderBusyWidget;

	l->addStretch();
	l->addWidget(bw,0,Qt::AlignHCenter);

	l->setContentsMargins(0,0,0,0);
	w->setLayout(l);
	w->setContentsMargins(0,0,0,0);

	content->addWidget(w);
}

void ComicVineDialog::debugClientResults(const QString & string)
{
	content->setCurrentWidget(searchSingleComic);
	QMessageBox::information(0,"-Response-", string);

}

//---------------------------------------
//TitleHeader
//---------------------------------------
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

//---------------------------------------
//SeriesQuestion
//---------------------------------------
SeriesQuestion::SeriesQuestion(QWidget * parent)
	:QWidget(parent)
{
	QVBoxLayout * l = new QVBoxLayout;

	QLabel * questionLabel = new QLabel(tr("You are trying to get information for various comics at once, are they part of the same series?"));
	questionLabel->setStyleSheet("QLabel {color:white; font-size:12px;font-family:Arial;}");
	yes = new QRadioButton(tr("yes"));
	no  = new QRadioButton(tr("no"));

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
	setLayout(l);
	setContentsMargins(0,0,0,0);
}

bool SeriesQuestion::getYes()
{
	return yes->isChecked();
}
//---------------------------------------
//ScrapperLineEdit
//---------------------------------------
ScrapperLineEdit::ScrapperLineEdit(const QString & title, QWidget * widget)
	:QLineEdit(widget)
{
	titleLabel = new QLabel(title,this);
	titleLabel->setStyleSheet("QLabel {color:white;}");

	setStyleSheet(QString("QLineEdit {"
		"border:none; background-color: #2E2E2E; color : white; padding-left: %1; padding-bottom: 1px; margin-bottom: 0px;" 
		"}").arg(titleLabel->sizeHint().width()+6));

	setFixedHeight(22);
}

void ScrapperLineEdit::resizeEvent(QResizeEvent *)
{
	QSize szl = titleLabel->sizeHint();
	titleLabel->move(6,(rect().bottom() + 1 - szl.height())/2);
}

//---------------------------------------
//SearchSingleComic
//---------------------------------------
SearchSingleComic::SearchSingleComic(QWidget * parent)
	:QWidget(parent)
{

	QLabel * label = new QLabel(tr("No results found, please provide some aditional information. At least one field is needed."));
	label->setStyleSheet("QLabel {color:white; font-size:12px;font-family:Arial;}");

	titleEdit = new ScrapperLineEdit(tr("Title:"));
	numberEdit = new ScrapperLineEdit(tr("Number:"));
	volumeEdit = new ScrapperLineEdit(tr("Series:"));

	numberEdit->setMaximumWidth(126);

	QVBoxLayout * l = new QVBoxLayout;
	QHBoxLayout * hl = new QHBoxLayout;
	hl->addWidget(titleEdit);
	hl->addWidget(numberEdit);

	l->addSpacing(35);
	l->addWidget(label);
	l->addLayout(hl);
	l->addWidget(volumeEdit);
	l->addStretch();

	l->setContentsMargins(0,0,0,0);
	setLayout(l);
	setContentsMargins(0,0,0,0);
}

//---------------------------------------
//SearchVolume
//---------------------------------------
SearchVolume::SearchVolume(QWidget * parent)
	:QWidget(parent)
{
	QLabel * label = new QLabel(tr("No results found, please provide some aditional information."));
	label->setStyleSheet("QLabel {color:white; font-size:12px;font-family:Arial;}");

	volumeEdit = new ScrapperLineEdit(tr("Series:"));

	QVBoxLayout * l = new QVBoxLayout;

	l->addSpacing(35);
	l->addWidget(label);
	l->addWidget(volumeEdit);
	l->addStretch();

	l->setContentsMargins(0,0,0,0);
	setLayout(l);
	setContentsMargins(0,0,0,0);
}
