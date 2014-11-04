#include "yacreader_macosx_toolbar.h"

#include <QWidget>
#include <QMacNativeWidget>
#include <qmacfunctions.h>

#include <QsLog.h>

#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>

//----------------------------

@interface CustomSeparator : NSView

@end


@implementation CustomSeparator

- (void) drawRect:(NSRect)rect {
    [[NSColor colorWithDeviceRed:0.5 green:0.5 blue:0.5 alpha:1] setFill];
    NSRectFill(rect);
    [super drawRect:rect];
}

@end

//----------------------------
@interface MyToolbarDelegate : NSObject <NSToolbarDelegate>
{
@public
    YACReaderMacOSXToolbar * mytoolbar;
}

- (NSToolbarItem *) toolbar:(NSToolbar *)toolbar itemForItemIdentifier:(NSString *) itemIdent willBeInsertedIntoToolbar:(BOOL) willBeInserted;
- (NSArray *)toolbarDefaultItemIdentifiers:(NSToolbar*)toolbar;
- (NSArray *)toolbarAllowedItemIdentifiers:(NSToolbar*)toolbar;
//- (NSArray *)toolbarSelectableItemIdentifiers:(NSToolbar *)toolbar;
- (IBAction)itemClicked:(id)sender;
@end


@implementation MyToolbarDelegate

- (NSArray *)toolbarDefaultItemIdentifiers:(NSToolbar*)toolbar
{
    Q_UNUSED(toolbar);

    NSMutableArray *array = [[NSMutableArray alloc] init];

    QList<QMacToolBarItem *> items = mytoolbar->items();
    foreach (const QMacToolBarItem * item, items) {
        [array addObject : item->nativeToolBarItem().itemIdentifier];
    }
    return array;
    //return toolbarPrivate->getItemIdentifiers(toolbarPrivate->items, false);
}

- (NSArray *)toolbarAllowedItemIdentifiers:(NSToolbar*)toolbar
{
    Q_UNUSED(toolbar);

    NSMutableArray *array = [[NSMutableArray alloc] init];

    QList<QMacToolBarItem *> items = mytoolbar->items();
    foreach (const QMacToolBarItem * item, items) {
        [array addObject : item->nativeToolBarItem().itemIdentifier];
    }
    return array;
    //return toolbarPrivate->getItemIdentifiers(toolbarPrivate->allowedItems, false);
}
/*
- (NSArray *)toolbarSelectableItemIdentifiers: (NSToolbar *)toolbar
{
    Q_UNUSED(toolbar);

    NSMutableArray *array = [[NSMutableArray alloc] init];

    QList<QMacToolBarItem *> items = mytoolbar->items();
    foreach (const QMacToolBarItem * item, items) {
        [array addObject : item->nativeToolBarItem().itemIdentifier];
    }
    return array;
    //NSMutableArray *array = toolbarPrivate->getItemIdentifiers(toolbarPrivate->items, true);
    //[array addObjectsFromArray:toolbarPrivate->getItemIdentifiers(toolbarPrivate->allowedItems, true)];
    //return array;
}*/

- (IBAction)itemClicked:(id)sender
{
    NSToolbarItem *item = reinterpret_cast<NSToolbarItem *>(sender);

    QString identifier = QString::fromNSString([item itemIdentifier]);
    QMacToolBarItem *toolButton = reinterpret_cast<QMacToolBarItem *>(identifier.toULongLong());
    Q_EMIT toolButton->activated();
}

- (NSToolbarItem *) toolbar: (NSToolbar *)toolbar itemForItemIdentifier: (NSString *) itemIdentifier willBeInsertedIntoToolbar:(BOOL) willBeInserted
{
    Q_UNUSED(toolbar);
    Q_UNUSED(willBeInserted);
    QList<QMacToolBarItem *> items = mytoolbar->items();

    foreach (const QMacToolBarItem * item, items) {
        NSToolbarItem *toolbarItem = item->nativeToolBarItem();
        if([toolbarItem.itemIdentifier isEqual:itemIdentifier])
        {

        [toolbarItem setTarget:self];
        [toolbarItem setAction:@selector(itemClicked:)];

        return toolbarItem;
        }
    }
    return nil;
}

- (BOOL)validateToolbarItem:(NSToolbarItem *)theItem
{
    int i = -1;

    QString identifier = QString::fromNSString(theItem.itemIdentifier);

    if(mytoolbar->actions.contains(identifier))
    {
        return mytoolbar->actions.value(identifier)->isEnabled();
    }
    else return NO;
}
@end

//----------------------------

YACReaderMacOSXToolbar::YACReaderMacOSXToolbar(QObject *parent)
{
    //setup native toolbar
    nativeToolBar= nativeToolbar();
    [nativeToolBar setSizeMode:NSToolbarSizeModeSmall];
    [nativeToolBar setDisplayMode:NSToolbarDisplayModeIconOnly];

    delegate = [[MyToolbarDelegate alloc] init];
    ((MyToolbarDelegate *)delegate)->mytoolbar = this;
    [nativeToolBar setDelegate:(MyToolbarDelegate *)delegate];
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
    NSToolbarItem * nativeItem = toolBarItem->nativeToolBarItem();
    //[nativeItem setTarget:[[MyToolbarDelegate alloc] init]];

    [nativeToolBar  validateVisibleItems];

    actions.insert(QString::fromNSString(nativeItem.itemIdentifier),action);
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
    //TODO fix it
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

void YACReaderMacOSXToolbar::addSearchEdit()
{
    QMacToolBarItem *toolBarItem = addItem(QIcon(),"");
    NSToolbarItem * nativeItem = toolBarItem->nativeToolBarItem();

    static const NSRect searchEditFrameRect = { { 0.0, 0.0 }, { 150, 24.0 } };
    NSTextField * searchEdit = [[NSTextField alloc] initWithFrame:searchEditFrameRect];

    [searchEdit setBezelStyle:NSTextFieldRoundedBezel];

    [nativeItem setView:searchEdit];
}
