#include "view_selector.h"

#include "library_header.h"

#include <QAction>
#include <QHBoxLayout>
#include <QPainter>
#include <QToolButton>

namespace {
constexpr int Padding = 2;
// Grid, flow and info icons are drawn on a 16 unit grid.
constexpr int IconSize = 16;
constexpr int Radius = 6;
}

ViewSelector::ViewSelector(QAction *grid, QAction *flow, QAction *info, QWidget *parent)
    : QWidget(parent)
{
    setAccessibleName(tr("View"));
    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(Padding, Padding, Padding, Padding);
    layout->setSpacing(0);
    for (QAction *action : { grid, flow, info }) {
        auto *button = new QToolButton(this);
        button->setStyle(headerButtonStyle());
        button->setDefaultAction(action);
        button->setAutoRaise(true);
        button->setFocusPolicy(Qt::TabFocus);
        button->setIconSize(QSize(IconSize, IconSize));
        button->setFixedSize(IconSize + 14, IconSize + 8);
        button->setAccessibleName(action->text());
        layout->addWidget(button);
        buttons.append(button);
    }
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

void ViewSelector::applyTheme(const HeaderColors &colors)
{
    background = colors.selectorBackground;
    const auto rgba = [](const QColor &c) { return c.name(QColor::HexArgb); };
    const QString qss = QStringLiteral(
                                "QToolButton { border: none; border-radius: %1px; background: transparent; }"
                                "QToolButton:hover { background: %2; }"
                                "QToolButton:pressed { background: %3; }"
                                "QToolButton:checked { background: %4; }"
                                "QToolButton:focus { border: 1px solid %5; }")
                                .arg(Radius - Padding)
                                .arg(rgba(colors.selectorHover), rgba(colors.selectorPressed), rgba(colors.selectorChecked), rgba(colors.icon));
    for (QToolButton *button : std::as_const(buttons))
        button->setStyleSheet(qss);
    update();
}

void ViewSelector::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(background);
    painter.drawRoundedRect(rect(), Radius, Radius);
}
