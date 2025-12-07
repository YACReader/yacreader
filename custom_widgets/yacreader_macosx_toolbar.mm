#include "yacreader_macosx_toolbar.h"
#include "QtWidgets/qmainwindow.h"

#include <QtGlobal>
#include <QtWidgets>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QWidget>
#include <QMacNativeWidget>
#include <qmacfunctions.h>
#include <qpa/qplatformnativeinterface.h>
#include <QsLog.h>

#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>

#import "shortcuts_manager.h"

//----------------------------
// A custom items separator for NSToolbar
@interface CustomSeparator : NSView

@end

@implementation CustomSeparator

- (void)drawRect:(NSRect)rect
{
    [[NSColor colorWithDeviceRed:0.5 green:0.5 blue:0.5 alpha:1] setFill];
    NSRectFill(rect);
    [super drawRect:rect];
}

@end

//----------------------------
// Toolbar delegate, needed for allow disabled/enabled items
@interface MyToolbarDelegate : NSObject <NSToolbarDelegate> {
@public
    YACReaderMacOSXToolbar *mytoolbar;
}

- (NSToolbarItem *)toolbar:(NSToolbar *)toolbar itemForItemIdentifier:(NSString *)itemIdent willBeInsertedIntoToolbar:(BOOL)willBeInserted;
- (NSArray *)toolbarDefaultItemIdentifiers:(NSToolbar *)toolbar;
- (NSArray *)toolbarAllowedItemIdentifiers:(NSToolbar *)toolbar;
//- (NSArray *)toolbarSelectableItemIdentifiers:(NSToolbar *)toolbar;
- (IBAction)itemClicked:(id)sender;
- (BOOL)validateToolbarItem:(NSToolbarItem *)theItem;
@end

@implementation MyToolbarDelegate

- (NSArray *)toolbarDefaultItemIdentifiers:(NSToolbar *)toolbar
{
    Q_UNUSED(toolbar);

    NSMutableArray *array = [[NSMutableArray alloc] init];

    QList<QMacToolBarItem *> items = mytoolbar->items();
    foreach (const QMacToolBarItem *item, items) {
        [array addObject:item->nativeToolBarItem().itemIdentifier];
    }
    return array;
}

- (NSArray *)toolbarAllowedItemIdentifiers:(NSToolbar *)toolbar
{
    Q_UNUSED(toolbar);

    NSMutableArray *array = [[NSMutableArray alloc] init];

    QList<QMacToolBarItem *> items = mytoolbar->items();
    foreach (const QMacToolBarItem *item, items) {
        [array addObject:item->nativeToolBarItem().itemIdentifier];
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
    if ([sender respondsToSelector:@selector(itemIdentifier)]) {
        NSToolbarItem *item = reinterpret_cast<NSToolbarItem *>(sender);

        QString identifier = QString::fromNSString([item itemIdentifier]);
        QMacToolBarItem *toolButton = reinterpret_cast<QMacToolBarItem *>(identifier.toULongLong());
        Q_EMIT toolButton->activated();
    }
}

- (NSToolbarItem *)toolbar:(NSToolbar *)toolbar itemForItemIdentifier:(NSString *)itemIdentifier willBeInsertedIntoToolbar:(BOOL)willBeInserted
{
    Q_UNUSED(toolbar);
    Q_UNUSED(willBeInserted);
    QList<QMacToolBarItem *> items = mytoolbar->items();

    foreach (const QMacToolBarItem *item, items) {
        NSToolbarItem *toolbarItem = item->nativeToolBarItem();
        if ([toolbarItem.itemIdentifier isEqual:itemIdentifier]) {

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

    if (mytoolbar->actions.contains(identifier)) {
        return mytoolbar->actions.value(identifier)->isEnabled();
    } else
        return NO;
}
@end

//----------------------------
// detect changes in native text field
// TODO implement validation and auto completion
@interface MyTextFieldDelegate : NSObject <NSTextFieldDelegate> {
@public
    YACReaderMacOSXSearchLineEdit *mylineedit;
}
@end

@implementation MyTextFieldDelegate

- (void)controlTextDidChange:(NSNotification *)notification
{
    NSTextField *textField = [notification object];
    NSLog(@"%@", [textField stringValue]);
    Q_EMIT mylineedit->filterChanged(QString::fromNSString([textField stringValue]));
}

@end
//----------------------------

YACReaderMacOSXToolbar::YACReaderMacOSXToolbar(QObject *parent)
    : viewSelector(0)
{
    // setup native toolbar
    nativeToolBar = nativeToolbar();
    [nativeToolBar setDisplayMode:NSToolbarDisplayModeIconOnly];
    [nativeToolBar setAllowsUserCustomization:NO];

    delegate = [[MyToolbarDelegate alloc] init];
    ((MyToolbarDelegate *)delegate)->mytoolbar = this;
    [nativeToolBar setDelegate:(MyToolbarDelegate *)delegate];

#ifdef YACREADER_LIBRARY
    NSWindow *nswindow = (NSWindow *)qApp->platformNativeInterface()->nativeResourceForWindow("nswindow", ((QMainWindow *)parent)->windowHandle());
    if ([nswindow respondsToSelector:@selector(setTitleVisibility:)]) {
        yosemite = true;
        // TODO yosemite new constants are not found in compilation time
        [nswindow setTitleVisibility:NSWindowTitleHidden];
        // TODO NSFullSizeContentViewWindowMask produces an offset in the windows' content
        // nswindow.styleMask |= 1 << 15; // NSFullSizeContentViewWindowMask;
        [nativeToolBar setSizeMode:NSToolbarSizeModeSmall]; // TODO figure out how to load specific images in Yosemite
    } else {
        [nativeToolBar setSizeMode:NSToolbarSizeModeSmall];
        yosemite = false;
    }
#else
    yosemite = false;
    [nativeToolBar setAutosavesConfiguration:YES]; // TODO this doesn't work
    [nativeToolBar setSizeMode:NSToolbarSizeModeSmall];
#endif
}

void YACReaderMacOSXToolbar::addAction(QAction *action)
{
    QMacToolBarItem *toolBarItem = addItem(action->icon(), action->text());
    if (action->data().toString() == TOGGLE_COMICS_VIEW_ACTION_YL)
        viewSelector = toolBarItem;
    connect(toolBarItem, &QMacToolBarItem::activated, action, [=] { emit action->triggered(); });

    NSToolbarItem *nativeItem = toolBarItem->nativeToolBarItem();
    actions.insert(QString::fromNSString(nativeItem.itemIdentifier), action);

    MacToolBarItemWrapper *wrapper = new MacToolBarItemWrapper(action, toolBarItem);
    // wrapper->actionToogled(true);
}

void YACReaderMacOSXToolbar::addDropDownItem(const QList<QAction *> &actions, const QAction *defaultAction)
{
    // TODO
}

void YACReaderMacOSXToolbar::addSpace(int size)
{
    QMacToolBarItem *toolBarItem = addItem(QIcon(), "");
    NSToolbarItem *nativeItem = toolBarItem->nativeToolBarItem();

    static const NSRect frameRect = { { 0.0, 0.0 }, { CGFloat(size), 16.0 } };
    NSView *view = [[NSView alloc] initWithFrame:frameRect];

    [nativeItem setView:view];
}

// reimplemented for convenience
void YACReaderMacOSXToolbar::addSeparator()
{
    // QMacToolBar::addSeparator();

    QMacToolBarItem *toolBarItem = addItem(QIcon(), "");
    NSToolbarItem *nativeItem = toolBarItem->nativeToolBarItem();

    static const NSRect frameRect = { { 0.0, 0.0 }, { 1, 16.0 } };
    CustomSeparator *view = [[CustomSeparator alloc] initWithFrame:frameRect];

    [nativeItem setView:view];
}

void YACReaderMacOSXToolbar::addStretch()
{
    QMacToolBarItem *toolBarItem = addItem(QIcon(), "");
    toolBarItem->setStandardItem(QMacToolBarItem::FlexibleSpace);
}

void YACReaderMacOSXToolbar::addWidget(QWidget *widget)
{
    // TODO fix it
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

YACReaderMacOSXSearchLineEdit *YACReaderMacOSXToolbar::addSearchEdit()
{
    QMacToolBarItem *toolBarItem = addItem(QIcon(), "");
    NSToolbarItem *nativeItem = toolBarItem->nativeToolBarItem();

    YACReaderMacOSXSearchLineEdit *searchEdit = new YACReaderMacOSXSearchLineEdit();

    if (yosemite)
        [nativeItem setView:(NSTextField *)searchEdit->getNSTextField()];
    else {
        static const NSRect searchEditFrameRect = { { 0.0, 0.0 }, { 165, 26.0 } };
        NSView *view = [[NSView alloc] initWithFrame:searchEditFrameRect];
        [view addSubview:((NSTextField *)searchEdit->getNSTextField())];
        [nativeItem setView:view];
    }

    return searchEdit;
}

// deprecated
QAction *YACReaderMacOSXToolbar::addFitToWidthSlider(QAction *attachToAction)
{
    QMacToolBarItem *toolBarItem = addItem(QIcon(":/images/viewer_toolbar/toWidthSlider.png"), "fit to width slider");

    NSToolbarItem *nativeItem = toolBarItem->nativeToolBarItem();
    actions.insert(QString::fromNSString(nativeItem.itemIdentifier), attachToAction);

    QAction *action = new QAction("", attachToAction->parent());

    connect(toolBarItem, &QMacToolBarItem::activated, action, [=] { emit action->triggered(); });

    return action;
}

void YACReaderMacOSXToolbar::updateViewSelectorIcon(const QIcon &icon)
{
    if (viewSelector)
        viewSelector->setIcon(icon);
}

void YACReaderMacOSXToolbar::attachToWindow(QMainWindow *window)
{
    QMacToolBar::attachToWindow(window->windowHandle());
}

YACReaderMacOSXSearchLineEdit::YACReaderMacOSXSearchLineEdit()
    : QObject()
{
    NSRect searchEditFrameRect = { { 0.0, -3.0 }, { 165, 32.0 } };
    // NSTextField * searchEdit = [[NSTextField alloc] initWithFrame:searchEditFrameRect];

    NSTextField *searchEdit = [[NSSearchField alloc] initWithFrame:searchEditFrameRect];
    //[searchEdit setBezelStyle:NSTextFieldRoundedBezel];

    [[searchEdit cell] setPlaceholderString:@"type to search"];

    MyTextFieldDelegate *delegate = [[MyTextFieldDelegate alloc] init];
    delegate->mylineedit = this;
    [searchEdit setDelegate:delegate];

    nstextfield = searchEdit;
}

void YACReaderMacOSXSearchLineEdit::setFocus(Qt::FocusReason reason)
{
    Q_UNUSED(reason)

    [((NSTextField *)nstextfield) becomeFirstResponder];
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
    emit filterChanged("");
}

void YACReaderMacOSXSearchLineEdit::clearText()
{
    // TODO be sure that this will not generate any event....
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

MacToolBarItemWrapper::MacToolBarItemWrapper(QAction *action, QMacToolBarItem *toolbaritem)
    : action(action), toolbaritem(toolbaritem)
{
    if (action->isCheckable()) {
        connect(action, &QAction::toggled, this, &MacToolBarItemWrapper::actionToggled);
        connect(toolbaritem, &QMacToolBarItem::activated, action, &QAction::toggle);
        updateIcon(action->isChecked());
    }
}

void MacToolBarItemWrapper::actionToggled(bool toogled)
{
    updateIcon(toogled);
}

void MacToolBarItemWrapper::updateIcon(bool enabled)
{
    if (enabled) {
        QIcon icon = action->icon();
        QPixmap tempPixmap = icon.pixmap(QSize(24, 24));
        QPainter painter;
        painter.begin(&tempPixmap);
        painter.fillRect(QRect(3, 21, 18, 1), QColor("#3F3F3F"));
        painter.fillRect(QRect(3, 22, 18, 1), QColor("#6E6E6E"));
        painter.fillRect(QRect(3, 23, 18, 1), QColor("#EEEEEE"));
        painter.end();

        toolbaritem->setIcon(QIcon(tempPixmap));
    } else
        toolbaritem->setIcon(action->icon());
}
#else

#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>

NSImage *QIconToNSImage(const QIcon &icon, const QSize &size, const QColor &color = QColor())
{
    QPixmap pixmap = icon.pixmap(size);
    QImage qImage = pixmap.toImage().convertToFormat(QImage::Format_RGBA8888);

    if (color.isValid()) {
        QPainter p;

        QImage mask(qImage);

        p.begin(&mask);
        p.setCompositionMode(QPainter::CompositionMode_SourceIn);
        QBrush brush(color);
        p.fillRect(QRect(0, 0, size.width(), size.height()), brush);
        p.end();

        p.begin(&qImage);
        p.setCompositionMode(QPainter::CompositionMode_Overlay);
        p.drawImage(0, 0, mask);
        p.end();
    }

    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    CGContextRef context = CGBitmapContextCreate(
            (void *)qImage.bits(),
            qImage.width(),
            qImage.height(),
            8,
            qImage.bytesPerLine(),
            colorSpace,
            kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big);

    CGImageRef cgImage = CGBitmapContextCreateImage(context);
    NSImage *nsImage = [[NSImage alloc] initWithCGImage:cgImage size:NSMakeSize(qImage.width(), qImage.height())];

    // Clean up
    CGImageRelease(cgImage);
    CGContextRelease(context);
    CGColorSpaceRelease(colorSpace);

    return nsImage;
}

void bindActionToNSToolbarItem(QAction *action, NSToolbarItem *toolbarItem, const QColor &iconColor = QColor())
{
    if (action == nullptr || toolbarItem == nil) {
        return;
    }

    auto update = [=] {
        toolbarItem.enabled = action->isEnabled();

        QString text = action->text();
        QString tooltip = action->toolTip();

        toolbarItem.label = text.isEmpty() ? @"" : [NSString stringWithUTF8String:text.toUtf8().constData()];
        toolbarItem.paletteLabel = toolbarItem.label;
        toolbarItem.toolTip = tooltip.isEmpty() ? @"" : [NSString stringWithUTF8String:tooltip.toUtf8().constData()];

        QIcon icon = action->icon();

        __auto_type image = QIconToNSImage(icon, { 24, 24 }, iconColor);

        if (action->isChecked()) {
            NSSize size = image.size;
            NSImage *decoratedImage = [[NSImage alloc] initWithSize:size];

            [decoratedImage lockFocus];

            NSRect rect = NSMakeRect(0, 0, size.width, size.height);
            NSBezierPath *path = [NSBezierPath bezierPathWithRoundedRect:rect xRadius:8 yRadius:8];
            [[NSColor colorWithCalibratedRed:0.8 green:0.8 blue:0.8 alpha:0.9] setFill];
            [path fill];

            NSRect imageRect = NSMakeRect(4, 4, size.width - 8, size.height - 8);
            [image drawInRect:imageRect
                     fromRect:NSZeroRect
                    operation:NSCompositingOperationSourceOver
                     fraction:1.0];

            [decoratedImage unlockFocus];

            toolbarItem.image = decoratedImage;
        } else {
            NSSize size = image.size;
            NSImage *decoratedImage = [[NSImage alloc] initWithSize:size];

            [decoratedImage lockFocus];

            NSRect imageRect = NSMakeRect(4, 4, size.width - 8, size.height - 8);
            [image drawInRect:imageRect
                     fromRect:NSZeroRect
                    operation:NSCompositingOperationSourceOver
                     fraction:1.0];

            [decoratedImage unlockFocus];

            toolbarItem.image = decoratedImage;
        }

        [image release];
    };

    if (action->isCheckable()) {
        QObject::connect(
                action, &QAction::triggered,
                [=](bool checked) {
                    update();
                });
    }

    QObject::connect(
            action, &QAction::enabledChanged,
            [=](bool enabled) {
                toolbarItem.enabled = enabled;
            });

    QObject::connect(
            action, &QAction::changed,
            [=]() {
                update();
            });

    toolbarItem.bordered = YES;

    update();
}

#ifdef YACREADER_LIBRARY

@interface YACReaderLibraryToolbarDelegate : NSObject <NSToolbarDelegate> {
@public
    YACReaderMacOSXToolbar *mytoolbar;
}

- (IBAction)itemClicked:(id)sender;

@end

@implementation YACReaderLibraryToolbarDelegate

- (NSArray<NSToolbarItemIdentifier> *)toolbarDefaultItemIdentifiers:(NSToolbar *)toolbar
{
    Q_UNUSED(toolbar);

    return @[
        @"Back",
        @"Forward",
        @"Settings",
        @"Server",
        @"Help",
        NSToolbarSpaceItemIdentifier,
        @"ToggleView",
        NSToolbarSpaceItemIdentifier,
        @"Search",
    ];
}

- (NSArray<NSToolbarItemIdentifier> *)toolbarAllowedItemIdentifiers:(NSToolbar *)toolbar
{
    Q_UNUSED(toolbar);

    return @[
        @"Back",
        @"Forward",
        @"Settings",
        @"Server",
        @"Help",
        @"ToggleView",
        @"Search",
        NSToolbarSpaceItemIdentifier,
    ];
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

    QAction *action = mytoolbar->actionFromIdentifier(identifier);
    ;

    if (action != nullptr) {
        action->trigger();
    }
}

- (NSToolbarItem *)toolbar:(NSToolbar *)toolbar itemForItemIdentifier:(NSString *)itemIdentifier willBeInsertedIntoToolbar:(BOOL)willBeInserted
{
    Q_UNUSED(toolbar);
    Q_UNUSED(willBeInserted);

    QString identifier = QString::fromNSString(itemIdentifier);

    if (identifier == "Search") {
        NSSearchToolbarItem *searchItem = [[NSSearchToolbarItem alloc] initWithItemIdentifier:itemIdentifier];

        searchItem.resignsFirstResponderWithCancel = true;
        searchItem.searchField.delegate = id<NSSearchFieldDelegate>(mytoolbar->getSearchEditDelegate());
        searchItem.toolTip = @"Search";

        return searchItem;
    }

    NSToolbarItem *toolbarItem = [[NSToolbarItem alloc] initWithItemIdentifier:itemIdentifier];

    toolbarItem.target = self;
    toolbarItem.action = @selector(itemClicked:);

    QAction *action = mytoolbar->actionFromIdentifier(identifier);

    if (identifier == "Back") {
        toolbarItem.navigational = YES;
    } else if (identifier == "Forward") {
        toolbarItem.navigational = YES;
    }

    bindActionToNSToolbarItem(action, toolbarItem);

    return toolbarItem;
}

- (BOOL)validateToolbarItem:(NSToolbarItem *)item
{

    QString identifier = QString::fromNSString([item itemIdentifier]);

    if (identifier == "Search") {
        return YES;
    }

    QAction *action = mytoolbar->actionFromIdentifier(identifier);

    if (action == nullptr) {
        return NO;
    }

    return action->isEnabled();
}

@end

@interface YACReaderLibrarySearchDelegate : NSObject <NSSearchFieldDelegate> {
@public
    YACReaderMacOSXToolbar *mytoolbar;
}
@end

@implementation YACReaderLibrarySearchDelegate

- (void)searchFieldDidStartSearching:(NSSearchField *)sender
{
}
- (void)searchFieldDidEndSearching:(NSSearchField *)sender
{
    [sender resignFirstResponder];
}
- (void)controlTextDidChange:(NSNotification *)notification
{
    NSSearchField *searchField = notification.object;
    NSLog(@"Search text changed: %@", searchField.stringValue);

    mytoolbar->emitFilterChange(QString::fromNSString(searchField.stringValue));
}

@end

YACReaderMacOSXToolbar::YACReaderMacOSXToolbar(QWidget *parent)
    : YACReaderMainToolBar(parent)
{
    backButton->setIconSize(QSize(24, 24));
    forwardButton->setIconSize(QSize(24, 24));
    settingsButton->setIconSize(QSize(24, 24));
    serverButton->setIconSize(QSize(24, 24));
    helpButton->setIconSize(QSize(24, 24));
    toggleComicsViewButton->setIconSize(QSize(24, 24));
}

QSize YACReaderMacOSXToolbar::sizeHint() const
{
    return QSize(400, 36);
}

void YACReaderMacOSXToolbar::addAction(QAction *action)
{
    if (backButton->defaultAction() == nullptr) {
        backButton->setDefaultAction(action);
        return;
    }
    if (forwardButton->defaultAction() == nullptr) {
        forwardButton->setDefaultAction(action);
        return;
    }
    if (settingsButton->defaultAction() == nullptr) {
        settingsButton->setDefaultAction(action);
        return;
    }
    if (serverButton->defaultAction() == nullptr) {
        serverButton->setDefaultAction(action);
        return;
    }
    if (helpButton->defaultAction() == nullptr) {
        helpButton->setDefaultAction(action);
        return;
    }
    if (toggleComicsViewButton->defaultAction() == nullptr) {
        toggleComicsViewButton->setDefaultAction(action);
        return;
    }
}

void YACReaderMacOSXToolbar::addSpace(int size)
{
}

void YACReaderMacOSXToolbar::addStretch()
{
}

YACReaderMacOSXSearchLineEdit *YACReaderMacOSXToolbar::addSearchEdit()
{
    auto search = new YACReaderMacOSXSearchLineEdit();

    setSearchWidget(search);

    return search;
}

void YACReaderMacOSXToolbar::updateViewSelectorIcon(const QIcon &icon)
{
}

void YACReaderMacOSXToolbar::attachToWindow(QMainWindow *window)
{
    NSView *nsview = (NSView *)window->winId();
    NSWindow *nswindow = [nsview window];

    YACReaderLibrarySearchDelegate *searchDelegate = [[YACReaderLibrarySearchDelegate alloc] init];
    this->searchEditDelegate = searchDelegate;
    searchDelegate->mytoolbar = this;

    // Create the NSToolbar
    NSToolbar *toolbar = [[NSToolbar alloc] initWithIdentifier:@"mainToolbar"];
    [toolbar setDisplayMode:NSToolbarDisplayModeIconOnly];
    [toolbar setShowsBaselineSeparator:false];

    __auto_type delegate = [[YACReaderLibraryToolbarDelegate alloc] init];
    delegate->mytoolbar = this;
    [toolbar setDelegate:delegate];

    [nswindow setToolbar:toolbar];
}

void YACReaderMacOSXToolbar::paintEvent(QPaintEvent *)
{
}

QAction *YACReaderMacOSXToolbar::actionFromIdentifier(const QString &identifier)
{
    if (identifier == "Back") {
        return backButton->defaultAction();
    } else if (identifier == "Forward") {
        return forwardButton->defaultAction();
    } else if (identifier == "Settings") {
        return settingsButton->defaultAction();
    } else if (identifier == "Server") {
        return serverButton->defaultAction();
    } else if (identifier == "Help") {
        return helpButton->defaultAction();
    } else if (identifier == "ToggleView") {
        return toggleComicsViewButton->defaultAction();
    }

    return nullptr;
}

#else

@interface YACReaderToolbarDelegate : NSObject <NSToolbarDelegate> {
@public
    YACReaderMacOSXToolbar *mytoolbar;
}

- (IBAction)itemClicked:(id)sender;

@end

@implementation YACReaderToolbarDelegate

- (NSArray<NSToolbarItemIdentifier> *)toolbarDefaultItemIdentifiers:(NSToolbar *)toolbar
{
    Q_UNUSED(toolbar);

    auto actions = mytoolbar->actions();
    NSMutableArray<NSToolbarItemIdentifier> *identifiers = [NSMutableArray arrayWithCapacity:actions.size()];

    for (QAction *action : actions) {
        [identifiers addObject:[NSString stringWithFormat:@"action_%p", action]];
    }

    return identifiers;
}

- (NSArray<NSToolbarItemIdentifier> *)toolbarAllowedItemIdentifiers:(NSToolbar *)toolbar
{
    Q_UNUSED(toolbar);

    auto actions = mytoolbar->actions();
    NSMutableArray<NSToolbarItemIdentifier> *identifiers = [NSMutableArray arrayWithCapacity:actions.size()];

    for (QAction *action : actions) {
        [identifiers addObject:[NSString stringWithFormat:@"action_%p", action]];
    }

    return identifiers;
}

// - (NSArray *)toolbarSelectableItemIdentifiers:(NSToolbar *)toolbar
// {
//     Q_UNUSED(toolbar);

//     auto actions = mytoolbar->actions();
//     NSMutableArray<NSToolbarItemIdentifier> *identifiers = [NSMutableArray arrayWithCapacity:actions.size()];

//     for (QAction *action : actions) {
//         if (action->isCheckable()) {
//             [identifiers addObject:[NSString stringWithFormat:@"action_%p", action]];
//         }
//     }

//     return identifiers;
// }

- (IBAction)itemClicked:(id)sender
{
    NSToolbarItem *item = reinterpret_cast<NSToolbarItem *>(sender);
    NSString *itemIdentifier = [item itemIdentifier];

    auto actions = mytoolbar->actions();

    for (QAction *action : actions) {
        if ([itemIdentifier isEqualTo:[NSString stringWithFormat:@"action_%p", action]]) {
            action->trigger();
        }
    }
}

- (NSToolbarItem *)toolbar:(NSToolbar *)toolbar itemForItemIdentifier:(NSString *)itemIdentifier willBeInsertedIntoToolbar:(BOOL)willBeInserted
{
    Q_UNUSED(toolbar);
    Q_UNUSED(willBeInserted);

    NSToolbarItem *toolbarItem = [[NSToolbarItem alloc] initWithItemIdentifier:itemIdentifier];

    toolbarItem.target = self;
    toolbarItem.action = @selector(itemClicked:);

    auto actions = mytoolbar->actions();

    for (QAction *action : actions) {
        if ([itemIdentifier isEqualTo:[NSString stringWithFormat:@"action_%p", action]]) {
            bindActionToNSToolbarItem(action, toolbarItem, QColor(200, 200, 200));
        }
    }

    return toolbarItem;
}

- (BOOL)validateToolbarItem:(NSToolbarItem *)item
{
    NSString *itemIdentifier = [item itemIdentifier];

    auto actions = mytoolbar->actions();

    for (QAction *action : actions) {
        if ([itemIdentifier isEqualTo:[NSString stringWithFormat:@"action_%p", action]]) {
            return action->isEnabled();
        }
    }

    return NO;
}

@end

YACReaderMacOSXToolbar::YACReaderMacOSXToolbar(QWidget *parent)
    : QWidget(parent)
{
    setMovable(false);
    setIconSize(QSize(24, 24));
}

void YACReaderMacOSXToolbar::attachToWindow(QMainWindow *window)
{
    NSView *nsview = (NSView *)window->winId();
    NSWindow *nswindow = [nsview window];

    NSToolbar *toolbar = [[NSToolbar alloc] initWithIdentifier:@"mainToolbar"];
    [toolbar setDisplayMode:NSToolbarDisplayModeIconOnly];
    [toolbar setShowsBaselineSeparator:false];

    __auto_type delegate = [[YACReaderToolbarDelegate alloc] init];
    delegate->mytoolbar = this;
    [toolbar setDelegate:delegate];

    [nswindow setToolbar:toolbar];
}

void YACReaderMacOSXToolbar::addStretch()
{
}

void YACReaderMacOSXToolbar::setHidden(bool hidden)
{
    NSView *nsView = reinterpret_cast<NSView *>(this->winId());
    NSWindow *window = [nsView window];
    if (window && window.toolbar) {
        window.toolbar.visible = !hidden;
    }
}
void YACReaderMacOSXToolbar::show()
{
    setHidden(false);
}

void YACReaderMacOSXToolbar::hide()
{
    setHidden(true);
}

#endif

#endif
