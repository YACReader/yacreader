#include "yacreader_macosx_toolbar.h"

#include <QWidget>
#include <QMacNativeWidget>

#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>

#import <Cocoa/Cocoa.h>

@interface CustomSeparator : NSView

@end


@implementation CustomSeparator

- (void) drawRect:(NSRect)rect {
    [[NSColor colorWithDeviceRed:0.5 green:0.5 blue:0.5 alpha:1] setFill];
    NSRectFill(rect);
    [super drawRect:rect];
}

@end


YACReaderMacOSXToolbar::YACReaderMacOSXToolbar(QObject *parent) :
    QMacToolBar(parent)
{
    //setup native toolbar
    nativeToolBar= nativeToolbar();
    [nativeToolBar setSizeMode:NSToolbarSizeModeSmall];
    [nativeToolBar setDisplayMode:NSToolbarDisplayModeIconOnly];

    //button testing
    /*QPixmap p(100,100);

    QIcon icon(p);
    QMacToolBarItem *toolBarItem = addItem(icon,"hola");
    NSToolbarItem * nativeItem = toolBarItem->nativeToolBarItem();


    [nativeItem setMaxSize:NSMakeSize(10,50)];
    [nativeItem setMinSize:NSMakeSize(10,50)];*/
    //toolBarItem->setStandardItem(QMacToolBarItem::FlexibleSpace);
    //connect(toolButton, SIGNAL(activated()), this, SLOT(fooClicked()))

    //window->window()->winId(); // create window->windowhandle()
    //attachToWindow(window->window()->windowHandle());
}

void YACReaderMacOSXToolbar::addAction(QAction *action)
{
    QMacToolBarItem *toolBarItem = addItem(action->icon(),action->text());

    connect(toolBarItem,SIGNAL(activated()),action, SIGNAL(triggered()));

    //TODO add support for enable/disable toolbaritems
}

void YACReaderMacOSXToolbar::addDropDownItem(const QList<QAction *> &actions, const QAction *defaultAction)
{
    foreach(QAction * action, actions)
    {

    }
}

void YACReaderMacOSXToolbar::addSpace(int size)
{
    QMacToolBarItem *toolBarItem = addItem(QIcon(),"");
    //NSToolbarItem * nativeItem = toolBarItem->nativeToolBarItem();
    toolBarItem->setStandardItem(QMacToolBarItem::Space);

    NSToolbarItem * nativeItem = toolBarItem->nativeToolBarItem();

    //TODO this doesn't work
    [nativeItem setMaxSize:NSMakeSize(size,24)];
    [nativeItem setMinSize:NSMakeSize(size,24)];
}

void YACReaderMacOSXToolbar::addSeparator()
{
    //QMacToolBar::addSeparator();

    QMacToolBarItem *toolBarItem = addItem(QIcon(),"");
    NSToolbarItem * nativeItem = toolBarItem->nativeToolBarItem();

    static const NSRect buttonFrameRect = { { 0.0, 0.0 }, { 1, 16.0 } };
    CustomSeparator *view = [[CustomSeparator alloc] initWithFrame:buttonFrameRect];

    [nativeItem setView:view];
}

void YACReaderMacOSXToolbar::addStretch()
{
    QMacToolBarItem *toolBarItem = addItem(QIcon(),"");
    toolBarItem->setStandardItem(QMacToolBarItem::FlexibleSpace);
}

void YACReaderMacOSXToolbar::addWidget(QWidget *widget)
{
   /* QMacNativeWidget *nativeWidget = new QMacNativeWidget();
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(widget);
    nativeWidget->setLayout(layout);


    NSView *nativeWidgetView = reinterpret_cast<NSView *>(nativeWidget->winId());
    QMacToolBarItem *toolBarItem = addItem(QIcon(),"");
    NSToolbarItem * nativeItem = toolBarItem->nativeToolBarItem();
    [nativeItem setView:nativeWidgetView];*/
}

void YACReaderMacOSXToolbar::show()
{
    [nativeToolBar setVisible:YES];
}

void YACReaderMacOSXToolbar::hide()
{
    [nativeToolBar setVisible:NO];
}
