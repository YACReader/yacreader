#include "scrapper_lineedit.h"
#include <QLabel>

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
