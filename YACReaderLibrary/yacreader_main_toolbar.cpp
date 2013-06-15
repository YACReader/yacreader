#include "yacreader_main_toolbar.h"

#include <QPainter>
#include <QHBoxLayout>
#include <QToolButton>
#include <QLabel>
#include <QResizeEvent>
#include <QPaintEvent>

YACReaderMainToolBar::YACReaderMainToolBar(QWidget *parent) :
    QWidget(parent)
{
    mainLayout = new QHBoxLayout;

    currentFolder = new QLabel(tr("Root"),this);
	currentFolder->setAlignment(Qt::AlignCenter);
	currentFolder->setStyleSheet(" QLabel {color:#404040; font-size:22px; font-weight:bold;}");

	QFont f=currentFolder->font();
	f.setStyleStrategy(QFont::PreferAntialias);
	currentFolder->setFont(f);

	QString qToolButtonStyleSheet = "QToolButton {border:none;}";

    backButton = new QToolButton();
    backButton->setStyleSheet(qToolButtonStyleSheet);
	QIcon icoBackButton;
    icoBackButton.addPixmap(QPixmap(":/images/main_toolbar/back.png"), QIcon::Normal);
    icoBackButton.addPixmap(QPixmap(":/images/main_toolbar/back_disabled.png"), QIcon::Disabled);
	backButton->setIcon(icoBackButton);

    forwardButton = new QToolButton();
    forwardButton->setStyleSheet(qToolButtonStyleSheet);
	forwardButton->setDisabled(true);
	QIcon icoFordwardButton;
    icoFordwardButton.addPixmap(QPixmap(":/images/main_toolbar/forward.png"), QIcon::Normal);
    icoFordwardButton.addPixmap(QPixmap(":/images/main_toolbar/forward_disabled.png"), QIcon::Disabled);
	forwardButton->setIcon(icoFordwardButton);

    settingsButton = new QToolButton();
    settingsButton->setStyleSheet(qToolButtonStyleSheet);
    QIcon icoSettingsButton;
	settingsButton->setIconSize(QSize(24,24));
    icoSettingsButton.addPixmap(QPixmap(":/images/main_toolbar/settings.png"), QIcon::Normal);
    settingsButton->setIcon(icoSettingsButton);

    serverButton = new QToolButton();
    serverButton->setStyleSheet(qToolButtonStyleSheet);
    QIcon icoServerButton;
    serverButton->setIconSize(QSize(17,24));
    icoServerButton.addPixmap(QPixmap(":/images/main_toolbar/server.png"), QIcon::Normal);
    serverButton->setIcon(icoServerButton);

    helpButton = new QToolButton();
    helpButton->setStyleSheet(qToolButtonStyleSheet);
    QIcon icoHelpButton;
    helpButton->setIconSize(QSize(14,25));
    icoHelpButton.addPixmap(QPixmap(":/images/main_toolbar/help.png"), QIcon::Normal);
    helpButton->setIcon(icoHelpButton);

    fullscreenButton = new QToolButton();
    fullscreenButton->setStyleSheet(qToolButtonStyleSheet);
    QIcon icoFullscreenButton;
    fullscreenButton->setIconSize(QSize(24,24));
    icoFullscreenButton.addPixmap(QPixmap(":/images/main_toolbar/fullscreen.png"), QIcon::Normal);
    fullscreenButton->setIcon(icoFullscreenButton);

    mainLayout->setMargin(0);
    mainLayout->setSpacing(0);

    mainLayout->addSpacing(12);
    mainLayout->addWidget(backButton);
    addDivider();
    mainLayout->addWidget(forwardButton);

    mainLayout->addSpacing(34);
    mainLayout->addWidget(settingsButton);
    addWideDivider();
    mainLayout->addWidget(serverButton);
    addWideDivider();
    mainLayout->addWidget(helpButton);

    mainLayout->addStretch();

    mainLayout->addWidget(fullscreenButton);
    mainLayout->addSpacing(10);

    setLayout(mainLayout);

    setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
}


QSize YACReaderMainToolBar::sizeHint() const
{
    return QSize(200,40);
}

void YACReaderMainToolBar::paintEvent(QPaintEvent * event)
{
    Q_UNUSED(event);

    QPainter painter (this);
    painter.fillRect(0,0,width(),height(),QColor("#F0F0F0"));
}

void YACReaderMainToolBar::resizeEvent(QResizeEvent * event)
{
    currentFolder->move((event->size().width()-currentFolder->width())/2,(event->size().height()-currentFolder->height())/2);
}

void YACReaderMainToolBar::addDivider()
{
    QPixmap img(":/images/main_toolbar/divider.png");
    QLabel * divider = new QLabel();
    divider->setPixmap(img);

    mainLayout->addSpacing(5);
    mainLayout->addWidget(divider);
    mainLayout->addSpacing(5);
}

void YACReaderMainToolBar::addWideDivider()
{
    mainLayout->addSpacing(3);
    addDivider();
    mainLayout->addSpacing(3);
}
