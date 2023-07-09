#include "no_search_results_widget.h"

#include "yacreader_global.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QPainter>

NoSearchResultsWidget::NoSearchResultsWidget(QWidget *parent)
    : QWidget(parent)
{
#ifdef Y_MAC_UI
    backgroundColor = "#FFFFFF";
#else
    backgroundColor = "#2A2A2A";
#endif

    auto layout = new QVBoxLayout;

    iconLabel = new QLabel();
    iconLabel->setPixmap(QPixmap(":/images/empty_search.png"));
    iconLabel->setAlignment(Qt::AlignCenter);

    titleLabel = new QLabel("No results");
    titleLabel->setAlignment(Qt::AlignCenter);

#ifdef Y_MAC_UI
    titleLabel->setStyleSheet("QLabel {color:#888888; font-size:24px;font-family:Arial;font-weight:bold;}");
#else
    titleLabel->setStyleSheet("QLabel {color:#CCCCCC; font-size:24px;font-family:Arial;font-weight:bold;}");
#endif

    layout->addSpacing(100);
    layout->addWidget(iconLabel);
    layout->addSpacing(30);
    layout->addWidget(titleLabel);
    layout->addStretch();
    layout->setContentsMargins(0, 0, 0, 0);
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
