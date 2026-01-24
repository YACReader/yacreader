#include "goto_flow_toolbar.h"

#include <QtWidgets>

#include "configuration.h"

GoToFlowToolBar::GoToFlowToolBar(QWidget *parent)
    : QStackedWidget(parent)
{
    // elementos interactivos
    auto normal = new QWidget(this); // container widget
    auto quickNavi = new QWidget(this); // container widget
    addWidget(normal);
    addWidget(quickNavi);
    auto normalLayout = new QHBoxLayout(normal);
    auto naviLayout = new QHBoxLayout(quickNavi);
    normal->setLayout(normalLayout);
    quickNavi->setLayout(naviLayout);

    slider = new QSlider(Qt::Horizontal, this);

    connect(slider, &QSlider::valueChanged, this, &GoToFlowToolBar::setCenter);
    connect(slider, &QSlider::valueChanged, this, &GoToFlowToolBar::setPage);

    pageHint = new QLabel("<b>" + tr("Page : ") + "</b>", this);
    v = new QIntValidator();
    v->setBottom(1);
    edit = new QLineEdit();
    edit->setValidator(v);
    edit->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    edit->setFixedSize(54, 50);
    edit->setAttribute(Qt::WA_MacShowFocusRect, false);
    edit->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    connect(edit, &QLineEdit::returnPressed, this, &GoToFlowToolBar::goTo);

    centerButton = new QPushButton();
    centerButton->setIconSize(QSize(12, 12));
    centerButton->setFixedSize(26, 50);
    centerButton->setAttribute(Qt::WA_LayoutUsesWidgetRect, true);
    connect(centerButton, &QAbstractButton::clicked, this, &GoToFlowToolBar::centerSlide);

    goToButton = new QPushButton();
    goToButton->setIconSize(QSize(12, 12));
    goToButton->setFixedSize(32, 50);
    goToButton->setAttribute(Qt::WA_LayoutUsesWidgetRect, true);
    connect(goToButton, &QPushButton::clicked, this, &GoToFlowToolBar::goTo);

    goToButton2 = new QPushButton();
    goToButton2->setIconSize(QSize(12, 12));
    goToButton2->setFixedSize(32, 50);
    goToButton2->setAttribute(Qt::WA_LayoutUsesWidgetRect, true);
    connect(goToButton2, &QPushButton::clicked, this, &GoToFlowToolBar::goTo);

    normalLayout->setContentsMargins(0, 0, 0, 0);
    normalLayout->setSpacing(0);
    normalLayout->addStretch();
    normalLayout->addWidget(pageHint);
    normalLayout->addWidget(edit);
    normalLayout->addWidget(centerButton);
    normalLayout->addWidget(goToButton);
    normalLayout->addStretch();

    naviLayout->setContentsMargins(5, 0, 0, 0);
    naviLayout->setSpacing(2);
    naviLayout->addWidget(slider);
    naviLayout->addWidget(goToButton2);

    this->addWidget(normal);
    this->addWidget(quickNavi);

    updateOptions();

    setFixedHeight(50);

    initTheme(this);
}

void GoToFlowToolBar::applyTheme(const Theme &theme)
{
    auto goToFlowTheme = theme.goToFlowWidget;

    slider->setStyleSheet(goToFlowTheme.sliderQSS);
    edit->setStyleSheet(goToFlowTheme.editQSS);
    pageHint->setStyleSheet(goToFlowTheme.labelQSS);

    centerButton->setStyleSheet(goToFlowTheme.buttonQSS);
    centerButton->setIcon(goToFlowTheme.centerIcon);

    goToButton->setStyleSheet(goToFlowTheme.buttonQSS);
    goToButton->setIcon(goToFlowTheme.goToIcon);

    goToButton2->setStyleSheet(goToFlowTheme.buttonQSS);
    goToButton2->setIcon(goToFlowTheme.goToIcon);

    update();
}

void GoToFlowToolBar::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.fillRect(0, 0, width(), height(), theme.goToFlowWidget.toolbarBackgroundColor);
}

void GoToFlowToolBar::setPage(int pageNumber)
{
    edit->setText(QString::number(pageNumber + 1));
    slider->setValue(pageNumber);
}

void GoToFlowToolBar::setTop(int numPages)
{
    v->setTop(numPages);
    slider->setMaximum(numPages - 1); // min is 0
}

void GoToFlowToolBar::goTo()
{
    unsigned int page = edit->text().toInt();
    if (page >= 1 && page <= v->top()) {
        emit goToPage(page - 1);
    }
}

void GoToFlowToolBar::centerSlide()
{
    if (edit->text().toInt() != 0)
        emit setCenter(edit->text().toInt() - 1);
}

void GoToFlowToolBar::updateOptions()
{
    if (Configuration::getConfiguration().getQuickNaviMode())
        setCurrentIndex(1);
    else
        setCurrentIndex(0);

    slider->setInvertedAppearance(Configuration::getConfiguration().getDoubleMangaPage());
}
