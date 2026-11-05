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
    nsImage.template = YES;
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

        QTimer::singleShot(0, this, [this] { updateMetrics(); });
    }

    ~MacWindowFrame() override
    {
        delegate->frame = nullptr;
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

    // The search is on the trailing edge, where macOS apps put it.
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

    // The lead spacer grows so settings, server and help end at the sidebar
    // edge; see alignWithSidebar().
    NSToolbarItem *makeSpacerItem(NSString *identifier)
    {
        const bool lead = [identifier isEqualToString:SidebarLeadId];
        NSToolbarItem *item = [[NSToolbarItem alloc] initWithItemIdentifier:identifier];
        NSView *view = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 1, 28)];
        view.translatesAutoresizingMaskIntoConstraints = NO;
        NSLayoutConstraint *width = [view.widthAnchor constraintEqualToConstant:lead ? 1 : 4];
        width.active = YES;
        [view.heightAnchor constraintEqualToConstant:28].active = YES;
        item.view = view;
        if (lead)
            sidebarLeadWidth = width;
        else
            sidebarEnd = view;
        return item;
    }

    NSToolbarItem *makeFolderItem()
    {
        NSToolbarItem *item = [[NSToolbarItem alloc] initWithItemIdentifier:FolderId];
        folderLabel = [NSTextField labelWithString:model->folderName().toNSString()];
        folderLabel.font = [NSFont boldSystemFontOfSize:13];
        folderLabel.lineBreakMode = NSLineBreakByTruncatingTail;
        [folderLabel setContentCompressionResistancePriority:NSLayoutPriorityDefaultLow
                                              forOrientation:NSLayoutConstraintOrientationHorizontal];
        [folderLabel.widthAnchor constraintLessThanOrEqualToConstant:280].active = YES;
        item.view = folderLabel;
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
        header->setHeight(safe.top() > 0 ? safe.top() : LibraryHeader::Height);

        int leading = 0;
        NSButton *zoom = [nsWindow() standardWindowButton:NSWindowZoomButton];
        if (zoom != nil && !zoom.hidden && zoom.window != nil) {
            const NSRect frame = [zoom convertRect:zoom.bounds toView:nil];
            leading = qCeil(NSMaxX(frame));
        }
        header->setLeadingInset(leading);
        scheduleAlignment();
    }

    bool eventFilter(QObject *watched, QEvent *event) override
    {
        if (event->type() == QEvent::Resize || event->type() == QEvent::Move || event->type() == QEvent::Show)
            scheduleAlignment();
        return WindowFrame::eventFilter(watched, event);
    }

    void scheduleAlignment()
    {
        if (alignmentPending)
            return;
        alignmentPending = true;
        alignmentPasses = 0;
        QTimer::singleShot(0, this, [this] { alignWithSidebar(); });
    }

    // Moves the native items so settings, server and help end at the sidebar
    // edge and navigation starts over the content, following the splitter.
    void alignWithSidebar()
    {
        alignmentPending = false;
        if (sidebarEnd == nil || sidebarLeadWidth == nil || sidebarEnd.window == nil)
            return;
        [sidebarEnd.window layoutIfNeeded];
        const NSRect end = [sidebarEnd convertRect:sidebarEnd.bounds toView:nil];
        const CGFloat delta = header->sidebarRightEdge() - NSMaxX(end);
        if (std::abs(delta) < 0.5 || ++alignmentPasses > 3)
            return;
        sidebarLeadWidth.constant = std::max<CGFloat>(1, sidebarLeadWidth.constant + delta);
        QTimer::singleShot(0, this, [this] { alignWithSidebar(); });
    }

    HeaderModel *model;
    YRLibraryHeaderToolbarDelegate *delegate = nil;
    NSToolbar *toolbar = nil;
    NSMutableDictionary<NSString *, NSToolbarItem *> *items = nil;
    NSToolbarItemGroup *viewGroup = nil;
    NSSearchToolbarItem *searchItem = nil;
    NSTextField *folderLabel = nil;
    NSView *sidebarEnd = nil;
    NSLayoutConstraint *sidebarLeadWidth = nil;
    bool alignmentPending = false;
    int alignmentPasses = 0;
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
