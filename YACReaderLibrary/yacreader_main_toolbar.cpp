#include "yacreader_main_toolbar.h"

#include <QPainter>
#include <QHBoxLayout>
#include <QToolButton>
#include <QLabel>
#include <QResizeEvent>
#include <QPaintEvent>
#include <QAction>

YACReaderMainToolBar::YACReaderMainToolBar(QWidget *parent)
    : QWidget(parent)
{
    mainLayout = new QHBoxLayout;

    currentFolder = new QLabel(this);
    //currentFolder->setAlignment(Qt::AlignCenter);
    currentFolder->setStyleSheet(" QLabel {color:#404040; font-size:22px; font-weight:bold;}");

    QFont f = currentFolder->font();
    f.setStyleStrategy(QFont::PreferAntialias);
    currentFolder->setFont(f);

    QString qToolButtonStyleSheet = "QToolButton {border:none;}";

    backButton = new QToolButton();
    backButton->setStyleSheet(qToolButtonStyleSheet);

    forwardButton = new QToolButton();
    forwardButton->setStyleSheet(qToolButtonStyleSheet);
    forwardButton->setDisabled(true);

    settingsButton = new QToolButton();
    settingsButton->setStyleSheet(qToolButtonStyleSheet);
    settingsButton->setIconSize(QSize(24, 24));

    serverButton = new QToolButton();
    serverButton->setStyleSheet(qToolButtonStyleSheet);
    serverButton->setIconSize(QSize(17, 24));

    helpButton = new QToolButton();
    helpButton->setStyleSheet(qToolButtonStyleSheet);
    helpButton->setIconSize(QSize(14, 25));

    toggleComicsViewButton = new QToolButton;
    toggleComicsViewButton->setStyleSheet(qToolButtonStyleSheet);
    toggleComicsViewButton->setIconSize(QSize(24, 24));

    fullscreenButton = new QToolButton();
    fullscreenButton->setStyleSheet(qToolButtonStyleSheet);
    fullscreenButton->setIconSize(QSize(24, 24));

    mainLayout->setMargin(0);
    mainLayout->setSpacing(0);

    mainLayout->addSpacing(12);
    mainLayout->addWidget(backButton, 0, Qt::AlignVCenter);
    addDivider();
    mainLayout->addWidget(forwardButton, 0, Qt::AlignVCenter);

    mainLayout->addSpacing(34);
    mainLayout->addWidget(settingsButton, 0, Qt::AlignVCenter);
    addWideDivider();
    mainLayout->addWidget(serverButton, 0, Qt::AlignVCenter);
    addWideDivider();
    mainLayout->addWidget(helpButton, 0, Qt::AlignVCenter);

    mainLayout->addStretch();

    mainLayout->addWidget(toggleComicsViewButton, 0, Qt::AlignVCenter);

    if (!fullscreenButton->actions().isEmpty()) {
        addWideDivider();
        mainLayout->addWidget(fullscreenButton, 0, Qt::AlignVCenter);
    }

    setLayout(mainLayout);

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

QSize YACReaderMainToolBar::sizeHint() const
{
    return QSize(200, 40);
}

void YACReaderMainToolBar::setSearchWidget(QWidget *w)
{
    addWideDivider();
    mainLayout->addWidget(w, 0, Qt::AlignVCenter);
}

void YACReaderMainToolBar::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.fillRect(0, 0, width(), height(), QColor("#F0F0F0"));
}

void YACReaderMainToolBar::resizeEvent(QResizeEvent *event)
{
    //210px x 2 = 420px
    int freeWidth = event->size().width() - 420;
    int maxLabelWidth = freeWidth >= 0 ? freeWidth : 0;
    currentFolder->setMaximumWidth(maxLabelWidth);
    currentFolder->adjustSize();

    QFontMetrics metrix(currentFolder->font());
    QString clippedText = metrix.elidedText(currentFolderName, Qt::ElideRight, maxLabelWidth);

    currentFolder->setText(clippedText);
    currentFolder->adjustSize();
    currentFolder->move((event->size().width() - currentFolder->width()) / 2, (event->size().height() - currentFolder->height()) / 2);
}

void YACReaderMainToolBar::addDivider()
{
    QPixmap img(":/images/main_toolbar/divider.png");
    QLabel *divider = new QLabel();
    divider->setPixmap(img);

    mainLayout->addSpacing(5);
    mainLayout->addWidget(divider, 0, Qt::AlignVCenter);
    mainLayout->addSpacing(5);
}

void YACReaderMainToolBar::addWideDivider()
{
    mainLayout->addSpacing(3);
    addDivider();
    mainLayout->addSpacing(3);
}

void YACReaderMainToolBar::setCurrentFolderName(const QString &name)
{
    currentFolder->setText(name);
    currentFolderName = name;
    currentFolder->adjustSize();

    int freeWidth = size().width() - 420;
    int maxLabelWidth = freeWidth >= 0 ? freeWidth : 0;
    currentFolder->setMaximumWidth(maxLabelWidth);

    QFontMetrics metrix(currentFolder->font());
    QString clippedText = metrix.elidedText(currentFolderName, Qt::ElideRight, maxLabelWidth);

    currentFolder->setText(clippedText);
    currentFolder->adjustSize();
    currentFolder->move((width() - currentFolder->width()) / 2, (height() - currentFolder->height()) / 2);
}
