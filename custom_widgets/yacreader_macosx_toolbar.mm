#include "yacreader_macosx_toolbar.h"

#include "QtWidgets/qmainwindow.h"

#include <QtWidgets>

#import <AppKit/AppKit.h>
#import <Cocoa/Cocoa.h>
#import <Foundation/Foundation.h>

// size is in points, the returned NSImage is backed by a bitmap that matches the screen scale
NSImage *QIconToNSImage(const QIcon &icon, const QSize &size, const QColor &color = QColor())
{
    const qreal dpr = std::max(qApp->devicePixelRatio(), 2.0);
    QPixmap pixmap = icon.pixmap(size, dpr);
    QImage qImage = pixmap.toImage().convertToFormat(QImage::Format_RGBA8888);

    if (color.isValid()) {
        QPainter p;

        QImage mask(qImage);

        p.begin(&mask);
        p.setCompositionMode(QPainter::CompositionMode_SourceIn);
        QBrush brush(color);
        p.fillRect(mask.rect(), brush);
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
    const QSizeF pointSize = QSizeF(qImage.size()) / qImage.devicePixelRatio();
    NSImage *nsImage = [[NSImage alloc] initWithCGImage:cgImage size:NSMakeSize(pointSize.width(), pointSize.height())];

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

        // Same visual size as the SF Symbols used by native toolbars (e.g. Finder)
        const QSize iconSize(16, 16);
        __auto_type image = QIconToNSImage(icon, iconSize, iconColor);

        if (image.size.width == 0 || image.size.height == 0) {
            [image release];
            return;
        }

        if (action->isCheckable()) {
            // Checkable items need some room around the icon to draw the checked background
            const bool checked = action->isChecked();
            const CGFloat padding = 4;
            NSSize iconPointSize = image.size;
            NSSize size = NSMakeSize(iconPointSize.width + 2 * padding, iconPointSize.height + 2 * padding);

            toolbarItem.image = [NSImage imageWithSize:size
                                               flipped:NO
                                        drawingHandler:^BOOL(NSRect rect) {
                                            if (checked) {
                                                NSBezierPath *path = [NSBezierPath bezierPathWithRoundedRect:rect xRadius:5 yRadius:5];
                                                [[NSColor colorWithCalibratedRed:0.8 green:0.8 blue:0.8 alpha:0.9] setFill];
                                                [path fill];
                                            }

                                            [image drawInRect:NSMakeRect(padding, padding, iconPointSize.width, iconPointSize.height)
                                                     fromRect:NSZeroRect
                                                    operation:NSCompositingOperationSourceOver
                                                     fraction:1.0];
                                            return YES;
                                        }];
        } else {
            // Without an explicit color let AppKit tint the icon (light/dark appearance, disabled state)
            [image setTemplate:!iconColor.isValid()];
            toolbarItem.image = image;
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

// Separators split the toolbar items into visual groups, the stretch pushes the following items to the trailing edge
static const char *toolbarStretchProperty = "yacreaderToolbarStretch";

static NSToolbarItemIdentifier identifierForAction(QAction *action)
{
    if (action->isSeparator()) {
        return action->property(toolbarStretchProperty).toBool() ? NSToolbarFlexibleSpaceItemIdentifier : NSToolbarSpaceItemIdentifier;
    }

    return [NSString stringWithFormat:@"action_%p", action];
}

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
        [identifiers addObject:identifierForAction(action)];
    }

    return identifiers;
}

- (NSArray<NSToolbarItemIdentifier> *)toolbarAllowedItemIdentifiers:(NSToolbar *)toolbar
{
    Q_UNUSED(toolbar);

    auto actions = mytoolbar->actions();
    NSMutableArray<NSToolbarItemIdentifier> *identifiers = [NSMutableArray arrayWithCapacity:actions.size() + 2];

    for (QAction *action : actions) {
        if (!action->isSeparator()) {
            [identifiers addObject:identifierForAction(action)];
        }
    }

    [identifiers addObject:NSToolbarSpaceItemIdentifier];
    [identifiers addObject:NSToolbarFlexibleSpaceItemIdentifier];

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

void YACReaderMacOSXToolbar::addSeparator()
{
    auto *separator = new QAction(this);
    separator->setSeparator(true);
    addAction(separator);
}

void YACReaderMacOSXToolbar::addStretch()
{
    auto *stretch = new QAction(this);
    stretch->setSeparator(true);
    stretch->setProperty(toolbarStretchProperty, true);
    addAction(stretch);
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
