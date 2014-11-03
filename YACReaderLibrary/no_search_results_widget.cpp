#include "no_search_results_widget.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QPainter>

NoSearchResultsWidget::NoSearchResultsWidget(QWidget *parent) :
    QWidget(parent)
{
    QVBoxLayout * layout = new QVBoxLayout;

    iconLabel = new QLabel();
    iconLabel->setPixmap(QPixmap(":/images/empty_search.png"));
    iconLabel->setAlignment(Qt::AlignCenter);

    titleLabel = new QLabel("No results");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("QLabel {color:#CCCCCC; font-size:24px;font-family:Arial;font-weight:bold;}");

    layout->addSpacing(100);
    layout->addWidget(iconLabel);
    layout->addSpacing(30);
    layout->addWidget(titleLabel);
    layout->addStretch();
    layout->setMargin(0);
    layout->setSpacing(0);

    setContentsMargins(0,0,0,0);

    setStyleSheet("QWidget {background:#2A2A2A}");

    setSizePolicy(QSizePolicy ::Expanding , QSizePolicy ::Expanding );
    setLayout(layout);
}

void NoSearchResultsWidget::paintEvent(QPaintEvent *)
{
    QPainter painter (this);
    painter.fillRect(0,0,width(),height(),QColor("#2A2A2A"));
}
