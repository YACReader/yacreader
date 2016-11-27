#include "scraper_scroll_label.h"

#include <QLabel>
#include <QDesktopServices>
#include <QUrl>

ScraperScrollLabel::ScraperScrollLabel(QWidget *parent) :
	QScrollArea(parent)
{
	textLabel = new QLabel(this);
	textLabel->setStyleSheet("QLabel {background-color: #2B2B2B; color:white; font-size:12px; font-family:Arial; }");

	textLabel->setWordWrap(true);
	textLabel->setMinimumSize(168,12);

	setWidget(textLabel);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setStyleSheet(
					"QScrollArea {background-color:#2B2B2B; border:none;}"
					"QScrollBar:vertical { border: none; background: #2B2B2B; width: 3px; margin: 0; }"
					"QScrollBar:horizontal { border: none; background: #2B2B2B; height: 3px; margin: 0; }"
					"QScrollBar::handle:vertical { background: #DDDDDD; width: 7px; min-height: 20px; }"
					"QScrollBar::handle:horizontal { background: #DDDDDD; width: 7px; min-height: 20px; }"
					"QScrollBar::add-line:vertical { border: none; background: #404040; height: 10px; subcontrol-position: bottom; subcontrol-origin: margin; margin: 0 3px 0 0;}"
					"QScrollBar::sub-line:vertical {  border: none; background: #404040; height: 10px; subcontrol-position: top; subcontrol-origin: margin; margin: 0 3px 0 0;}"
					"QScrollBar::add-line:horizontal { border: none; background: #404040; width: 10px; subcontrol-position: bottom; subcontrol-origin: margin; margin: 0 0 3px 0;}"
					"QScrollBar::sub-line:horizontal {  border: none; background: #404040; width: 10px; subcontrol-position: top; subcontrol-origin: margin; margin: 0 0 3px 0;}"
					"QScrollBar::up-arrow:vertical {border:none;width: 9px;height: 6px;background: url(':/images/folders_view/line-up.png') center top no-repeat;}"
					"QScrollBar::down-arrow:vertical {border:none;width: 9px;height: 6px;background: url(':/images/folders_view/line-down.png') center top no-repeat;}"
					"QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical, QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal {background: none; }"
				);

	connect(textLabel,SIGNAL(linkActivated(QString)),this,SLOT(openLink(QString)));
}

void ScraperScrollLabel::setAltText(const QString &text)
{
	textLabel->setAlignment(Qt::AlignTop|Qt::AlignHCenter);
	textLabel->setText(text);
	textLabel->adjustSize();
}

void ScraperScrollLabel::setText(const QString &text)
{
	textLabel->setAlignment(Qt::AlignTop|Qt::AlignLeft);
	textLabel->setText(text);
	textLabel->adjustSize();
}

void ScraperScrollLabel::openLink(const QString & link)
{
	QDesktopServices::openUrl(QUrl("http://www.comicvine.com"+link));
}
