#include "yacreader_library_item_widget.h"

#include "theme.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QToolButton>
#include <QMouseEvent>

YACReaderLibraryItemWidget::YACReaderLibraryItemWidget(QString n /*ame*/, QString p /*ath*/, QWidget *parent)
    : QWidget(parent), name(n), path(p), isSelected(false)
{
    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->setMargin(0);
    mainLayout->setSpacing(0);

    QPixmap iconPixmap(":/images/sidebar/libraryIcon.png");
    icon = new QLabel(this);
    icon->setPixmap(iconPixmap);

    nameLabel = new QLabel(name, this);

    options = new QToolButton(this);

    options->setIcon(QIcon(QPixmap(":/images/sidebar/libraryOptions.png")));

    options->setHidden(true);

    options->setFixedWidth(18);
    options->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
    options->setStyleSheet("QToolButton {border:none;}");

    connect(options, SIGNAL(clicked()), this, SIGNAL(showOptions()));

    mainLayout->addWidget(icon);
    mainLayout->addWidget(nameLabel, Qt::AlignLeft);
    mainLayout->addStretch();
    mainLayout->addWidget(options);

    setLayout(mainLayout);

    setStyleSheet(Theme::currentTheme().itemLibraryNoSelectedStyleSheet);

    QString iconStyleSheet = "QLabel {padding:0 0 0 24px; margin:0px}";
    icon->setStyleSheet(iconStyleSheet);

    QString nameLabelStyleSheet = "QLabel {padding:0 0 0 3px; margin:0px;}";
    nameLabel->setStyleSheet(nameLabelStyleSheet);

    setMinimumHeight(20);
}

void YACReaderLibraryItemWidget::showUpDownButtons(bool show)
{
    up->setHidden(!show);
    down->setHidden(!show);
}

void YACReaderLibraryItemWidget::deselect()
{
    setStyleSheet(Theme::currentTheme().itemLibraryNoSelectedStyleSheet);

    QPixmap iconPixmap(":/images/sidebar/libraryIcon.png");
    icon->setPixmap(iconPixmap);

    options->setHidden(true);

    isSelected = false;
}

void YACReaderLibraryItemWidget::select()
{
    setStyleSheet(Theme::currentTheme().itemLibrarySelectedStyleSheet);

    options->setHidden(false);

    QPixmap iconPixmap(":/images/sidebar/libraryIconSelected.png");
    icon->setPixmap(iconPixmap);

    isSelected = true;
}

void YACReaderLibraryItemWidget::setName(const QString &name)
{
    this->name = name;
    nameLabel->setText(name);
}
