#include "theme_factory.h"

#include <QApplication>

#include "icon_utils.h"

struct ToolbarParams {
    ToolbarThemeTemplates t;

    QColor iconColor;
    QColor iconDisabledColor;
    QColor iconCheckedColor;
    QColor backgroundColor;
    QColor separatorColor;
    QColor checkedButtonColor;
    QColor menuIndicatorColor;
};

struct ViewerParams {
    ViewerThemeTemplates t;

    QColor defaultBackgroundColor;
    QColor defaultTextColor;
    QColor infoBackgroundColor;
    QColor infoTextColor;
};

struct GoToFlowWidgetParams {
    GoToFlowWidgetThemeTemplates t;

    QColor flowBackgroundColor;
    QColor flowTextColor;
    QColor toolbarBackgroundColor;
    QColor sliderBorderColor;
    QColor sliderGrooveColor;
    QColor sliderHandleColor;
    QColor editBorderColor;
    QColor editBackgroundColor;
    QColor editTextColor;
    QColor labelTextColor;
    QColor iconColor;
};

struct WhatsNewDialogParams {
    QColor backgroundColor;
    QColor headerTextColor;
    QColor versionTextColor;
    QColor contentTextColor;
    QColor linkColor;
    QColor closeButtonColor;
    QColor headerDecorationColor;
};

struct ShortcutsIconsParams {
    QColor iconColor; // Main icon color (replaces #f0f)
};

struct ThemeParams {
    QString themeName;

    ToolbarParams toolbarParams;
    ViewerParams viewerParams;
    GoToFlowWidgetParams goToFlowWidgetParams;
    HelpAboutDialogTheme helpAboutDialogParams;
    WhatsNewDialogParams whatsNewDialogParams;
    ShortcutsIconsParams shortcutsIconsParams;
};

void setToolbarIconPair(QIcon &icon,
                        QIcon &icon18,
                        const QString &basePath,
                        const QColor &iconColor,
                        const QColor &disabledColor,
                        const QColor &checkedColor,
                        const QString &themeName)
{
    QString path18 = basePath;
    if (path18.endsWith(".svg"))
        path18.insert(path18.size() - 4, "_18x18");
    else
        path18.append("_18x18");

    // Normal
    const QString normalPath = recoloredSvgToThemeFile(basePath, iconColor, themeName);
    const QString normalPath18 = recoloredSvgToThemeFile(path18, iconColor, themeName);
    // Disabled
    const QString disabledPath = recoloredSvgToThemeFile(basePath, disabledColor, themeName, { .suffix = "_disabled" });
    const QString disabledPath18 = recoloredSvgToThemeFile(path18, disabledColor, themeName, { .suffix = "_disabled" });
    // Checked (On state)
    const QString checkedPath = recoloredSvgToThemeFile(basePath, checkedColor, themeName, { .suffix = "_checked" });
    const QString checkedPath18 = recoloredSvgToThemeFile(path18, checkedColor, themeName, { .suffix = "_checked" });

    icon.addFile(normalPath, QSize(), QIcon::Normal, QIcon::Off);
    icon.addFile(disabledPath, QSize(), QIcon::Disabled, QIcon::Off);
    icon.addFile(checkedPath, QSize(), QIcon::Normal, QIcon::On);
    icon.addFile(disabledPath, QSize(), QIcon::Disabled, QIcon::On);

    icon18.addFile(normalPath18, QSize(), QIcon::Normal, QIcon::Off);
    icon18.addFile(disabledPath18, QSize(), QIcon::Disabled, QIcon::Off);
    icon18.addFile(checkedPath18, QSize(), QIcon::Normal, QIcon::On);
    icon18.addFile(disabledPath18, QSize(), QIcon::Disabled, QIcon::On);
}

Theme makeTheme(const ThemeParams &params)
{
    Theme theme;

    // Toolbar & actions
    theme.toolbar.toolbarQSS = params.toolbarParams.t.toolbarQSS.arg(params.toolbarParams.backgroundColor.name(), params.toolbarParams.separatorColor.name(), params.toolbarParams.checkedButtonColor.name(), recoloredSvgToThemeFile(params.toolbarParams.t.menuArrowPath, params.toolbarParams.menuIndicatorColor, params.themeName));

    auto setToolbarIconPairT = [&](QIcon &icon, QIcon &icon18, const QString &basePath) {
        setToolbarIconPair(icon, icon18, basePath, params.toolbarParams.iconColor, params.toolbarParams.iconDisabledColor, params.toolbarParams.iconCheckedColor, params.themeName);
    };

    setToolbarIconPairT(theme.toolbar.openAction, theme.toolbar.openAction18x18, ":/images/viewer_toolbar/open.svg");
    setToolbarIconPairT(theme.toolbar.openFolderAction, theme.toolbar.openFolderAction18x18, ":/images/viewer_toolbar/openFolder.svg");
    setToolbarIconPairT(theme.toolbar.openLatestComicAction, theme.toolbar.openLatestComicAction18x18, ":/images/viewer_toolbar/openNext.svg");
    setToolbarIconPairT(theme.toolbar.saveImageAction, theme.toolbar.saveImageAction18x18, ":/images/viewer_toolbar/save.svg");
    setToolbarIconPairT(theme.toolbar.openComicOnTheLeftAction, theme.toolbar.openComicOnTheLeftAction18x18, ":/images/viewer_toolbar/openPrevious.svg");
    setToolbarIconPairT(theme.toolbar.openComicOnTheRightAction, theme.toolbar.openComicOnTheRightAction18x18, ":/images/viewer_toolbar/openNext.svg");
    setToolbarIconPairT(theme.toolbar.goToPageOnTheLeftAction, theme.toolbar.goToPageOnTheLeftAction18x18, ":/images/viewer_toolbar/previous.svg");
    setToolbarIconPairT(theme.toolbar.goToPageOnTheRightAction, theme.toolbar.goToPageOnTheRightAction18x18, ":/images/viewer_toolbar/next.svg");
    setToolbarIconPairT(theme.toolbar.adjustHeightAction, theme.toolbar.adjustHeightAction18x18, ":/images/viewer_toolbar/toHeight.svg");
    setToolbarIconPairT(theme.toolbar.adjustWidthAction, theme.toolbar.adjustWidthAction18x18, ":/images/viewer_toolbar/toWidth.svg");
    setToolbarIconPairT(theme.toolbar.leftRotationAction, theme.toolbar.leftRotationAction18x18, ":/images/viewer_toolbar/rotateL.svg");
    setToolbarIconPairT(theme.toolbar.rightRotationAction, theme.toolbar.rightRotationAction18x18, ":/images/viewer_toolbar/rotateR.svg");
    setToolbarIconPairT(theme.toolbar.doublePageAction, theme.toolbar.doublePageAction18x18, ":/images/viewer_toolbar/doublePage.svg");
    setToolbarIconPairT(theme.toolbar.doubleMangaPageAction, theme.toolbar.doubleMangaPageAction18x18, ":/images/viewer_toolbar/doubleMangaPage.svg");
    setToolbarIconPairT(theme.toolbar.showZoomSliderlAction, theme.toolbar.showZoomSliderlAction18x18, ":/images/viewer_toolbar/zoom.svg");
    setToolbarIconPairT(theme.toolbar.goToPageAction, theme.toolbar.goToPageAction18x18, ":/images/viewer_toolbar/goto.svg");
    setToolbarIconPairT(theme.toolbar.optionsAction, theme.toolbar.optionsAction18x18, ":/images/viewer_toolbar/options.svg");
    setToolbarIconPairT(theme.toolbar.helpAboutAction, theme.toolbar.helpAboutAction18x18, ":/images/viewer_toolbar/help.svg");
    setToolbarIconPairT(theme.toolbar.showMagnifyingGlassAction, theme.toolbar.showMagnifyingGlassAction18x18, ":/images/viewer_toolbar/magnifyingGlass.svg");
    setToolbarIconPairT(theme.toolbar.setBookmarkAction, theme.toolbar.setBookmarkAction18x18, ":/images/viewer_toolbar/bookmark.svg");
    setToolbarIconPairT(theme.toolbar.showBookmarksAction, theme.toolbar.showBookmarksAction18x18, ":/images/viewer_toolbar/showBookmarks.svg");
    setToolbarIconPairT(theme.toolbar.showShorcutsAction, theme.toolbar.showShorcutsAction18x18, ":/images/viewer_toolbar/shortcuts.svg");
    setToolbarIconPairT(theme.toolbar.showInfoAction, theme.toolbar.showInfoAction18x18, ":/images/viewer_toolbar/info.svg");
    setToolbarIconPairT(theme.toolbar.closeAction, theme.toolbar.closeAction18x18, ":/images/viewer_toolbar/close.svg");
    setToolbarIconPairT(theme.toolbar.showDictionaryAction, theme.toolbar.showDictionaryAction18x18, ":/images/viewer_toolbar/translator.svg");
    setToolbarIconPairT(theme.toolbar.adjustToFullSizeAction, theme.toolbar.adjustToFullSizeAction18x18, ":/images/viewer_toolbar/full.svg");
    setToolbarIconPairT(theme.toolbar.fitToPageAction, theme.toolbar.fitToPageAction18x18, ":/images/viewer_toolbar/fitToPage.svg");
    setToolbarIconPairT(theme.toolbar.showFlowAction, theme.toolbar.showFlowAction18x18, ":/images/viewer_toolbar/flow.svg");
    // end Toolbar & actions

    // Viewer
    theme.viewer.defaultBackgroundColor = params.viewerParams.defaultBackgroundColor;
    theme.viewer.defaultTextColor = params.viewerParams.defaultTextColor;
    theme.viewer.infoBackgroundColor = params.viewerParams.infoBackgroundColor;
    theme.viewer.infoLabelQSS = params.viewerParams.t.infoLabelQSS.arg(params.viewerParams.infoTextColor.name());
    // end Viewer

    // GoToFlowWidget
    auto &gotoParams = params.goToFlowWidgetParams;
    theme.goToFlowWidget.flowBackgroundColor = gotoParams.flowBackgroundColor;
    theme.goToFlowWidget.flowTextColor = gotoParams.flowTextColor;
    theme.goToFlowWidget.toolbarBackgroundColor = gotoParams.toolbarBackgroundColor;
    theme.goToFlowWidget.sliderQSS = gotoParams.t.sliderQSS.arg(
            gotoParams.sliderBorderColor.name(QColor::HexArgb),
            gotoParams.sliderGrooveColor.name(QColor::HexArgb),
            gotoParams.sliderHandleColor.name(QColor::HexArgb));
    theme.goToFlowWidget.editQSS = gotoParams.t.editQSS.arg(
            gotoParams.editBorderColor.name(QColor::HexArgb),
            gotoParams.editBackgroundColor.name(QColor::HexArgb),
            gotoParams.editTextColor.name());
    theme.goToFlowWidget.buttonQSS = gotoParams.t.buttonQSS;
    theme.goToFlowWidget.labelQSS = gotoParams.t.labelQSS.arg(gotoParams.labelTextColor.name());

    const QString centerIconPath = recoloredSvgToThemeFile(":/images/centerFlow.svg", gotoParams.iconColor, params.themeName);
    const QString goToIconPath = recoloredSvgToThemeFile(":/images/gotoFlow.svg", gotoParams.iconColor, params.themeName);
    theme.goToFlowWidget.centerIcon = QIcon(centerIconPath);
    theme.goToFlowWidget.goToIcon = QIcon(goToIconPath);
    // end GoToFlowWidget

    // HelpAboutDialog
    theme.helpAboutDialog = params.helpAboutDialogParams;
    // end HelpAboutDialog

    // WhatsNewDialog
    const auto &wn = params.whatsNewDialogParams;
    theme.whatsNewDialog.backgroundColor = wn.backgroundColor;
    theme.whatsNewDialog.headerTextColor = wn.headerTextColor;
    theme.whatsNewDialog.versionTextColor = wn.versionTextColor;
    theme.whatsNewDialog.contentTextColor = wn.contentTextColor;
    theme.whatsNewDialog.linkColor = wn.linkColor;
    theme.whatsNewDialog.closeButtonIcon = QPixmap(recoloredSvgToThemeFile(":/images/custom_dialog/custom_close_button.svg", wn.closeButtonColor, params.themeName));
    theme.whatsNewDialog.headerDecoration = QPixmap(recoloredSvgToThemeFile(":/images/whats_new/whatsnew_header.svg", wn.headerDecorationColor, params.themeName));
    // end WhatsNewDialog

    // ShortcutsIcons
    const auto &sci = params.shortcutsIconsParams;
    auto makeShortcutsIcon = [&](const QString &basePath) {
        const QString path = recoloredSvgToThemeFile(basePath, sci.iconColor, params.themeName);
        return QIcon(path);
    };

    theme.shortcutsIcons.comicsIcon = makeShortcutsIcon(":/images/shortcuts/shortcuts_group_comics.svg");
    theme.shortcutsIcons.generalIcon = makeShortcutsIcon(":/images/shortcuts/shortcuts_group_general.svg");
    theme.shortcutsIcons.magnifyingGlassIcon = makeShortcutsIcon(":/images/shortcuts/shortcuts_group_mglass.svg");
    theme.shortcutsIcons.pageIcon = makeShortcutsIcon(":/images/shortcuts/shortcuts_group_page.svg");
    theme.shortcutsIcons.readingIcon = makeShortcutsIcon(":/images/shortcuts/shortcuts_group_reading.svg");
    // end ShortcutsIcons

    // FindFolder icon (used in OptionsDialog)
    {
        const QString path = recoloredSvgToThemeFile(":/images/find_folder.svg", params.toolbarParams.iconColor, params.themeName);
        const qreal dpr = qApp->devicePixelRatio();
        theme.dialogIcons.findFolderIcon = QIcon(renderSvgToPixmap(path, 13, 13, dpr));
    }

    return theme;
}

ThemeParams classicThemeParams();
ThemeParams lightThemeParams();
ThemeParams darkThemeParams();

Theme makeTheme(ThemeId themeId)
{
    switch (themeId) {
    case ThemeId::Classic:
        return makeTheme(classicThemeParams());
    case ThemeId::Light:
        return makeTheme(lightThemeParams());
    case ThemeId::Dark:
        return makeTheme(darkThemeParams());
    }
}

ThemeParams classicThemeParams()
{
    ThemeParams params;
    params.themeName = "classic";

    ToolbarParams toolbarParams;
    toolbarParams.iconColor = QColor(0x404040);
    toolbarParams.iconDisabledColor = QColor(0x858585);
    toolbarParams.iconCheckedColor = QColor(0x5A5A5A);
    toolbarParams.backgroundColor = QColor(0xF3F3F3);
    toolbarParams.separatorColor = QColor(0xCCCCCC);
    toolbarParams.checkedButtonColor = QColor(0xCCCCCC);
    toolbarParams.menuIndicatorColor = QColor(0x404040);

    params.toolbarParams = toolbarParams;

    ViewerParams viewerParams;
    viewerParams.defaultBackgroundColor = QColor(0x282828);
    viewerParams.defaultTextColor = Qt::white;
    viewerParams.infoBackgroundColor = QColor::fromRgba(0xBB000000);
    viewerParams.infoTextColor = Qt::white;
    viewerParams.t = ViewerThemeTemplates();

    params.viewerParams = viewerParams;

    GoToFlowWidgetParams goToFlowWidgetParams;
    goToFlowWidgetParams.flowBackgroundColor = QColor(0x282828);
    goToFlowWidgetParams.flowTextColor = Qt::white;
    goToFlowWidgetParams.toolbarBackgroundColor = QColor::fromRgba(0x99000000);
    goToFlowWidgetParams.sliderBorderColor = QColor::fromRgba(0x22FFFFFF);
    goToFlowWidgetParams.sliderGrooveColor = QColor::fromRgba(0x77000000);
    goToFlowWidgetParams.sliderHandleColor = QColor::fromRgba(0x55FFFFFF);
    goToFlowWidgetParams.editBorderColor = QColor::fromRgba(0x77000000);
    goToFlowWidgetParams.editBackgroundColor = QColor::fromRgba(0x55000000);
    goToFlowWidgetParams.editTextColor = Qt::white;
    goToFlowWidgetParams.labelTextColor = Qt::white;
    goToFlowWidgetParams.iconColor = Qt::white;
    goToFlowWidgetParams.t = GoToFlowWidgetThemeTemplates();

    params.goToFlowWidgetParams = goToFlowWidgetParams;

    params.helpAboutDialogParams.headingColor = QColor(0x302f2d);
    params.helpAboutDialogParams.linkColor = QColor(0xC19441);

    params.whatsNewDialogParams.backgroundColor = QColor(0xFFFFFF);
    params.whatsNewDialogParams.headerTextColor = QColor(0x0A0A0A);
    params.whatsNewDialogParams.versionTextColor = QColor(0x858585);
    params.whatsNewDialogParams.contentTextColor = QColor(0x0A0A0A);
    params.whatsNewDialogParams.linkColor = QColor(0xE8B800);
    params.whatsNewDialogParams.closeButtonColor = QColor(0x444444);
    params.whatsNewDialogParams.headerDecorationColor = QColor(0xE8B800);

    // ShortcutsIcons
    ShortcutsIconsParams sci;
    sci.iconColor = QColor(0x404040); // Dark icons for light background
    params.shortcutsIconsParams = sci;

    return params;
}

ThemeParams lightThemeParams()
{
    ThemeParams params;
    params.themeName = "light";

    ToolbarParams toolbarParams;
    toolbarParams.iconColor = QColor(0x404040);
    toolbarParams.iconDisabledColor = QColor(0xB0B0B0);
    toolbarParams.iconCheckedColor = QColor(0x5A5A5A);
    toolbarParams.backgroundColor = QColor(0xF3F3F3);
    toolbarParams.separatorColor = QColor(0xCCCCCC);
    toolbarParams.checkedButtonColor = QColor(0xCCCCCC);
    toolbarParams.menuIndicatorColor = QColor(0x404040);

    params.toolbarParams = toolbarParams;

    ViewerParams viewerParams;
    viewerParams.defaultBackgroundColor = QColor(0xF6F6F6);
    viewerParams.defaultTextColor = QColor(0x202020);
    viewerParams.infoBackgroundColor = QColor::fromRgba(0xBBFFFFFF);
    viewerParams.infoTextColor = QColor(0x404040);
    viewerParams.t = ViewerThemeTemplates();

    params.viewerParams = viewerParams;

    GoToFlowWidgetParams goToFlowWidgetParams;
    goToFlowWidgetParams.flowBackgroundColor = QColor(0xF6F6F6);
    goToFlowWidgetParams.flowTextColor = QColor(0x202020);
    goToFlowWidgetParams.toolbarBackgroundColor = QColor::fromRgba(0xBBFFFFFF);
    goToFlowWidgetParams.sliderBorderColor = QColor::fromRgba(0x22000000);
    goToFlowWidgetParams.sliderGrooveColor = QColor::fromRgba(0x33000000);
    goToFlowWidgetParams.sliderHandleColor = QColor::fromRgba(0x55000000);
    goToFlowWidgetParams.editBorderColor = QColor::fromRgba(0x33000000);
    goToFlowWidgetParams.editBackgroundColor = QColor::fromRgba(0x22000000);
    goToFlowWidgetParams.editTextColor = QColor(0x202020);
    goToFlowWidgetParams.labelTextColor = QColor(0x202020);
    goToFlowWidgetParams.iconColor = QColor(0x404040);
    goToFlowWidgetParams.t = GoToFlowWidgetThemeTemplates();

    params.goToFlowWidgetParams = goToFlowWidgetParams;

    params.helpAboutDialogParams.headingColor = QColor(0x302f2d);
    params.helpAboutDialogParams.linkColor = QColor(0xC19441);

    params.whatsNewDialogParams.backgroundColor = QColor(0xFFFFFF);
    params.whatsNewDialogParams.headerTextColor = QColor(0x0A0A0A);
    params.whatsNewDialogParams.versionTextColor = QColor(0x858585);
    params.whatsNewDialogParams.contentTextColor = QColor(0x0A0A0A);
    params.whatsNewDialogParams.linkColor = QColor(0xE8B800);
    params.whatsNewDialogParams.closeButtonColor = QColor(0x444444);
    params.whatsNewDialogParams.headerDecorationColor = QColor(0xE8B800);

    // ShortcutsIcons
    ShortcutsIconsParams sci;
    sci.iconColor = QColor(0x606060); // Dark icons for light background
    params.shortcutsIconsParams = sci;

    return params;
}

ThemeParams darkThemeParams()
{
    ThemeParams params;
    params.themeName = "dark";

    ToolbarParams toolbarParams;
    toolbarParams.iconColor = QColor(0xCCCCCC);
    toolbarParams.iconDisabledColor = QColor(0x444444);
    toolbarParams.iconCheckedColor = QColor(0xDADADA);
    toolbarParams.backgroundColor = QColor(0x202020);
    toolbarParams.separatorColor = QColor(0x444444);
    toolbarParams.checkedButtonColor = QColor(0x3A3A3A);
    toolbarParams.menuIndicatorColor = QColor(0xCCCCCC);

    params.toolbarParams = toolbarParams;

    ViewerParams viewerParams;
    viewerParams.defaultBackgroundColor = QColor(40, 40, 40);
    viewerParams.defaultTextColor = Qt::white;
    viewerParams.infoBackgroundColor = QColor::fromRgba(0xBB000000);
    viewerParams.infoTextColor = QColor(0xB0B0B0);
    viewerParams.t = ViewerThemeTemplates();

    params.viewerParams = viewerParams;

    GoToFlowWidgetParams goToFlowWidgetParams;
    goToFlowWidgetParams.flowBackgroundColor = QColor(40, 40, 40);
    goToFlowWidgetParams.flowTextColor = Qt::white;
    goToFlowWidgetParams.toolbarBackgroundColor = QColor::fromRgba(0x99000000);
    goToFlowWidgetParams.sliderBorderColor = QColor::fromRgba(0x22FFFFFF);
    goToFlowWidgetParams.sliderGrooveColor = QColor::fromRgba(0x77000000);
    goToFlowWidgetParams.sliderHandleColor = QColor::fromRgba(0x55FFFFFF);
    goToFlowWidgetParams.editBorderColor = QColor::fromRgba(0x77000000);
    goToFlowWidgetParams.editBackgroundColor = QColor::fromRgba(0x55000000);
    goToFlowWidgetParams.editTextColor = Qt::white;
    goToFlowWidgetParams.labelTextColor = Qt::white;
    goToFlowWidgetParams.iconColor = QColor(0xCCCCCC);
    goToFlowWidgetParams.t = GoToFlowWidgetThemeTemplates();

    params.goToFlowWidgetParams = goToFlowWidgetParams;

    params.helpAboutDialogParams.headingColor = QColor(0xE0E0E0);
    params.helpAboutDialogParams.linkColor = QColor(0xD4A84B);

    params.whatsNewDialogParams.backgroundColor = QColor(0x2A2A2A);
    params.whatsNewDialogParams.headerTextColor = QColor(0xE0E0E0);
    params.whatsNewDialogParams.versionTextColor = QColor(0x858585);
    params.whatsNewDialogParams.contentTextColor = QColor(0xE0E0E0);
    params.whatsNewDialogParams.linkColor = QColor(0xE8B800);
    params.whatsNewDialogParams.closeButtonColor = QColor(0xDDDDDD);
    params.whatsNewDialogParams.headerDecorationColor = QColor(0xE8B800);

    // ShortcutsIcons
    ShortcutsIconsParams sci;
    sci.iconColor = QColor(0xD0D0D0); // Light icons for dark background
    params.shortcutsIconsParams = sci;

    return params;
}

// TODO
ThemeParams paramsFromFile(const QString &filePath)
{
    return {};
}
