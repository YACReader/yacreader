#include "library_header.h"

#include "header_model.h"
#include "view_selector.h"
#include "yacreader_search_line_edit.h"

#include <QAction>
#include <QApplication>
#include <QLabel>
#include <QPainter>
#include <QProxyStyle>
#include <QToolButton>

namespace {
constexpr int SearchMinWidth = 160;
constexpr int SearchMaxWidth = 300;
constexpr int Gap = 16;
constexpr int AppIconSize = 20;

class HeaderButtonStyle : public QProxyStyle
{
public:
    int pixelMetric(PixelMetric metric, const QStyleOption *option, const QWidget *widget) const override
    {
        if (metric == PM_ButtonShiftHorizontal || metric == PM_ButtonShiftVertical)
            return 0;
        return QProxyStyle::pixelMetric(metric, option, widget);
    }
};

QToolButton *makeButton(QAction *action, QWidget *parent)
{
    auto *button = new QToolButton(parent);
    button->setStyle(headerButtonStyle());
    button->setDefaultAction(action);
    button->setAutoRaise(true);
    button->setFocusPolicy(Qt::TabFocus);
    button->setIconSize(QSize(LibraryHeader::IconSize, LibraryHeader::IconSize));
    button->setFixedSize(LibraryHeader::ButtonSize, LibraryHeader::ButtonSize);
    button->setAccessibleName(action->toolTip());
    return button;
}

QString buttonQss(const QColor &hover, const QColor &pressed)
{
    return QStringLiteral("QToolButton { border: none; border-radius: 6px; background: transparent; }"
                          "QToolButton:hover { background: %1; }"
                          "QToolButton:pressed { background: %2; }")
            .arg(hover.name(QColor::HexArgb), pressed.name(QColor::HexArgb));
}

QColor withAlpha(QColor color, int alpha)
{
    color.setAlpha(alpha);
    return color;
}

int centeredY(int height, int itemHeight)
{
    return (height - itemHeight) / 2;
}

QString appName()
{
    return QStringLiteral("YACReader Library");
}
} // namespace

QStyle *headerButtonStyle()
{
    static auto *style = new HeaderButtonStyle;
    return style;
}

HeaderColors headerColors(const Theme &theme)
{
    const auto &mt = theme.mainToolbar;
    HeaderColors c;
    c.sidebarBackground = theme.sidebar.backgroundColor;
    c.sidebarText = theme.sidebar.titleTextColor;
    c.sidebarButtonHover = withAlpha(mt.sidebarIconColor, 0x1A);
    c.sidebarButtonPressed = withAlpha(mt.sidebarIconColor, 0x10);
    c.toolbarBackground = mt.backgroundColor;
    c.folderName = mt.folderNameColor;
    c.icon = mt.iconColor;
    // Overlays of the toolbar foreground, so every theme gets matching states.
    c.buttonHover = withAlpha(mt.iconColor, 0x1A);
    c.buttonPressed = withAlpha(mt.iconColor, 0x10);
    c.selectorBackground = withAlpha(mt.iconColor, 0x14);
    c.selectorHover = withAlpha(mt.iconColor, 0x1A);
    c.selectorPressed = withAlpha(mt.iconColor, 0x10);
    c.selectorChecked = withAlpha(mt.iconColor, 0x30);
    return c;
}

// SidebarHeader ---------------------------------------------------------------

SidebarHeader::SidebarHeader(HeaderModel *model, QWidget *parent)
    : QWidget(parent)
{
#ifndef Q_OS_MACOS
    appIcon = new QLabel(this);
    appName = new QLabel(::appName(), this);
    QFont font = appName->font();
    font.setWeight(QFont::DemiBold);
    appName->setFont(font);
#endif
    const auto &actions = model->actions();
    for (QAction *action : { actions.settings, actions.server, actions.help }) {
        if (action != nullptr)
            buttons.append(makeButton(action, this));
    }
}

void SidebarHeader::applyTheme(const HeaderColors &colors, const QIcon &icon)
{
    background = colors.sidebarBackground;
    if (appIcon != nullptr) {
        appIcon->setPixmap(icon.pixmap(QSize(AppIconSize, AppIconSize), devicePixelRatioF()));
        appName->setStyleSheet(QStringLiteral("color: %1;").arg(colors.sidebarText.name()));
    }
    for (QToolButton *button : std::as_const(buttons))
        button->setStyleSheet(buttonQss(colors.sidebarButtonHover, colors.sidebarButtonPressed));
    update();
}

void SidebarHeader::setAppIdentityVisible(bool visible)
{
    if (appIcon == nullptr)
        return;
    appIcon->setVisible(visible);
    appName->setVisible(visible);
}

void SidebarHeader::paintEvent(QPaintEvent *)
{
    QPainter(this).fillRect(rect(), background);
}

void SidebarHeader::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    relayout();
}

void SidebarHeader::relayout()
{
    int right = width() - 8;
    for (auto it = buttons.rbegin(); it != buttons.rend(); ++it) {
        right -= LibraryHeader::ButtonSize;
        (*it)->move(right, centeredY(height(), LibraryHeader::ButtonSize));
        right -= 2;
    }
    if (appIcon == nullptr)
        return;
    const int left = 12;
    appIcon->setGeometry(left, centeredY(height(), AppIconSize), AppIconSize, AppIconSize);
    const int nameLeft = left + AppIconSize + 8;
    const int nameWidth = qMax(0, right - 8 - nameLeft);
    appName->setGeometry(nameLeft, 0, nameWidth, height());
    appName->setText(appName->fontMetrics().elidedText(::appName(), Qt::ElideRight, nameWidth));
}

// ContentHeader ---------------------------------------------------------------

ContentHeader::ContentHeader(HeaderModel *model, QWidget *parent)
    : QWidget(parent)
{
    const auto &actions = model->actions();
    back = makeButton(actions.back, this);
    forward = makeButton(actions.forward, this);
    if (actions.fullscreen != nullptr)
        fullscreen = makeButton(actions.fullscreen, this);

    folderName = new QLabel(this);
    QFont font = folderName->font();
    font.setPointSizeF(font.pointSizeF() * 1.15);
    font.setBold(true);
    folderName->setFont(font);

    search = new YACReaderSearchLineEdit(this);
    search->setMaximumWidth(SearchMaxWidth);
    search->setEnabled(model->isSearchEnabled());
    search->setSearchMenu(model->searchMenu());

    selector = new ViewSelector(actions.gridView, actions.flowView, actions.infoView, this);

    connect(model, &HeaderModel::folderNameChanged, this, [this](const QString &name) {
        folderNameText = name;
        relayout();
    });
    connect(search, &YACReaderSearchLineEdit::filterChanged, model, [this, model](const QString &text) {
        if (!settingSearchText)
            model->editSearch(text);
    });
    connect(model, &HeaderModel::searchTextSet, this, [this](const QString &text) {
        settingSearchText = true;
        search->setText(text);
        settingSearchText = false;
    });
    connect(model, &HeaderModel::searchEnabledChanged, search, &QWidget::setEnabled);
    connect(model, &HeaderModel::searchMenuChanged, search, &YACReaderSearchLineEdit::setSearchMenu);
    connect(model, &HeaderModel::searchFocusRequested, search, [this] { search->setFocus(Qt::ShortcutFocusReason); });
}

void ContentHeader::applyTheme(const HeaderColors &colors)
{
    background = colors.toolbarBackground;
    folderName->setStyleSheet(QStringLiteral("color: %1;").arg(colors.folderName.name()));
    for (QToolButton *button : { back, forward, fullscreen }) {
        if (button != nullptr)
            button->setStyleSheet(buttonQss(colors.buttonHover, colors.buttonPressed));
    }
    selector->applyTheme(colors);
    update();
}

void ContentHeader::paintEvent(QPaintEvent *)
{
    QPainter(this).fillRect(rect(), background);
}

void ContentHeader::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    relayout();
}

void ContentHeader::relayout()
{
    const int h = height();
    int right = width() - 12;
    if (fullscreen != nullptr) {
        right -= LibraryHeader::ButtonSize;
        fullscreen->move(right, centeredY(h, LibraryHeader::ButtonSize));
        right -= 8;
    }
    const QSize selectorSize = selector->sizeHint();
    right -= selectorSize.width();
    selector->setGeometry(right, centeredY(h, selectorSize.height()), selectorSize.width(), selectorSize.height());
    const int rightGroupLeft = right - Gap;

    int left = 10;
    back->move(left, centeredY(h, LibraryHeader::ButtonSize));
    left += LibraryHeader::ButtonSize + 4;
    forward->move(left, centeredY(h, LibraryHeader::ButtonSize));
    left += LibraryHeader::ButtonSize + 12;
    const int titleLeft = left;

    // The search position depends only on the header width, never on the
    // folder name, so it does not move while navigating. The name uses the
    // space up to the search and is elided after that.
    const int available = qMax(0, rightGroupLeft - titleLeft);
    const int searchWidth = qBound(SearchMinWidth, available / 3, SearchMaxWidth);
    // Halfway between the center of the free space and the center of the
    // content column: a fixed offset, so both gaps grow at the same rate.
    const int center = ((titleLeft + rightGroupLeft) / 2 + width() / 2) / 2;
    const int searchLeft = qMax(titleLeft + Gap, qMin(center - searchWidth / 2, rightGroupLeft - searchWidth));
    const int searchHeight = search->height();
    search->setGeometry(searchLeft, centeredY(h, searchHeight), searchWidth, searchHeight);

    const QFontMetrics metrics = folderName->fontMetrics();
    const int fullTitleWidth = metrics.horizontalAdvance(folderNameText) + 2;
    const int titleWidth = qBound(0, searchLeft - Gap - titleLeft, fullTitleWidth);
    folderName->setText(metrics.elidedText(folderNameText, Qt::ElideRight, titleWidth));
    folderName->setGeometry(titleLeft, 0, titleWidth, h);
}

// LibraryHeader ---------------------------------------------------------------

LibraryHeader::LibraryHeader(HeaderModel *model, QObject *parent)
    : QObject(parent)
{
    sidebar = new SidebarHeader(model);
    content = new ContentHeader(model);
    for (QWidget *part : { static_cast<QWidget *>(sidebar), static_cast<QWidget *>(content) }) {
        part->setFixedHeight(Height);
        part->setEnabled(model->isEnabled());
        connect(model, &HeaderModel::enabledChanged, part, &QWidget::setEnabled);
    }
    initTheme(this);
}

void LibraryHeader::applyTheme(const Theme &theme)
{
    const HeaderColors colors = headerColors(theme);
    const QIcon appIcon = qApp->windowIcon().isNull() ? QIcon(QStringLiteral(":/icon.ico")) : qApp->windowIcon();
    sidebar->applyTheme(colors, appIcon);
    content->applyTheme(colors);
}

void LibraryHeader::setAppIdentityVisible(bool visible)
{
    sidebar->setAppIdentityVisible(visible);
}

void LibraryHeader::setVisible(bool visible)
{
    sidebar->setVisible(visible);
    content->setVisible(visible);
}
