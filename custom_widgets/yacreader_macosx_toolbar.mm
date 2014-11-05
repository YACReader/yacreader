#include "yacreader_macosx_toolbar.h"

#include <QWidget>
#include <QMacNativeWidget>
#include <qmacfunctions.h>

#include <QsLog.h>

#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>

//----------------------------
//A custom items separator for NSToolbar
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
//Toolbar delegate, needed for allow disabled/enabled items
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
- (BOOL)validateToolbarItem:(NSToolbarItem *)theItem;
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
    QString identifier = QString::fromNSString(theItem.itemIdentifier);

    if(mytoolbar->actions.contains(identifier))
    {
        return mytoolbar->actions.value(identifier)->isEnabled();
    }
    else return NO;
}
@end

//----------------------------
//detect changes in native text field
//TODO implement validation and auto completion
@interface MyTextFieldDelegate : NSObject <NSTextFieldDelegate>
{
@public
    YACReaderMacOSXSearchLineEdit * mylineedit;
}
@end

@implementation MyTextFieldDelegate

- (void)controlTextDidChange:(NSNotification *)notification {
    NSTextField *textField = [notification object];
    Q_EMIT mylineedit->filterChanged(YACReader::NoModifiers, QString::fromNSString([textField stringValue]));
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
}

void YACReaderMacOSXToolbar::addAction(QAction *action)
{
    QMacToolBarItem *toolBarItem = addItem(action->icon(),action->text());
    connect(toolBarItem,SIGNAL(activated()),action, SIGNAL(triggered()));
    NSToolbarItem * nativeItem = toolBarItem->nativeToolBarItem();
    actions.insert(QString::fromNSString(nativeItem.itemIdentifier),action);
}

void YACReaderMacOSXToolbar::addDropDownItem(const QList<QAction *> &actions, const QAction *defaultAction)
{
    //TODO
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

    //if a fix isn't found probably it is better to use QMacToolBar::
}

//reimplemented for convenience
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

YACReaderMacOSXSearchLineEdit * YACReaderMacOSXToolbar::addSearchEdit()
{
    QMacToolBarItem *toolBarItem = addItem(QIcon(),"");
    NSToolbarItem * nativeItem = toolBarItem->nativeToolBarItem();

    YACReaderMacOSXSearchLineEdit * searchEdit = new YACReaderMacOSXSearchLineEdit();
    static const NSRect searchEditFrameRect = { { 0.0, 0.0 }, { 165, 26.0 } };
    NSView * view = [[NSView alloc] initWithFrame:searchEditFrameRect];
    [view addSubview:((NSTextField *)searchEdit->getNSTextField())];
    [nativeItem setView:view];

    return searchEdit;
}


YACReaderMacOSXSearchLineEdit::YACReaderMacOSXSearchLineEdit()
    :QObject()
{
    static const NSRect searchEditFrameRect = { { 0.0, -3.0 }, { 165, 32.0 } };
    //NSTextField * searchEdit = [[NSTextField alloc] initWithFrame:searchEditFrameRect];

    NSTextField * searchEdit = [[NSSearchField alloc] initWithFrame:searchEditFrameRect];
    //[searchEdit setBezelStyle:NSTextFieldRoundedBezel];

    [[searchEdit cell] setPlaceholderString:@"type to search"];

    MyTextFieldDelegate * delegate = [[MyTextFieldDelegate alloc] init];
    delegate->mylineedit = this;
    [searchEdit setDelegate:delegate];

    nstextfield = searchEdit;
}

void *YACReaderMacOSXSearchLineEdit::getNSTextField()
{
    return nstextfield;
}

QString YACReaderMacOSXSearchLineEdit::text()
{
    return QString::fromNSString([((NSTextField *)nstextfield) stringValue]);
}

void YACReaderMacOSXSearchLineEdit::clear()
{
    [((NSTextField *)nstextfield) setStringValue:@""];
}

void YACReaderMacOSXSearchLineEdit::setDisabled(bool disabled)
{
    [((NSTextField *)nstextfield) setEnabled:!disabled];
}

void YACReaderMacOSXSearchLineEdit::setEnabled(bool enabled)
{
    [((NSTextField *)nstextfield) setEnabled:enabled];
}
