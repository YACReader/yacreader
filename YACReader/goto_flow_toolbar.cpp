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
    slider->setStyleSheet(
            "QSlider::groove:horizontal {"
            "  border: 1px solid #22FFFFFF;"
            "  border-radius: 1px;"
            "  background: #77000000;"
            "  margin: 2px 0;"
            "  padding: 1px;"
            "}"
            "QSlider::handle:horizontal {"
            "  background: #55FFFFFF;"
            "  width: 48px;"
            "  border-radius: 1px;"
            "}");

    connect(slider, &QSlider::valueChanged, this, [&](int v) { emit setCenter(v); });
    connect(slider, &QSlider::valueChanged, this, [=](int v) { emit goToPage(v); });

    pageHint = new QLabel("<b>" + tr("Page : ") + "</b>", this);
    v = new QIntValidator(this);
    v->setBottom(1);
    edit = new QLineEdit(this);
    edit->setValidator(v);
    edit->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    edit->setStyleSheet("QLineEdit {border: 1px solid #77000000; background: #55000000; color: white; padding: 3px 5px 5px 5px; margin: 13px 5px 12px 5px; font-weight:bold}");
    edit->setFixedSize(54, 50);
    edit->setAttribute(Qt::WA_MacShowFocusRect, false);
    // edit->setAttribute(Qt::WA_LayoutUsesWidgetRect,true);
    // edit->resize(QSize(54,50));
    edit->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    // edit->setAutoFillBackground(false);
    connect(edit, &QLineEdit::returnPressed, this, &GoToFlowToolBar::goTo);

    QString centerButtonCSS = "QPushButton {background-image: url(:/images/imgCenterSlide.png); width: 100%; height:100%; background-repeat: none; border: none;} "
                              "QPushButton:focus { border: none; outline: none;}"
                              "QPushButton:pressed  {background-image: url(:/images/imgCenterSlidePressed.png); width: 100%; height:100%; background-repeat: none; border: none;} ";
    centerButton = new QPushButton(this);
    // centerButton->setIcon(QIcon(":/images/center.png"));
    centerButton->setStyleSheet(centerButtonCSS);
    centerButton->setFixedSize(26, 50);
    centerButton->setAttribute(Qt::WA_LayoutUsesWidgetRect, true);
    connect(centerButton, &QAbstractButton::clicked, this, &GoToFlowToolBar::centerSlide);

    QString goToButtonCSS = "QPushButton {background-image: url(:/images/imgGoToSlide.png); width: 100%; height:100%; background-repeat: none; border: none;} "
                            "QPushButton:focus { border: none; outline: none;}"
                            "QPushButton:pressed  {background-image: url(:/images/imgGoToSlidePressed.png); width: 100%; height:100%; background-repeat: none; border: none;} ";
    goToButton = new QPushButton(this);
    // goToButton->setIcon(QIcon(":/images/goto.png"));
    goToButton->setStyleSheet(goToButtonCSS);
    goToButton->setFixedSize(32, 50);
    goToButton->setAttribute(Qt::WA_LayoutUsesWidgetRect, true);

    connect(goToButton, &QPushButton::clicked, this, &GoToFlowToolBar::goTo);

    normalLayout->setMargin(0);
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
    naviLayout->addWidget(goToButton);

    updateOptions();

    setFixedHeight(50);
}

void GoToFlowToolBar::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.fillRect(0, 0, width(), height(), QColor(0x99000000));
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
