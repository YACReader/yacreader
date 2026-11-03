#include "yacreader_main_toolbar.h"

#include <QAction>
#include <QHBoxLayout>
#include <QLabel>
#include <QPaintEvent>
#include <QPainter>
#include <QResizeEvent>
#include <QToolButton>

#include <utility>

YACReaderMainToolBar::YACReaderMainToolBar(QWidget *parent)
    : QWidget(parent)
{
    mainLayout = new QHBoxLayout;

    currentFolder = new QLabel(this);

    QFont f = currentFolder->font();
    f.setStyleStrategy(QFont::PreferAntialias);
    currentFolder->setFont(f);

    // every button has the same box and every icon the same 24px canvas, so spacing reads the same on all platforms
    backButton = addButton();
    forwardButton = addButton();
    forwardButton->setDisabled(true);
    settingsButton = addButton();
    serverButton = addButton();
    helpButton = addButton();
    toggleComicsViewButton = addButton();
    fullscreenButton = addButton();

    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    mainLayout->addSpacing(6);
    mainLayout->addWidget(backButton, 0, Qt::AlignVCenter);
    addDivider(2);
    mainLayout->addWidget(forwardButton, 0, Qt::AlignVCenter);

    mainLayout->addSpacing(26);
    mainLayout->addWidget(settingsButton, 0, Qt::AlignVCenter);
    addDivider();
    mainLayout->addWidget(serverButton, 0, Qt::AlignVCenter);
    addDivider();
    mainLayout->addWidget(helpButton, 0, Qt::AlignVCenter);

    mainLayout->addStretch();

    mainLayout->addWidget(toggleComicsViewButton, 0, Qt::AlignVCenter);
#ifndef Q_OS_MACOS
    addDivider();
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
    addDivider();
    mainLayout->addWidget(w, 0, Qt::AlignVCenter);
    mainLayout->addSpacing(3);
}

void YACReaderMainToolBar::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.fillRect(0, 0, width(), height(), theme.mainToolbar.backgroundColor);
}

void YACReaderMainToolBar::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateCurrentFolderLabel();
}

QToolButton *YACReaderMainToolBar::addButton()
{
    auto button = new QToolButton();
    button->setStyleSheet("QToolButton {border:none;}");
    button->setIconSize(QSize(24, 24));
    button->setFixedSize(28, 28);
    return button;
}

void YACReaderMainToolBar::addDivider(int spacing)
{
    QLabel *divider = new QLabel();
    divider->setPixmap(theme.mainToolbar.dividerPixmap);
    dividers.append(divider);

    mainLayout->addSpacing(spacing);
    mainLayout->addWidget(divider, 0, Qt::AlignVCenter);
    mainLayout->addSpacing(spacing);
}

void YACReaderMainToolBar::setCurrentFolderName(const QString &name)
{
    currentFolderName = name;
    updateCurrentFolderLabel();
}

void YACReaderMainToolBar::updateCurrentFolderLabel()
{
    // the title stays centered on the toolbar, so it can use twice the distance to the nearest button group
    const int margin = 16;
    const int center = width() / 2;
    const int leftEdge = helpButton->geometry().right();
    const int rightEdge = toggleComicsViewButton->geometry().left();
    const int maxLabelWidth = qMax(0, 2 * (qMin(center - leftEdge, rightEdge - center) - margin));

    QFontMetrics metrix(currentFolder->font());
    currentFolder->setMaximumWidth(maxLabelWidth);
    currentFolder->setText(metrix.elidedText(currentFolderName, Qt::ElideRight, maxLabelWidth));
    currentFolder->adjustSize();
    currentFolder->move((width() - currentFolder->width()) / 2, (height() - currentFolder->height()) / 2);
}

void YACReaderMainToolBar::applyTheme(const Theme &theme)
{
    const auto &mt = theme.mainToolbar;

    currentFolder->setStyleSheet(mt.folderNameLabelQSS);

    // Update dividers
    for (QLabel *divider : std::as_const(dividers)) {
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
