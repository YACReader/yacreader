#include "scraper_tableview.h"

#include <QHeaderView>

ScraperTableView::ScraperTableView(QWidget *parent) :
	QTableView(parent)
{
    QString tableStylesheet = "QTableView {color:white; border:0px;alternate-background-color: #2E2E2E;background-color: #2B2B2B; outline: 0px;}"
			"QTableView::item {outline: 0px; border: 0px; color:#FFFFFF;}"
			"QTableView::item:selected {outline: 0px; background-color: #555555;  }"
			"QHeaderView::section:horizontal {background-color:#292929; border-bottom:1px solid #1F1F1F; border-right:1px solid qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #292929, stop: 1 #1F1F1F); border-left:none; border-top:none; padding:4px; color:#ebebeb;}"
			"QHeaderView::section:vertical {border-bottom: 1px solid #DFDFDF;border-top: 1px solid #FEFEFE;}"
			"QHeaderView::down-arrow {image: url(':/images/comic_vine/downArrow.png');}"
			"QHeaderView::up-arrow {image: url(':/images/comic_vine/upArrow.png');}"
			"QScrollBar:vertical { border: none; background: #2B2B2B; width: 3px; margin: 0; }"
			"QScrollBar:horizontal { border: none; background: #2B2B2B; height: 3px; margin: 0; }"
			"QScrollBar::handle:vertical { background: #DDDDDD; width: 7px; min-height: 20px; }"
			"QScrollBar::handle:horizontal { background: #DDDDDD; width: 7px; min-height: 20px; }"
			"QScrollBar::add-line:vertical { border: none; background: #404040; height: 10px; subcontrol-position: bottom; subcontrol-origin: margin; margin: 0 3px 0 0;}"
			"QScrollBar::sub-line:vertical {  border: none; background: #404040; height: 10px; subcontrol-position: top; subcontrol-origin: margin; margin: 0 3px 0 0;}"
			"QScrollBar::add-line:horizontal { border: none; background: #404040; width: 10px; subcontrol-position: bottom; subcontrol-origin: margin; margin: 0 0 3px 0;}"
			"QScrollBar::sub-line:horizontal {  border: none; background: #404040; width: 10px; subcontrol-position: top; subcontrol-origin: margin; margin: 0 0 3px 0;}"
			"QScrollBar::up-arrow:vertical {border:none;width: 9px;height: 6px;background: url(':/images/folders_view/line-up.png') center top no-repeat;}"
			"QScrollBar::down-arrow:vertical {border:none;width: 9px;height: 6px;background: url(':/images/folders_view/line-down.png') center top no-repeat;}"
			"QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical, QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal {background: none; }";

	setStyleSheet(tableStylesheet);

	setShowGrid(false);
#if QT_VERSION >= 0x050000
	verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
#else
	verticalHeader()->setResizeMode(QHeaderView::Fixed);
#endif

    horizontalHeader()->setStretchLastSection(true);
#if QT_VERSION >= 0x050000
	horizontalHeader()->setSectionsClickable(false);
#else
	horizontalHeader()->setClickable(false);
#endif
	//comicView->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
	verticalHeader()->setDefaultSectionSize(24);
#if QT_VERSION >= 0x050000
	verticalHeader()->setSectionsClickable(false); //TODO comportamiento anómalo
#else
	verticalHeader()->setClickable(false); //TODO comportamiento anómalo
#endif

	setCornerButtonEnabled(false);

	setSelectionBehavior(QAbstractItemView::SelectRows);
	setSelectionMode(QAbstractItemView::ExtendedSelection);

	setAlternatingRowColors(true);

	verticalHeader()->hide();

    setSelectionMode(QAbstractItemView::SingleSelection);
}
