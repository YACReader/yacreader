#include "yacreader_sidebar.h"

#include <QPainter>
#include <QLayout>

#include "yacreader_folders_view.h"
#include "yacreader_reading_lists_view.h"
#include "yacreader_library_list_widget.h"
#include "yacreader_search_line_edit.h"
#include "yacreader_titled_toolbar.h"


YACReaderSideBar::YACReaderSideBar(QWidget *parent) :
	QWidget(parent)
{
	setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Minimum);

	//widgets
    foldersView = new YACReaderFoldersView;
    readingListsView = new YACReaderReadingListsView;
	selectedLibrary = new YACReaderLibraryListWidget;

	librariesTitle = new YACReaderTitledToolBar(tr("LIBRARIES"));
	foldersTitle = new YACReaderTitledToolBar(tr("FOLDERS"));
    readingListsTitle = new YACReaderTitledToolBar(tr("READING LISTS"));

    splitter = new QSplitter(this);
    splitter->setOrientation(Qt::Vertical);

#ifndef Q_OS_MAC
    splitter->setStyleSheet("QSplitter::handle { "
                            " image: none; background-color = black; "
                            " }"
                            "QSplitter::handle:vertical { height: 39px;}");
#else
    splitter->setStyleSheet("QSplitter::handle:vertical { height: 26px; background-color: transparent;}");
#endif

	selectedLibrary->setContextMenuPolicy(Qt::ActionsContextMenu);
	selectedLibrary->setAttribute(Qt::WA_MacShowFocusRect,false);
	selectedLibrary->setFocusPolicy(Qt::NoFocus);

	//layout
	QVBoxLayout * l = new QVBoxLayout;

	l->setContentsMargins(0,0,0,0);

    //LIBRARIES-------------------------------------------------------
#ifndef Q_OS_MAC
	l->addSpacing(5);
#endif

	l->addWidget(librariesTitle);

#ifndef Q_OS_MAC
	l->addSpacing(4);
    l->addWidget(new YACReaderSideBarSeparator(this));
	l->addSpacing(3);
#endif

	l->addWidget(selectedLibrary);
#ifndef Q_OS_MAC
    l->addSpacing(11);
#else
     l->addSpacing(6);
#endif

    //END LIBRARIES---------------------------------------------------

    //FOLDERS---------------------------------------------------------
    QWidget * foldersContainer = new QWidget(this);
    QVBoxLayout * foldersLayout = new QVBoxLayout;
    foldersLayout->setContentsMargins(0,0,0,0);
    foldersLayout->setSpacing(0);

#ifndef Q_OS_MAC
    //foldersLayout->addSpacing(6);

    //foldersLayout->addSpacing(5);
    foldersLayout->addWidget(new YACReaderSideBarSeparator(this));
    foldersLayout->addSpacing(4);
#else
    //foldersLayout->addSpacing(6);
#endif

    foldersLayout->addWidget(foldersTitle);

#ifndef Q_OS_MAC
    foldersLayout->addSpacing(4);
    foldersLayout->addWidget(new YACReaderSideBarSeparator(this));
    foldersLayout->addSpacing(4);
#endif

    foldersLayout->addWidget(foldersView);
    foldersLayout->addSpacing(6);

    foldersContainer->setLayout(foldersLayout);
    splitter->addWidget(foldersContainer);
    //END FOLDERS------------------------------------------------------

    //READING LISTS----------------------------------------------------
    splitter->addWidget(readingListsView);

    QVBoxLayout * readingListsHeaderLayout = new QVBoxLayout;
    readingListsHeaderLayout->setContentsMargins(0,0,0,0);
    readingListsHeaderLayout->setSpacing(0);

#ifndef Q_OS_MAC
    //readingListsHeaderLayout->addSpacing(6);

    //readingListsHeaderLayout->addSpacing(5);
    readingListsHeaderLayout->addWidget(new YACReaderSideBarSeparator(this));
    readingListsHeaderLayout->addSpacing(4);
#else
    //readingListsHeaderLayout->addSpacing(6);
#endif

    readingListsHeaderLayout->addWidget(readingListsTitle);

#ifndef Q_OS_MAC
    readingListsHeaderLayout->addSpacing(4);
    readingListsHeaderLayout->addWidget(new YACReaderSideBarSeparator(this));
    readingListsHeaderLayout->addSpacing(4);
#endif

    //readingListsLayout->addWidget(readingListsView);
    readingListsHeaderLayout->addStretch();
    QSplitterHandle * handle = splitter->handle(1);
    //handle->setCursor(QCursor(Qt::ArrowCursor));
    handle->setLayout(readingListsHeaderLayout);
    //END READING LISTS------------------------------------------------

    l->addWidget(splitter);
    l->setSpacing(0);

	setLayout(l);
}


void YACReaderSideBar::paintEvent(QPaintEvent * event)
{
	Q_UNUSED(event)

#ifdef Q_OS_MAC
	QPainter painter(this);

	QLinearGradient lG(0,0,0,height());

	lG.setColorAt(0,QColor("#E8ECF1"));
	lG.setColorAt(1,QColor("#D1D8E0"));

	painter.fillRect(0,0,width(),height(),lG);
#else
	QPainter painter(this);

	painter.fillRect(0,0,width(),height(),QColor("#454545"));
	//QWidget::paintEvent(event);
#endif

	

	//QPixmap shadow(":/images/side_bar/shadow.png");
	//painter.drawPixmap(width()-shadow.width(),0,shadow.width(),height(),shadow);

	//   painter.setRenderHint(QPainter::Antialiasing);
	// painter.drawLine(rect().topLeft(), rect().bottomRight());

	//QWidget::paintEvent(event);
}

QSize YACReaderSideBar::sizeHint() const
{
    return QSize(275,200);
}

YACReaderSideBarSeparator::YACReaderSideBarSeparator(QWidget *parent)
    :QWidget(parent)
{
    setFixedHeight(1);
}

void YACReaderSideBarSeparator::paintEvent(QPaintEvent * event)
{
   Q_UNUSED(event)

   QPainter painter(this);

   painter.fillRect(5,0,width()-10,height(),QColor("#575757"));
}
