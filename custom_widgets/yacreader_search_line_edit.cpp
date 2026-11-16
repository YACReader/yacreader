#include "yacreader_search_line_edit.h"

#include <QLabel>
#include <QMenu>
#include <QPainter>
#include <QStyle>
#include <QToolButton>

namespace {
const int menuButtonSpacing = 2;
}

YACReaderSearchLineEdit::YACReaderSearchLineEdit(QWidget *parent)
    : QLineEdit(parent), paddingLeft(0), paddingRight(0), basePaddingRight(0)
{
    clearButton = new QToolButton(this);
    menuButton = new QToolButton(this);
    searchLabel = new QLabel(this);

    clearButton->setIconSize(QSize(12, 12));
    menuButton->setAutoRaise(true);
    menuButton->setFixedSize(18, 18);
    menuButton->setIconSize(QSize(10, 6));
    menuButton->setCursor(Qt::ArrowCursor);
    menuButton->setPopupMode(QToolButton::InstantPopup);
    menuButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    menuButton->setToolTip(tr("Search filters"));
    menuButton->hide();

    clearButton->setCursor(Qt::ArrowCursor);
    clearButton->hide();
    connect(clearButton, &QAbstractButton::clicked, this, &QLineEdit::clear);
    connect(this, &QLineEdit::textChanged, this, &YACReaderSearchLineEdit::updateCloseButton);

    int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    paddingLeft = 15 + frameWidth + 6 + 5;
    basePaddingRight = 12 + frameWidth + 10;
    paddingRight = basePaddingRight;

    QSize msz = minimumSizeHint();
    setMinimumSize(qMax(msz.width(), clearButton->sizeHint().height() + frameWidth * 2 + 2),
                   qMax(msz.height(), clearButton->sizeHint().height() + frameWidth * 2 + 2));

    setMaximumWidth(255);
    setFixedHeight(26);

    setPlaceholderText(tr("type to search"));

    connect(this, &QLineEdit::textChanged, this, &YACReaderSearchLineEdit::processText);

    initTheme(this);
}

void YACReaderSearchLineEdit::setSearchMenu(QMenu *menu)
{
    menuButton->setMenu(menu);
    menuButton->setVisible(menu != nullptr);

    paddingRight = basePaddingRight + (menu != nullptr ? menuButton->width() + menuButtonSpacing : 0);
    applyTheme(theme);
    positionButtons();
}

void YACReaderSearchLineEdit::applyTheme(const Theme &theme)
{
    const auto &searchTheme = theme.searchLineEdit;

    setStyleSheet(searchTheme.lineEditQSS.arg(paddingLeft).arg(paddingRight));
    searchLabel->setStyleSheet(searchTheme.searchLabelQSS);
    clearButton->setStyleSheet(searchTheme.clearButtonQSS);
    menuButton->setStyleSheet(QStringLiteral(
            "QToolButton { border: none; padding: 0px; }"
            "QToolButton::menu-indicator { image: none; width: 0px; }"));

    searchLabel->setPixmap(searchTheme.searchIcon);
    clearButton->setIcon(QIcon(searchTheme.clearIcon));

    const qreal dpr = devicePixelRatioF();
    QPixmap chevron(qCeil(10 * dpr), qCeil(6 * dpr));
    chevron.setDevicePixelRatio(dpr);
    chevron.fill(Qt::transparent);

    QPainter painter(&chevron);
    painter.setRenderHint(QPainter::Antialiasing);
    QPen pen(searchTheme.iconColor);
    pen.setWidthF(1.4);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    painter.setPen(pen);
    painter.drawPolyline(QPolygonF {
            QPointF(1, 1),
            QPointF(5, 5),
            QPointF(9, 1) });
    painter.end();

    menuButton->setIcon(QIcon(chevron));
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
    positionButtons();

    QSize szl = searchLabel->sizeHint();
    searchLabel->move(8, (rect().bottom() + 2 - szl.height()) / 2);
}

void YACReaderSearchLineEdit::positionButtons()
{
    const QSize clearSize = clearButton->sizeHint();
    const int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    const int marginRight = style()->pixelMetric(QStyle::PM_LayoutRightMargin);
    const int rightEdge = rect().right() - frameWidth - marginRight - 6;
    const int menuX = rightEdge - menuButton->width();
    menuButton->move(menuX, (height() - menuButton->height()) / 2);

    const int clearRight = menuButton->menu() != nullptr ? menuX - menuButtonSpacing : rightEdge;
    clearButton->move(clearRight - clearSize.width(),
                      (rect().bottom() + 2 - clearSize.height()) / 2);
}

void YACReaderSearchLineEdit::updateCloseButton(const QString &text)
{
    clearButton->setVisible(!text.isEmpty());
}

void YACReaderSearchLineEdit::processText(const QString &text)
{
    emit filterChanged(text);
}
