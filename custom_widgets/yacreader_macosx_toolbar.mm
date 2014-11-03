#include "yacreader_macosx_toolbar.h"

#include <QWidget>

#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>

YACReaderMacOSXToolbar::YACReaderMacOSXToolbar(QWidget * window, QObject *parent) :
    QMacToolBar(parent)
{
    //setup native toolbar
    NSToolbar * nativeToolBar= nativeToolbar();
    [nativeToolBar setSizeMode:NSToolbarSizeModeSmall];
    [nativeToolBar setDisplayMode:NSToolbarDisplayModeIconOnly];

    //button testing
    QPixmap p(100,100);

    QIcon icon(p);
    QMacToolBarItem *toolBarItem = addItem(icon,"hola");
    NSToolbarItem * nativeItem = toolBarItem->nativeToolBarItem();


    [nativeItem setMaxSize:NSMakeSize(10,50)];
    [nativeItem setMinSize:NSMakeSize(10,50)];
    //toolBarItem->setStandardItem(QMacToolBarItem::FlexibleSpace);
    //connect(toolButton, SIGNAL(activated()), this, SLOT(fooClicked()))

    window->window()->winId(); // create window->windowhandle()
    attachToWindow(window->window()->windowHandle());
}
