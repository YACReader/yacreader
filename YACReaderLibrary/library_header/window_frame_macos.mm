#include "header_model.h"
#include "library_header.h"
#include "window_frame.h"

#include <QAction>
#include <QEvent>
#include <QIcon>
#include <QImage>
#include <QMainWindow>
#include <QMenu>
#include <QTimer>
#include <QWindow>
#include <QtMath>

#import <AppKit/AppKit.h>
#include <algorithm>
#include <cmath>

class MacWindowFrame;

@interface YRLibraryHeaderToolbarDelegate : NSObject <NSToolbarDelegate, NSSearchFieldDelegate> {
@public
    MacWindowFrame *frame;
}
- (void)itemClicked:(NSToolbarItem *)sender;
- (void)viewModeChanged:(id)sender;
- (void)searchMenuItemClicked:(NSMenuItem *)sender;
@end

// Covers the header strip, which sits under the title bar but inside Qt's
// view. AppKit won't move the window from there on its own (Qt's view is
// opaque, so it doesn't let mouse-downs move the window), so the drag is
// handed to the window server like Chromium and Zed do; it does the moving
// and the tiling. Only the double-click action is ours, and it follows the
// "Double-click a window's title bar to" setting.
@interface YRHeaderDragView : NSView
@end

@implementation YRHeaderDragView
- (BOOL)mouseDownCanMoveWindow
{
    return NO;
}

- (void)mouseDown:(NSEvent *)event
{
    if (event.clickCount != 2) {
        [self.window performWindowDragWithEvent:event];
        return;
    }
    // "Fill" (macOS 15+) has no public API; zoom is the closest match. The
    // key is missing until the user changes the setting.
    NSString *action = [NSUserDefaults.standardUserDefaults stringForKey:@"AppleActionOnDoubleClick"];
    if ([action isEqualToString:@"Minimize"])
        [self.window performMiniaturize:nil];
    else if (![action isEqualToString:@"None"])
        [self.window performZoom:nil];
}
@end

static NSString *const SidebarLeadId = @"yr.sidebarLead";
static NSString *const SettingsId = @"yr.settings";
static NSString *const ServerId = @"yr.server";
static NSString *const HelpId = @"yr.help";
static NSString *const SidebarEndId = @"yr.sidebarEnd";
static NSString *const BackId = @"yr.back";
static NSString *const ForwardId = @"yr.forward";
static NSString *const FolderId = @"yr.folder";
static NSString *const ViewModeId = @"yr.viewMode";
static NSString *const SearchId = @"yr.search";

namespace {

constexpr int ItemIconSize = 18;
constexpr int ViewIconSize = 16;
constexpr CGFloat FolderLabelInset = 8;
constexpr int MaxAlignmentPasses = 12;
// The sidebar end spacer starts this far before the sidebar edge; with the
// toolbar item spacing, the sidebar glass then ends in line with the sidebar
// title buttons.
constexpr CGFloat SidebarGroupInset = 4;
// Width of the sidebar end spacer, which separates the sidebar glass from the
// navigation glass; it puts back and forward in line with the content.
constexpr CGFloat NavigationGap = 24;

// Template images use only the alpha channel; AppKit draws them in the
// toolbar colors (light/dark appearance, disabled state, Liquid Glass).
NSImage *templateImage(const QIcon &icon, int size)
{
    if (icon.isNull())
        return nil;
    const QImage image = icon.pixmap(QSize(size, size), 2.0).toImage();
    CGImageRef cgImage = image.toCGImage();
    if (cgImage == nullptr)
        return nil;
    NSImage *nsImage = [[NSImage alloc] initWithCGImage:cgImage size:NSMakeSize(size, size)];
    CGImageRelease(cgImage);
    [nsImage setTemplate:YES];
    return nsImage;
}

QString itemLabel(const QAction *action)
{
    return action->text().isEmpty() ? action->toolTip() : action->text();
}

NSColor *toNSColor(const QColor &color)
{
    return [NSColor colorWithSRGBRed:color.redF() green:color.greenF() blue:color.blueF() alpha:color.alphaF()];
}

} // namespace

// Traffic lights stay native (never drawn by us). The header parts paint the
// themed sidebar and toolbar surfaces under a transparent title bar, and
// native NSToolbar items (Liquid Glass on macOS 26) sit on top of them, bound
// to the header model.
class MacWindowFrame : public WindowFrame
{
public:
    MacWindowFrame(QMainWindow *window, LibraryHeader *header)
        : WindowFrame(window, header), model(header->model())
    {
        window->setWindowFlag(Qt::ExpandedClientAreaHint, true);
        window->setWindowFlag(Qt::NoTitleBarBackgroundHint, true);
        window->setAttribute(Qt::WA_ContentsMarginsRespectsSafeArea, false);

        NSWindow *nsWindow = this->nsWindow();
        nsWindow.titleVisibility = NSWindowTitleHidden;

        items = [NSMutableDictionary dictionary];
        delegate = [[YRLibraryHeaderToolbarDelegate alloc] init];
        delegate->frame = this;
        toolbar = [[NSToolbar alloc] initWithIdentifier:@"YRLibraryHeader"];
        toolbar.displayMode = NSToolbarDisplayModeIconOnly;
        toolbar.allowsUserCustomization = NO;
        toolbar.delegate = delegate;
        nsWindow.toolbar = toolbar;
        nsWindow.toolbarStyle = NSWindowToolbarStyleUnified;

        header->setControlsVisible(false);

        // The Qt header has no controls of its own here, so all of it drags.
        NSView *qtView = (__bridge NSView *)reinterpret_cast<void *>(window->winId());
        dragView = [[YRHeaderDragView alloc] initWithFrame:NSMakeRect(0, 0, NSWidth(qtView.bounds), LibraryHeader::Height)];
        dragView.autoresizingMask = NSViewWidthSizable | (qtView.isFlipped ? NSViewMaxYMargin : NSViewMinYMargin);
        [qtView addSubview:dragView];

        header->sidebarPart()->installEventFilter(this);
        header->contentPart()->installEventFilter(this);

        connect(window->windowHandle(), &QWindow::safeAreaMarginsChanged, this, [this] { updateMetrics(); });

        const HeaderActions &actions = model->actions();
        for (QAction *action : { actions.settings, actions.server, actions.help, actions.back, actions.forward }) {
            if (action != nullptr)
                connect(action, &QAction::changed, this, [this, action] { updateActionItem(action); });
        }
        for (QAction *action : { actions.gridView, actions.flowView, actions.infoView }) {
            connect(action, &QAction::toggled, this, [this] { syncViewMode(); });
            connect(action, &QAction::changed, this, [this] { updateViewModeItem(); });
        }
        connect(model, &HeaderModel::enabledChanged, this, [this] { updateEnabledState(); });
        connect(model, &HeaderModel::folderNameChanged, this, [this](const QString &name) {
            if (folderLabel != nil)
                folderLabel.stringValue = name.toNSString();
        });
        connect(model, &HeaderModel::searchTextSet, this, [this](const QString &text) {
            if (searchItem != nil)
                searchItem.searchField.stringValue = text.toNSString();
        });
        connect(model, &HeaderModel::searchEnabledChanged, this, [this] { updateEnabledState(); });
        connect(model, &HeaderModel::searchMenuChanged, this, [this] { updateSearchMenu(); });
        connect(model, &HeaderModel::searchFocusRequested, this, [this] {
            if (searchItem != nil && searchItem.searchField.enabled)
                [searchItem beginSearchInteraction];
        });

        // AppKit moves the traffic lights out of the window right after the
        // "will" notifications, about half a second before the "did" ones.
        NSNotificationCenter *center = NSNotificationCenter.defaultCenter;
        observers = [NSMutableArray array];
        [observers addObject:[center addObserverForName:NSWindowWillEnterFullScreenNotification
                                                 object:nsWindow
                                                  queue:nil
                                             usingBlock:^(NSNotification *) {
                                                 setTrafficLightsHidden(true);
                                             }]];
        [observers addObject:[center addObserverForName:NSWindowWillExitFullScreenNotification
                                                 object:nsWindow
                                                  queue:nil
                                             usingBlock:^(NSNotification *) {
                                                 setTrafficLightsHidden(false);
                                             }]];
        for (NSNotificationName name in @[ NSWindowDidEnterFullScreenNotification, NSWindowDidExitFullScreenNotification ]) {
            [observers addObject:[center addObserverForName:name
                                                     object:nsWindow
                                                      queue:nil
                                                 usingBlock:^(NSNotification *) {
                                                     updateMetrics();
                                                 }]];
        }
        // In full screen the toolbar has its own window, which moves when the
        // menu bar is revealed; the traffic lights come back with it.
        [observers addObject:[center addObserverForName:NSWindowDidMoveNotification
                                                 object:nil
                                                  queue:nil
                                             usingBlock:^(NSNotification *notification) {
                                                 if (trafficLightsHidden && sidebarEnd != nil && notification.object == sidebarEnd.window && notification.object != this->nsWindow())
                                                     scheduleAlignment();
                                             }]];

        QTimer::singleShot(0, this, [this] { updateMetrics(); });
    }

    ~MacWindowFrame() override
    {
        for (id observer in observers)
            [NSNotificationCenter.defaultCenter removeObserver:observer];
        delegate->frame = nullptr;
        [dragView removeFromSuperview];
        if (searchItem != nil)
            searchItem.searchField.delegate = nil;
        toolbar.delegate = nil;
    }

    void setColors(const FrameColors &colors) override
    {
        NSWindow *nsWindow = this->nsWindow();
        const bool dark = colors.headerBackground.lightnessF() < 0.5;
        nsWindow.appearance = [NSAppearance appearanceNamed:dark ? NSAppearanceNameDarkAqua : NSAppearanceNameAqua];
        nsWindow.backgroundColor = toNSColor(colors.contentBackground);
    }

    // The search is on the trailing edge, where macOS apps put it. The sidebar
    // end spacer splits the sidebar and navigation items into two Liquid Glass
    // groups.
    NSArray<NSToolbarItemIdentifier> *itemIdentifiers() const
    {
        NSMutableArray<NSToolbarItemIdentifier> *identifiers = [NSMutableArray arrayWithObject:SidebarLeadId];
        const HeaderActions &actions = model->actions();
        if (actions.settings != nullptr)
            [identifiers addObject:SettingsId];
        if (actions.server != nullptr)
            [identifiers addObject:ServerId];
        if (actions.help != nullptr)
            [identifiers addObject:HelpId];
        [identifiers addObjectsFromArray:@[ SidebarEndId, BackId, ForwardId, FolderId,
                                            NSToolbarFlexibleSpaceItemIdentifier, ViewModeId, SearchId ]];
        return identifiers;
    }

    NSToolbarItem *makeItem(NSString *identifier)
    {
        NSToolbarItem *item = createItem(identifier);
        if (item != nil)
            items[identifier] = item;
        return item;
    }

    void triggerAction(NSString *identifier)
    {
        QAction *action = actionFor(identifier);
        if (action != nullptr && action->isEnabled() && model->isEnabled())
            action->trigger();
    }

    bool isItemEnabled(NSString *identifier) const
    {
        if (!model->isEnabled())
            return false;
        if ([identifier isEqualToString:ViewModeId])
            return model->actions().gridView->isEnabled();
        const QAction *action = actionFor(identifier);
        return action == nullptr || action->isEnabled();
    }

    void viewModeSelected()
    {
        QAction *action = viewActions().value(viewGroup.selectedIndex);
        if (action != nullptr && action->isEnabled())
            action->trigger();
        syncViewMode();
    }

    void searchEdited(const QString &text)
    {
        model->editSearch(text);
    }

private:
    NSWindow *nsWindow() const
    {
        NSView *view = (__bridge NSView *)reinterpret_cast<void *>(window->winId());
        return view.window;
    }

    QList<QAction *> viewActions() const
    {
        const HeaderActions &actions = model->actions();
        return { actions.gridView, actions.flowView, actions.infoView };
    }

    QAction *actionFor(NSString *identifier) const
    {
        const HeaderActions &actions = model->actions();
        if ([identifier isEqualToString:SettingsId])
            return actions.settings;
        if ([identifier isEqualToString:ServerId])
            return actions.server;
        if ([identifier isEqualToString:HelpId])
            return actions.help;
        if ([identifier isEqualToString:BackId])
            return actions.back;
        if ([identifier isEqualToString:ForwardId])
            return actions.forward;
        return nullptr;
    }

    NSString *identifierFor(const QAction *action) const
    {
        for (NSString *identifier in @[ SettingsId, ServerId, HelpId, BackId, ForwardId ]) {
            if (actionFor(identifier) == action)
                return identifier;
        }
        return nil;
    }

    NSToolbarItem *createItem(NSString *identifier)
    {
        if ([identifier isEqualToString:SidebarLeadId] || [identifier isEqualToString:SidebarEndId])
            return makeSpacerItem(identifier);
        if ([identifier isEqualToString:FolderId])
            return makeFolderItem();
        if ([identifier isEqualToString:SearchId])
            return makeSearchItem();
        if ([identifier isEqualToString:ViewModeId])
            return makeViewModeItem();

        QAction *action = actionFor(identifier);
        if (action == nullptr)
            return nil;
        NSToolbarItem *item = [[NSToolbarItem alloc] initWithItemIdentifier:identifier];
        item.bordered = YES;
        item.target = delegate;
        item.action = @selector(itemClicked:);
        fillActionItem(item, action);
        return item;
    }

    // The lead spacer grows so settings, server and help end in line with the
    // sidebar title buttons; see alignWithSidebar(). Spacers are unbordered so
    // they stay out of the glass background of the neighbouring items.
    NSToolbarItem *makeSpacerItem(NSString *identifier)
    {
        const bool lead = [identifier isEqualToString:SidebarLeadId];
        NSToolbarItem *item = [[NSToolbarItem alloc] initWithItemIdentifier:identifier];
        item.bordered = NO;
        NSView *view = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 1, 28)];
        view.translatesAutoresizingMaskIntoConstraints = NO;
        NSLayoutConstraint *width = [view.widthAnchor constraintEqualToConstant:lead ? 1 : NavigationGap];
        width.active = YES;
        [view.heightAnchor constraintEqualToConstant:28].active = YES;
        item.view = view;
        if (lead) {
            sidebarLeadWidth = width;
        } else {
            sidebarEnd = view;
            sidebarEndWidth = width;
        }
        return item;
    }

    // The label is inset so it doesn't sit against the navigation glass.
    NSToolbarItem *makeFolderItem()
    {
        NSToolbarItem *item = [[NSToolbarItem alloc] initWithItemIdentifier:FolderId];
        item.bordered = NO;
        folderLabel = [NSTextField labelWithString:model->folderName().toNSString()];
        folderLabel.font = [NSFont boldSystemFontOfSize:13];
        folderLabel.lineBreakMode = NSLineBreakByTruncatingTail;
        folderLabel.translatesAutoresizingMaskIntoConstraints = NO;
        [folderLabel setContentCompressionResistancePriority:NSLayoutPriorityDefaultLow
                                              forOrientation:NSLayoutConstraintOrientationHorizontal];

        NSView *view = [[NSView alloc] initWithFrame:NSZeroRect];
        [view addSubview:folderLabel];
        [NSLayoutConstraint activateConstraints:@[
            [folderLabel.leadingAnchor constraintEqualToAnchor:view.leadingAnchor
                                                      constant:FolderLabelInset],
            [folderLabel.trailingAnchor constraintEqualToAnchor:view.trailingAnchor],
            [folderLabel.centerYAnchor constraintEqualToAnchor:view.centerYAnchor],
            [view.heightAnchor constraintEqualToConstant:28],
            [folderLabel.widthAnchor constraintLessThanOrEqualToConstant:280],
        ]];
        item.view = view;
        return item;
    }

    NSToolbarItem *makeSearchItem()
    {
        searchItem = [[NSSearchToolbarItem alloc] initWithItemIdentifier:SearchId];
        searchItem.preferredWidthForSearchField = 240;
        searchItem.resignsFirstResponderWithCancel = YES;
        searchItem.searchField.delegate = delegate;
        searchItem.searchField.stringValue = model->searchText().toNSString();
        updateSearchMenu();
        updateEnabledState();
        return searchItem;
    }

    NSToolbarItem *makeViewModeItem()
    {
        NSMutableArray<NSImage *> *images = [NSMutableArray array];
        NSMutableArray<NSString *> *labels = [NSMutableArray array];
        for (QAction *action : viewActions()) {
            NSImage *image = templateImage(action->icon(), ViewIconSize);
            [images addObject:image != nil ? image : [[NSImage alloc] initWithSize:NSMakeSize(ViewIconSize, ViewIconSize)]];
            [labels addObject:action->text().toNSString()];
        }
        viewGroup = [NSToolbarItemGroup groupWithItemIdentifier:ViewModeId
                                                         images:images
                                                  selectionMode:NSToolbarItemGroupSelectionModeSelectOne
                                                         labels:labels
                                                         target:delegate
                                                         action:@selector(viewModeChanged:)];
        viewGroup.controlRepresentation = NSToolbarItemGroupControlRepresentationExpanded;
        viewGroup.label = tr("View").toNSString();
        syncViewMode();
        return viewGroup;
    }

    void fillActionItem(NSToolbarItem *item, const QAction *action)
    {
        item.image = templateImage(action->icon(), ItemIconSize);
        item.label = itemLabel(action).toNSString();
        item.paletteLabel = item.label;
        item.toolTip = action->toolTip().toNSString();
        item.enabled = action->isEnabled() && model->isEnabled();
    }

    void updateActionItem(const QAction *action)
    {
        NSString *identifier = identifierFor(action);
        NSToolbarItem *item = identifier != nil ? items[identifier] : nil;
        if (item != nil)
            fillActionItem(item, action);
    }

    void updateViewModeItem()
    {
        if (viewGroup == nil)
            return;
        const QList<QAction *> actions = viewActions();
        NSArray<NSToolbarItem *> *subitems = viewGroup.subitems;
        for (int i = 0; i < actions.size() && i < int(subitems.count); ++i) {
            NSImage *image = templateImage(actions.at(i)->icon(), ViewIconSize);
            if (image != nil)
                subitems[i].image = image;
        }
        viewGroup.enabled = isItemEnabled(ViewModeId);
    }

    void syncViewMode()
    {
        if (viewGroup == nil)
            return;
        const QList<QAction *> actions = viewActions();
        for (int i = 0; i < actions.size(); ++i) {
            if (actions.at(i)->isChecked())
                viewGroup.selectedIndex = i;
        }
    }

    void updateEnabledState()
    {
        [toolbar validateVisibleItems];
        if (searchItem != nil)
            searchItem.searchField.enabled = model->isEnabled() && model->isSearchEnabled();
        if (viewGroup != nil)
            viewGroup.enabled = isItemEnabled(ViewModeId);
    }

    // The native field keeps a copy of the menu; its items trigger the
    // QActions of the model menu.
    void updateSearchMenu()
    {
        QMenu *menu = model->searchMenu();
        if (searchItem == nil || menu == nullptr)
            return;
        NSMenu *nativeMenu = [[NSMenu alloc] initWithTitle:@""];
        for (QAction *action : menu->actions()) {
            if (action->isSeparator()) {
                [nativeMenu addItem:NSMenuItem.separatorItem];
                continue;
            }
            NSMenuItem *item = [[NSMenuItem alloc] initWithTitle:action->text().remove(QLatin1Char('&')).toNSString()
                                                          action:@selector(searchMenuItemClicked:)
                                                   keyEquivalent:@""];
            item.target = delegate;
            item.representedObject = [NSValue valueWithPointer:action];
            [nativeMenu addItem:item];
        }
        NSSearchFieldCell *cell = searchItem.searchField.cell;
        cell.searchMenuTemplate = nativeMenu;
    }

    void updateMetrics()
    {
        const QMargins safe = window->windowHandle()->safeAreaMargins();
        const int height = safe.top() > 0 ? safe.top() : LibraryHeader::Height;
        header->setHeight(height);

        NSView *qtView = dragView.superview;
        dragView.frame = NSMakeRect(0, qtView.isFlipped ? 0 : NSHeight(qtView.bounds) - height, NSWidth(qtView.bounds), height);

        int leading = 0;
        NSButton *zoom = [nsWindow() standardWindowButton:NSWindowZoomButton];
        if (!trafficLightsHidden && zoom != nil && !zoom.hidden && zoom.window == nsWindow()) {
            const NSRect frame = [zoom convertRect:zoom.bounds toView:nil];
            leading = qCeil(NSMaxX(frame));
        }
        header->setLeadingInset(leading);
        scheduleAlignment();
    }

    // On show, the items are aligned before the window draws its first frame,
    // so they don't jump into place after launch.
    bool eventFilter(QObject *watched, QEvent *event) override
    {
        if (event->type() == QEvent::Show)
            alignWithSidebar();
        else if (event->type() == QEvent::Resize || event->type() == QEvent::Move)
            scheduleAlignment();
        return WindowFrame::eventFilter(watched, event);
    }

    void scheduleAlignment()
    {
        if (alignmentPending)
            return;
        alignmentPending = true;
        QTimer::singleShot(0, this, [this] { alignWithSidebar(); });
    }

    // Moves the native items so the sidebar end spacer starts SidebarGroupInset
    // before the sidebar edge, which lines up settings, server and help with
    // the sidebar and navigation with the content, following the splitter.
    // Forcing the window layout applies each change right away, so all the
    // passes run at once, before the window draws again.
    void alignWithSidebar()
    {
        alignmentPending = false;
        if (sidebarEnd == nil || sidebarLeadWidth == nil || sidebarEnd.window == nil)
            return;
        if (trafficLightsHidden) {
            alignWithHiddenTrafficLights();
            return;
        }
        sidebarEndWidth.constant = NavigationGap;
        for (int pass = 0; pass < MaxAlignmentPasses && alignmentStep(); ++pass) { }
    }

    void setTrafficLightsHidden(bool hidden)
    {
        trafficLightsHidden = hidden;
        // The lead spacer has a minimum slot, so it must leave the toolbar for
        // the items to reach the leading edge.
        const bool hasLead = [toolbar.items.firstObject.itemIdentifier isEqualToString:SidebarLeadId];
        if (hidden && hasLead)
            [toolbar removeItemAtIndex:0];
        else if (!hidden && !hasLead)
            [toolbar insertItemWithItemIdentifier:SidebarLeadId atIndex:0];
        lastLead = 0;
        lastEndX = 0;
        leftmostEndX = 0;
        hiddenLeftmostEndOffset = -1;
        updateMetrics();
    }

    // Like native apps, settings, server and help sit at the leading edge and
    // follow the traffic lights when the menu bar reveals them; the sidebar
    // end spacer takes up that move, so navigation stays over the content.
    // Screen coordinates, because the toolbar has its own window here.
    void alignWithHiddenTrafficLights()
    {
        [sidebarEnd.window layoutIfNeeded];
        const NSRect end = [sidebarEnd.window convertRectToScreen:[sidebarEnd convertRect:sidebarEnd.bounds toView:nil]];
        QWidget *sidebarPart = header->sidebarPart();
        const CGFloat endOffset = NSMinX(end) - sidebarPart->mapToGlobal(QPoint(0, 0)).x();
        if (hiddenLeftmostEndOffset < 0 || endOffset < hiddenLeftmostEndOffset)
            hiddenLeftmostEndOffset = endOffset;
        sidebarPart->setMinimumWidth(qCeil(hiddenLeftmostEndOffset + SidebarGroupInset));

        sidebarEndWidth.constant = std::max(NavigationGap, sidebarPart->width() - SidebarGroupInset + NavigationGap - endOffset);
    }

    // One pass of alignWithSidebar(); returns whether it moved the items.
    // AppKit gives the lead spacer a minimum slot (about 36pt on macOS 26), so
    // below that, changing its width doesn't move anything. When a change has
    // no effect the items are at their leftmost position: that is where the
    // sidebar edge can't go past, and the spacer keeps growing until it moves
    // them again.
    bool alignmentStep()
    {
        [sidebarEnd.window layoutIfNeeded];
        const CGFloat endX = NSMinX([sidebarEnd convertRect:sidebarEnd.bounds toView:nil]);
        const CGFloat lead = sidebarLeadWidth.constant;
        const bool stuck = lead != lastLead && lastLead > 0 && std::abs(endX - lastEndX) < 0.5;
        if (stuck)
            leftmostEndX = endX;
        lastLead = lead;
        lastEndX = endX;

        QWidget *sidebarPart = header->sidebarPart();
        const int sidebarLeft = sidebarPart->mapTo(sidebarPart->window(), QPoint(0, 0)).x();
        const CGFloat minEndX = std::max(leftmostEndX, endX - (lead - 1));
        sidebarPart->setMinimumWidth(qCeil(minEndX + SidebarGroupInset) - sidebarLeft);

        const CGFloat targetX = header->sidebarRightEdge() - SidebarGroupInset;
        const CGFloat delta = targetX - endX;
        if (std::abs(delta) < 0.5)
            return false;
        // Can't move further left; the minimum width widens the sidebar.
        if (delta < 0 && endX <= leftmostEndX + 0.5)
            return false;
        // Through the dead zone the spacer doubles, up to the sidebar width.
        if (stuck && delta > 0) {
            if (lead > targetX)
                return false;
            sidebarLeadWidth.constant = lead + std::max(delta, lead);
        } else {
            sidebarLeadWidth.constant = std::max<CGFloat>(1, lead + delta);
        }
        return true;
    }

    HeaderModel *model;
    YRLibraryHeaderToolbarDelegate *delegate = nil;
    NSToolbar *toolbar = nil;
    NSMutableDictionary<NSString *, NSToolbarItem *> *items = nil;
    NSToolbarItemGroup *viewGroup = nil;
    NSSearchToolbarItem *searchItem = nil;
    NSTextField *folderLabel = nil;
    YRHeaderDragView *dragView = nil;
    NSView *sidebarEnd = nil;
    NSLayoutConstraint *sidebarLeadWidth = nil;
    NSLayoutConstraint *sidebarEndWidth = nil;
    NSMutableArray *observers = nil;
    bool alignmentPending = false;
    bool trafficLightsHidden = false;
    CGFloat lastLead = 0;
    CGFloat lastEndX = 0;
    CGFloat leftmostEndX = 0;
    CGFloat hiddenLeftmostEndOffset = -1;
};

@implementation YRLibraryHeaderToolbarDelegate

- (NSArray<NSToolbarItemIdentifier> *)toolbarDefaultItemIdentifiers:(NSToolbar *)toolbar
{
    Q_UNUSED(toolbar);
    return frame != nullptr ? frame->itemIdentifiers() : @[];
}

- (NSArray<NSToolbarItemIdentifier> *)toolbarAllowedItemIdentifiers:(NSToolbar *)toolbar
{
    Q_UNUSED(toolbar);
    return frame != nullptr ? frame->itemIdentifiers() : @[];
}

- (NSToolbarItem *)toolbar:(NSToolbar *)toolbar itemForItemIdentifier:(NSToolbarItemIdentifier)itemIdentifier willBeInsertedIntoToolbar:(BOOL)flag
{
    Q_UNUSED(toolbar);
    Q_UNUSED(flag);
    return frame != nullptr ? frame->makeItem(itemIdentifier) : nil;
}

- (void)itemClicked:(NSToolbarItem *)sender
{
    if (frame != nullptr)
        frame->triggerAction(sender.itemIdentifier);
}

- (void)viewModeChanged:(id)sender
{
    Q_UNUSED(sender);
    if (frame != nullptr)
        frame->viewModeSelected();
}

- (BOOL)validateToolbarItem:(NSToolbarItem *)item
{
    return frame != nullptr && frame->isItemEnabled(item.itemIdentifier);
}

- (void)controlTextDidChange:(NSNotification *)notification
{
    NSSearchField *field = notification.object;
    if (frame != nullptr)
        frame->searchEdited(QString::fromNSString(field.stringValue));
}

- (void)searchMenuItemClicked:(NSMenuItem *)sender
{
    NSValue *value = sender.representedObject;
    auto *action = static_cast<QAction *>(value.pointerValue);
    if (frame != nullptr && action != nullptr)
        action->trigger();
}

@end

WindowFrame *createPlatformWindowFrame(QMainWindow *window, LibraryHeader *header)
{
    return new MacWindowFrame(window, header);
}
