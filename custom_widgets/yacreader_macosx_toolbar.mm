#include "yacreader_macosx_toolbar.h"
#include "QtWidgets/qmainwindow.h"

#include <QtWidgets>

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
        if (icon.isNull()) {
            return;
        }

        __auto_type image = QIconToNSImage(icon, { 24, 24 }, iconColor);

        if (image.size.width == 0 || image.size.height == 0) {
            [image release];
            return;
        }

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
