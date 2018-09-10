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

    //installEventFilter(this);

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
    mainLayout->addWidget(nameLabel, Qt::AlignLeft);
    mainLayout->addStretch();
    mainLayout->addWidget(options);
    /*mainLayout->addWidget(up);
	mainLayout->addWidget(down);*/

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
    setStyleSheet(Theme::currentTheme().itemLibraryNoSelectedStyleSheet);

    QPixmap iconPixmap(":/images/sidebar/libraryIcon.png");
    icon->setPixmap(iconPixmap);

    /*up->setHidden(true);
	down->setHidden(true);*/

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
