#include "yacreader_search_line_edit.h"

#include <QToolButton>
#include <QStyle>
#include <QLabel>

YACReaderSearchLineEdit::YACReaderSearchLineEdit(QWidget *parent)
    : QLineEdit(parent), paddingLeft(0), paddingRight(0)
{
    clearButton = new QToolButton(this);
    searchLabel = new QLabel(this);

    clearButton->setIconSize(QSize(12, 12));

    clearButton->setCursor(Qt::ArrowCursor);
    clearButton->hide();
    connect(clearButton, &QAbstractButton::clicked, this, &QLineEdit::clear);
    connect(this, &QLineEdit::textChanged, this, &YACReaderSearchLineEdit::updateCloseButton);

    int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    paddingLeft = 15 + frameWidth + 6 + 5;
    paddingRight = 12 + frameWidth + 10;

    QSize msz = minimumSizeHint();
    setMinimumSize(qMax(msz.width(), clearButton->sizeHint().height() + frameWidth * 2 + 2),
                   qMax(msz.height(), clearButton->sizeHint().height() + frameWidth * 2 + 2));

    setMaximumWidth(255);
    setFixedHeight(26);

    setPlaceholderText(tr("type to search"));

    connect(this, &QLineEdit::textChanged, this, &YACReaderSearchLineEdit::processText);

    initTheme(this);
}

void YACReaderSearchLineEdit::applyTheme(const Theme &theme)
{
    const auto &searchTheme = theme.searchLineEdit;

    setStyleSheet(searchTheme.lineEditQSS.arg(paddingLeft).arg(paddingRight));
    searchLabel->setStyleSheet(searchTheme.searchLabelQSS);
    clearButton->setStyleSheet(searchTheme.clearButtonQSS);

    searchLabel->setPixmap(searchTheme.searchIcon);
    clearButton->setIcon(QIcon(searchTheme.clearIcon));
}

void YACReaderSearchLineEdit::clearText()
{
    disconnect(this, &QLineEdit::textChanged, this, &YACReaderSearchLineEdit::processText);
    clear();
    connect(this, &QLineEdit::textChanged, this, &YACReaderSearchLineEdit::processText);
}

const QString YACReaderSearchLineEdit::text()
{
    return QLineEdit::text();
}

void YACReaderSearchLineEdit::resizeEvent(QResizeEvent *)
{
    QSize sz = clearButton->sizeHint();
    int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    int marginRight = style()->pixelMetric(QStyle::PM_LayoutRightMargin);
    clearButton->move(rect().right() - frameWidth - sz.width() - marginRight - 6,
                      (rect().bottom() + 2 - sz.height()) / 2);

    QSize szl = searchLabel->sizeHint();
    searchLabel->move(8, (rect().bottom() + 2 - szl.height()) / 2);
}

void YACReaderSearchLineEdit::updateCloseButton(const QString &text)
{
    clearButton->setVisible(!text.isEmpty());
}

void YACReaderSearchLineEdit::processText(const QString &text)
{
    emit filterChanged(text);
}
