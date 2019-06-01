#include "yacreader_search_line_edit.h"

#include <QToolButton>
#include <QStyle>
#include <QLabel>

#include <QRegExpValidator>

#include "QsLog.h"

YACReaderSearchLineEdit::YACReaderSearchLineEdit(QWidget *parent)
    : QLineEdit(parent)
{
    clearButton = new QToolButton(this);
    searchLabel = new QLabel(this);

    QPixmap clearSearchPixmap(":/images/main_toolbar/clearSearch.svg");
    QPixmap inconSearchPixmap(":/images/main_toolbar/iconSearch.svg");

    clearSearchPixmap.setDevicePixelRatio(devicePixelRatio());
    inconSearchPixmap.setDevicePixelRatio(devicePixelRatio());

    searchLabel->setStyleSheet("QLabel { border: none; padding: 0px; }");
    searchLabel->setPixmap(inconSearchPixmap);

    clearButton->setIcon(QIcon(clearSearchPixmap));
    clearButton->setIconSize(clearSearchPixmap.size());
    clearButton->setCursor(Qt::ArrowCursor);
    clearButton->setStyleSheet("QToolButton { border: none; padding: 0px; }");
    clearButton->hide();
    connect(clearButton, SIGNAL(clicked()), this, SLOT(clear()));
    connect(this, SIGNAL(textChanged(const QString &)), this, SLOT(updateCloseButton(const QString &)));
    int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);

    setStyleSheet(QString("QLineEdit {color: #ABABAB; border:none; border-radius: 4px; background-color:#404040; padding-left: %1px; padding-right: %2px; padding-bottom: 1px; margin-right: 9px;} ").arg(searchLabel->sizeHint().width() + frameWidth + 6 + 5).arg(clearButton->sizeHint().width() + frameWidth + 2));

    QSize msz = minimumSizeHint();
    setMinimumSize(qMax(msz.width(), clearButton->sizeHint().height() + frameWidth * 2 + 2),
                   qMax(msz.height(), clearButton->sizeHint().height() + frameWidth * 2 + 2));

    setMaximumWidth(173);
    setFixedHeight(26);

    setAttribute(Qt::WA_MacShowFocusRect, false);
    setPlaceholderText(tr("type to search"));

    //search modifiers
    modifiers << "[read]"
              << "[unread]"; //<< "[author]";
    modifiersCompleter = new QCompleter(modifiers);

    QString regExpString;
    foreach (QString modifier, modifiers) {
        regExpString = regExpString + modifier.replace("[", "\\[").replace("]", "\\]") + ".*|";
    }

    regExpString = regExpString + "[^\\[].*";

    QLOG_TRACE() << regExpString;

    QRegExp regExp(regExpString);
    QValidator *validator = new QRegExpValidator(regExp, this);

    setValidator(validator);
    setCompleter(modifiersCompleter);

    connect(this, SIGNAL(textChanged(QString)), this, SLOT(processText(QString)));
}

void YACReaderSearchLineEdit::clearText()
{
    disconnect(this, SIGNAL(textChanged(QString)), this, SLOT(processText(QString)));
    clear();
    connect(this, SIGNAL(textChanged(QString)), this, SLOT(processText(QString)));
}

//modifiers are not returned
const QString YACReaderSearchLineEdit::text()
{
    QString text = QLineEdit::text();

    QRegExp regExp("\\[.*\\]");
    return text.remove(regExp).trimmed();
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

    QRegExp regExp("(\\[.*\\])(.*)");
    if (text.startsWith("[")) {
        if (regExp.exactMatch(text)) //avoid search while the modifiers are being written
        {
            QString modifier = regExp.cap(1);
            QString searchText = regExp.cap(2).trimmed();

            int indexOfModifier = modifiers.indexOf(modifier);
            if (indexOfModifier != -1) {
                QLOG_TRACE() << "modifier : " << modifier << "text : " << searchText;
                emit filterChanged(static_cast<YACReader::SearchModifiers>(indexOfModifier + 1), searchText); //TODO, do not use on indexOF
            } else {
                QLOG_ERROR() << "invalid modifier : " << modifier;
            }
        }

        QLOG_TRACE() << "full text :" << text << " : " << regExp.indexIn(text);
    } else {
        QLOG_TRACE() << "NoModifiers : " << text;
        emit filterChanged(YACReader::NoModifiers, text);
    }
}
