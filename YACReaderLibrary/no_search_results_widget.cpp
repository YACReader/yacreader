#include "no_search_results_widget.h"

#include "theme.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QPainter>

NoSearchResultsWidget::NoSearchResultsWidget(QWidget *parent)
    : QWidget(parent)
{
    auto layout = new QVBoxLayout;

    iconLabel = new QLabel();
    iconLabel->setPixmap(QPixmap(":/images/empty_search.png"));
    iconLabel->setAlignment(Qt::AlignCenter);

    titleLabel = new QLabel("No results");
    titleLabel->setAlignment(Qt::AlignCenter);

    auto theme = Theme::currentTheme();

    backgroundColor = theme.noComicsContentBackgroundColor;
    titleLabel->setStyleSheet(theme.noComicsContentTitleLabelStyle);

    layout->addSpacing(100);
    layout->addWidget(iconLabel);
    layout->addSpacing(30);
    layout->addWidget(titleLabel);
    layout->addStretch();
    layout->setMargin(0);
    layout->setSpacing(0);

    setContentsMargins(0, 0, 0, 0);

    setStyleSheet(QString("QWidget {background:%1}").arg(backgroundColor));

    setSizePolicy(QSizePolicy ::Expanding, QSizePolicy ::Expanding);
    setLayout(layout);
}

void NoSearchResultsWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.fillRect(0, 0, width(), height(), QColor(backgroundColor));
}
