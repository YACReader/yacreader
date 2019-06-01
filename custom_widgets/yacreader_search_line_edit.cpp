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
    connect(this, SIGNAL(textChanged(const QString &)), this, SLOT(updateCloseButton(const QString &)));
    int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
#ifdef Q_OS_MAC
    setStyleSheet(QString("QLineEdit {border-top:1px solid #9F9F9F; border-bottom:1px solid #ACACAC; border-right:1px solid #ACACAC; border-left:1px solid #ACACAC; border-radius: 10px; background-color:qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #CACACA, stop: 0.15 #FFFFFF); padding-left: %1px; padding-right: %2px; padding-bottom: 1px; margin-bottom: 1px;} ").arg(searchLabel->sizeHint().width() + frameWidth + 6).arg(clearButton->sizeHint().width() + frameWidth + 2));
#else
    setStyleSheet(QString("QLineEdit {color: #ABABAB; border:none; border-radius: 4px; background-color:#404040; padding-left: %1px; padding-right: %2px; padding-bottom: 1px; margin-right: 9px;} ").arg(searchLabel->sizeHint().width() + frameWidth + 6 + 5).arg(clearButton->sizeHint().width() + frameWidth + 2));
#endif
    QSize msz = minimumSizeHint();
    setMinimumSize(qMax(msz.width(), clearButton->sizeHint().height() + frameWidth * 2 + 2),
                   qMax(msz.height(), clearButton->sizeHint().height() + frameWidth * 2 + 2));

#ifdef Q_OS_MAC
    setMaximumWidth(212);
#else
    setMaximumWidth(173);
    setFixedHeight(26);
#endif

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
#ifdef Q_OS_MAC
    QSize sz = clearButton->sizeHint();
    int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    clearButton->move(rect().right() - frameWidth - sz.width(),
                      (rect().bottom() + 1 - sz.height()) / 2);

    QSize szl = searchLabel->sizeHint();
    searchLabel->move(6, (rect().bottom() + 1 - szl.height()) / 2);
#else
    QSize sz = clearButton->sizeHint();
    int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    int marginRight = style()->pixelMetric(QStyle::PM_LayoutRightMargin);
    clearButton->move(rect().right() - frameWidth - sz.width() - marginRight - 6,
                      (rect().bottom() + 2 - sz.height()) / 2);

    QSize szl = searchLabel->sizeHint();
    searchLabel->move(8, (rect().bottom() + 2 - szl.height()) / 2);
#endif
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
