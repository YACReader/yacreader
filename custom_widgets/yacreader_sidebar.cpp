#include "yacreader_sidebar.h"

#include <QPainter>
#include <QLayout>

#include "yacreader_treeview.h"
#include "yacreader_library_list_widget.h"
#include "yacreader_search_line_edit.h"
#include "yacreader_titled_toolbar.h"

YACReaderSideBar::YACReaderSideBar(QWidget *parent) :
	QWidget(parent)
{
	setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Minimum);

	//widgets
	foldersView = new YACReaderTreeView;
	selectedLibrary = new YACReaderLibraryListWidget;
	foldersFilter = new YACReaderSearchLineEdit();

	librariesTitle = new YACReaderTitledToolBar(tr("LIBRARIES"));

	foldersTitle = new YACReaderTitledToolBar(tr("FOLDERS"));

	selectedLibrary->setContextMenuPolicy(Qt::ActionsContextMenu);
	selectedLibrary->setAttribute(Qt::WA_MacShowFocusRect,false);
	selectedLibrary->setFocusPolicy(Qt::NoFocus);

	foldersFilter->setAttribute(Qt::WA_MacShowFocusRect,false);
	foldersFilter->setPlaceholderText(tr("Search folders and comics"));

	//layout
	QVBoxLayout * l = new QVBoxLayout;

	l->setContentsMargins(0,0,0,0);
#ifndef Q_OS_MAC
	l->addSpacing(5);
#endif

	l->addWidget(librariesTitle);

#ifndef Q_OS_MAC
	{QWidget * w = new QWidget();
	w->setStyleSheet("QWidget {border:none; border-bottom:1px solid #636363;border-top:1px solid #292929;}");
	w->setMinimumHeight(2);

	l->addSpacing(4);

	l->addWidget(w);}

	l->addSpacing(3);
#endif

	l->addWidget(selectedLibrary);

#ifndef Q_OS_MAC
	l->addSpacing(6);

	{QWidget * w = new QWidget();
	w->setStyleSheet("QWidget {border:none; border-bottom:1px solid #636363;border-top:1px solid #292929;}");
	w->setMinimumHeight(2);

	l->addSpacing(5);

	l->addWidget(w);}

	l->addSpacing(4);
#else
	l->addSpacing(6);
#endif

	l->addWidget(foldersTitle);

#ifndef Q_OS_MAC
	{QWidget * w = new QWidget();
	w->setStyleSheet("QWidget {border:none; border-bottom:1px solid #636363;border-top:1px solid #292929;}");
	w->setMinimumHeight(2);

	l->addSpacing(4);

	l->addWidget(w);}


	l->addSpacing(4);
#endif

	l->addWidget(foldersView);

	l->addWidget(foldersFilter);

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
	return QSize(225,200);
}
