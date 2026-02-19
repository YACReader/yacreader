#include "yacreader_library_item_widget.h"

#include "yacreader_global.h"
#include "yacreader_global_gui.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QToolButton>
#include <QMouseEvent>

using namespace YACReader;

YACReaderLibraryItemWidget::YACReaderLibraryItemWidget(QString n /*ame*/, QString p /*ath*/, QWidget *parent)
    : QWidget(parent), name(n), path(p), isSelected(false)
{
    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // installEventFilter(this);

    icon = new QLabel(this);

    nameLabel = new QLabel(name, this);

    options = new QToolButton(this);

    optionsIconLabel = new QLabel(options);
    optionsIconLabel->move(4, 3);
    optionsIconLabel->setFixedSize(14, 14);

    options->setHidden(true);
    options->setIconSize(QSize(18, 18));
    options->setStyleSheet("QToolButton {border:none;}");
    connect(options, &QAbstractButton::clicked, this, &YACReaderLibraryItemWidget::showOptions);
    /*up = new QToolButton(this);
        up->setIcon(QIcon(":/images/libraryUp.png"));
        up->setHidden(true);
        up->setFixedWidth(18);
        up->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Minimum);

        down = new QToolButton(this);
        down->setIcon(QIcon(":/images/libraryDown.png"));
        down->setHidden(true);
        down->setFixedWidth(18);
        down->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Minimum);*/

    mainLayout->addWidget(icon);
    mainLayout->addWidget(nameLabel, 1, Qt::AlignLeft);
    mainLayout->addWidget(options, Qt::AlignCenter);
    /*mainLayout->addWidget(up);
        mainLayout->addWidget(down);*/

    setLayout(mainLayout);

    QString iconStyleSheet = "QLabel {padding:0 0 0 24px; margin:0px}";
    icon->setStyleSheet(iconStyleSheet);

    QString nameLabelStyleSheet = "QLabel {padding:0 0 0 3px; margin:0px;}";
    nameLabel->setStyleSheet(nameLabelStyleSheet);

    setMinimumHeight(20);
    setAttribute(Qt::WA_StyledBackground, true);

    initTheme(this);
}

void YACReaderLibraryItemWidget::applyTheme(const Theme &theme)
{
    const auto &icons = theme.sidebarIcons;
    const auto &li = theme.libraryItem;

    // Update icon based on current selection state
    QIcon iconToUse = isSelected ? li.libraryIconSelected : icons.libraryIcon;
    icon->setPixmap(iconToUse.pixmap(16, 16));

    // Update options icon (uses libraryItem theme since it's only shown when selected)
    QPixmap optionsPixmap = li.libraryOptionsIcon.pixmap(8, 8);
    optionsPixmap.setDevicePixelRatio(devicePixelRatioF());
    optionsIconLabel->setPixmap(optionsPixmap);

    // Update widget styling based on selection state
    if (isSelected) {
        QString styleSheet = QString("color: %1; background-color: %2; font-weight:bold;")
                                     .arg(li.selectedTextColor.name())
                                     .arg(li.selectedBackgroundColor.name());
        setStyleSheet(styleSheet);
    } else {
        QString styleSheet = QString("background-color:transparent; color: %1;")
                                     .arg(li.textColor.name());
        setStyleSheet(styleSheet);
    }
}

void YACReaderLibraryItemWidget::showUpDownButtons(bool show)
{
    up->setHidden(!show);
    down->setHidden(!show);
}

/*
bool YACReaderLibraryItemWidget::eventFilter(QObject *object, QEvent *event){
        if(!isSelected && object==this && (event->type()==QEvent::Enter))
        {
                QString styleSheet = "background-color:#5E5E5E; border-top: 1px solid #5E5E5E;border-bottom: 1px solid #5E5E5E; ";
                setStyleSheet(styleSheet);

                up->setHidden(false);
                down->setHidden(false);
                options->setHidden(false);

                return true;
        }
        if(!isSelected && object==this && (event->type()==QEvent::Leave))
        {
                QString styleSheet = "background-color:#454545; border-top: 1px solid #454545;border-bottom: 1px solid #454545;";
                setStyleSheet(styleSheet);

                up->setHidden(true);
                down->setHidden(true);
                options->setHidden(true);

                return true;
        }

        if(object==this && (event->type()==QEvent::MouseButtonRelease))
        {
                QString styleSheet = "background-color:#2E2E2E; border-top: 1px solid #1F1F1F;border-bottom: 1px solid #636363; padding-top:1px; padding-bottom:1px;";
                setStyleSheet(styleSheet);
                emit(selected(name,path));
                isSelected = true;
                return true;
        }

        return false;
}*/

void YACReaderLibraryItemWidget::deselect()
{
    QString styleSheet = QString("background-color:transparent; color: %1;")
                                 .arg(theme.libraryItem.textColor.name());
    setStyleSheet(styleSheet);

    icon->setPixmap(theme.sidebarIcons.libraryIcon.pixmap(16, 16));

    /*up->setHidden(true);
        down->setHidden(true);*/
    options->setHidden(true);

    isSelected = false;
}

void YACReaderLibraryItemWidget::select()
{
    const auto &li = theme.libraryItem;
    QString styleSheet = QString("color: %1; background-color: %2; font-weight:bold;")
                                 .arg(li.selectedTextColor.name())
                                 .arg(li.selectedBackgroundColor.name());
    setStyleSheet(styleSheet);

    options->setHidden(false);

    icon->setPixmap(theme.libraryItem.libraryIconSelected.pixmap(16, 16));

    isSelected = true;
}

void YACReaderLibraryItemWidget::setName(const QString &name)
{
    this->name = name;
    nameLabel->setText(name);
}
