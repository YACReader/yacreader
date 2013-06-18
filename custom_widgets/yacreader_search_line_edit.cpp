#include "yacreader_search_line_edit.h"

#include <QToolButton>
#include <QStyle>
#include <QLabel>

YACReaderSearchLineEdit::YACReaderSearchLineEdit(QWidget *parent)
    : QLineEdit(parent)
{
    clearButton = new QToolButton(this);
	searchLabel = new QLabel(this);

    QPixmap pixmap(":/images/clearSearch.png");
	QPixmap pixmapIcon(":/images/iconSearch.png");
	
	searchLabel->setStyleSheet("QLabel { border: none; padding: 0px; }");
	searchLabel->setPixmap(pixmapIcon);

    clearButton->setIcon(QIcon(pixmap));
    clearButton->setIconSize(pixmap.size());
    clearButton->setCursor(Qt::ArrowCursor);
    clearButton->setStyleSheet("QToolButton { border: none; padding: 0px; }");
    clearButton->hide();
    connect(clearButton, SIGNAL(clicked()), this, SLOT(clear()));
    connect(this, SIGNAL(textChanged(const QString&)), this, SLOT(updateCloseButton(const QString&)));
    int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
#ifdef Q_OS_MAC
    setStyleSheet(QString("QLineEdit {border-top:1px solid #9F9F9F; border-bottom:1px solid #ACACAC; border-right:1px solid #ACACAC; border-left:1px solid #ACACAC; border-radius: 10px; background-color:qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #CACACA, stop: 0.15 #FFFFFF); padding-left: %1px; padding-right: %2px; padding-bottom: 1px; margin-bottom: 1px;} ").arg(searchLabel->sizeHint().width() + frameWidth + 6).arg(clearButton->sizeHint().width() + frameWidth + 2));
#else
	setStyleSheet(QString("QLineEdit {border:none; background-color:qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #CACACA, stop: 0.15 #FFFFFF);; padding-left: %1px; padding-right: %2px; padding-bottom: 1px; margin-bottom: 0px;} ").arg(searchLabel->sizeHint().width() + frameWidth + 6).arg(clearButton->sizeHint().width() + frameWidth + 2));
#endif
    QSize msz = minimumSizeHint();
    setMinimumSize(qMax(msz.width(), clearButton->sizeHint().height() + frameWidth * 2 + 2),
                   qMax(msz.height(), clearButton->sizeHint().height() + frameWidth * 2 + 2));

#ifdef Q_OS_MAC
    setMaximumWidth(300);
#endif
}

void YACReaderSearchLineEdit::resizeEvent(QResizeEvent *)
{
    QSize sz = clearButton->sizeHint();
    int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    clearButton->move(rect().right() - frameWidth - sz.width(),
                      (rect().bottom() + 1 - sz.height())/2);

	QSize szl = searchLabel->sizeHint();
	searchLabel->move(6,(rect().bottom() + 1 - szl.height())/2);
}

void YACReaderSearchLineEdit::updateCloseButton(const QString& text)
{
    clearButton->setVisible(!text.isEmpty());
}
