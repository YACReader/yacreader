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

    mainLayout->setContentsMargins(0, 0, 0, 0);
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
#ifndef Q_OS_MACOS
    addWideDivider();
    mainLayout->addWidget(fullscreenButton, 0, Qt::AlignVCenter);
#endif

    setLayout(mainLayout);

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    initTheme(this);
}

QSize YACReaderMainToolBar::sizeHint() const
{
    return QSize(200, 40);
}

void YACReaderMainToolBar::setSearchWidget(QWidget *w)
{
    addWideDivider();
    mainLayout->addWidget(w, 0, Qt::AlignVCenter);
    mainLayout->addSpacing(6);
}

void YACReaderMainToolBar::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.fillRect(0, 0, width(), height(), theme.mainToolbar.backgroundColor);
}

void YACReaderMainToolBar::resizeEvent(QResizeEvent *event)
{
    // 210px x 2 = 420px
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
    QLabel *divider = new QLabel();
    divider->setPixmap(theme.mainToolbar.dividerPixmap);
    dividers.append(divider);

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

void YACReaderMainToolBar::applyTheme(const Theme &theme)
{
    const auto &mt = theme.mainToolbar;

    currentFolder->setStyleSheet(mt.folderNameLabelQSS);

    // Update dividers
    for (QLabel *divider : dividers) {
        divider->setPixmap(mt.dividerPixmap);
    }

    // Update action icons via the button's default action
    // (buttons get their icons from their default actions when setDefaultAction is called)
    if (auto action = backButton->defaultAction())
        action->setIcon(mt.backIcon);
    if (auto action = forwardButton->defaultAction())
        action->setIcon(mt.forwardIcon);
    if (auto action = settingsButton->defaultAction())
        action->setIcon(mt.settingsIcon);
    if (auto action = serverButton->defaultAction())
        action->setIcon(mt.serverIcon);
    if (auto action = helpButton->defaultAction())
        action->setIcon(mt.helpIcon);
    if (auto action = fullscreenButton->defaultAction())
        action->setIcon(mt.fullscreenIcon);

    // toggleComicsViewButton icon is handled by YACReaderContentViewsManager::applyTheme()

    update(); // Repaint with new background color
}
