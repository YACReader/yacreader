#include "theme_factory.h"

#include <QApplication>

#include "icon_utils.h"
#include "yacreader_global.h"

struct ComicVineParams {
    ComicVineThemeTemplates t;

    QColor contentTextColor;
    QColor contentBackgroundColor;
    QColor contentAltBackgroundColor;
    QColor dialogBackgroundColor;

    QColor tableBackgroundColor;
    QColor tableAltBackgroundColor;
    QColor tableBorderColor;
    QColor tableSelectedColor;
    QColor tableHeaderBackgroundColor;
    QColor tableHeaderBorderColor;
    QColor tableHeaderTextColor;
    QColor tableScrollHandleColor;
    QColor tableScrollBackgroundColor;
    QColor tableSectionBorderLight;
    QColor tableSectionBorderDark;

    QColor labelTextColor;
    QColor labelBackgroundColor;
    QColor hyperlinkColor; // TODO: it doesn't work

    QColor buttonBackgroundColor;
    QColor buttonTextColor;
    QColor buttonBorderColor;

    QColor radioUncheckedColor;

    QColor radioCheckedBackgroundColor;
    QColor radioCheckedIndicatorColor;

    QColor checkBoxTickColor;

    QColor toolButtonAccentColor;

    QColor downArrowColor;
    QColor upArrowColor;

    QColor busyIndicatorColor;
    QColor navIconColor;
    QColor rowIconColor;
};

struct EmptyContainerParams {
    EmptyContainerThemeTemplates t;

    QColor backgroundColor;
    QColor titleTextColor;

    // For NoLibrariesWidget
    QColor textColor;
    QColor descriptionTextColor;

    QColor searchIconColor; // Color for search-related icons (replaces #f0f in search_result.svg)
};

struct SidebarParams {
    QColor backgroundColor;
    QColor separatorColor;
    QColor sectionSeparatorColor; // Horizontal separators between sidebar sections

    bool uppercaseLabels;

    // Title bar colors
    QColor titleTextColor;
    QColor titleDropShadowColor;
    QColor busyIndicatorColor;
};

struct ImportWidgetParams {
    ImportWidgetThemeTemplates t;

    QColor backgroundColor;
    QColor titleTextColor;
    QColor descriptionTextColor;
    QColor currentComicTextColor;
    QColor coversViewBackgroundColor;
    QColor coversLabelColor;
    QColor coversDecorationBgColor;
    QColor coversDecorationShadowColor;
    QColor modeIconColor;
    QColor iconColor;
    QColor iconCheckedColor;
};

struct TreeViewParams {
    TreeViewThemeTemplates t;

    QColor textColor;
    QColor selectionBackgroundColor;
    QColor scrollBackgroundColor;
    QColor scrollHandleColor;
    QColor selectedTextColor;
    QColor folderIndicatorColor;
};

struct TableViewParams {
    TableViewThemeTemplates t;

    QColor alternateBackgroundColor;
    QColor backgroundColor;
    QColor cornerButtonBackgroundColor;
    QColor cornerButtonBorderColor;
    QColor cornerButtonGradientColor;
    QColor itemBorderBottomColor;
    QColor itemBorderTopColor;
    QColor itemTextColor;
    QColor selectedColor;
    QColor selectedTextColor;
    QColor headerTextColor;

    QColor starRatingColor;
    QColor starRatingSelectedColor;
};

struct QmlViewParams {
    // Grid colors
    QColor backgroundColor;
    QColor cellColor;
    QColor cellColorWithBackground;
    QColor selectedColor;
    QColor selectedBorderColor;
    QColor borderColor;
    QColor titleColor;
    QColor textColor;
    bool showDropShadow;

    // Info panel colors
    QColor infoBackgroundColor;
    QColor infoBorderColor;
    QColor infoShadowColor;
    QColor infoTextColor;
    QColor infoTitleColor;

    // Rating and favorite colors
    QColor ratingUnselectedColor;
    QColor ratingSelectedColor;
    QColor favUncheckedColor;
    QColor favCheckedColor;
    QColor readTickUncheckedColor;
    QColor readTickCheckedColor;

    // Current comic banner
    QColor currentComicBackgroundColor;

    // Continue reading section (FolderContentView)
    QColor continueReadingBackgroundColor;
    QColor continueReadingColor;
};

struct MainToolbarParams {
    MainToolbarThemeTemplates t;

    QColor backgroundColor;
    QColor folderNameColor;
    QColor dividerColor;
    QColor iconColor;
    QColor iconDisabledColor;
};

struct ContentSplitterParams {
    ContentSplitterThemeTemplates t;

    QColor handleColor;
    int horizontalHandleHeight; // for vertical splitter (horizontal handle)
    int verticalHandleWidth; // for horizontal splitter (vertical handle)
};

struct SidebarIconsParams {
    // Icon colors - #f0f placeholder gets replaced with these
    QColor iconColor; // Main icon color (replaces #f0f)
    QColor shadowColor; // Shadow color (replaces #0ff)
    QColor extraColor; // Extra info like ticks (replaces #ff0)

    // When true, use QFileIconProvider for folder icons and overlay a tick for finished folders
    bool useSystemFolderIcons;
    QColor folderReadOverlayColor; // Color for the tick overlay (replaces #f0f in folder_read_overlay.svg)
};

struct ServerConfigDialogThemeTemplates {
    QString dialogQSS = "ServerConfigDialog { background-color: %1; }";
    QString titleLabelQSS = "QLabel { color: %1; font-size: 30px; font-family: Arial; }";
    QString qrMessageLabelQSS = "QLabel { color: %1; font-size: 18px; font-family: Arial; }";
    QString propagandaLabelQSS = "QLabel { color: %1; font-size: 13px; font-family: Arial; font-style: italic; }";
    QString labelQSS = "QLabel { color: %1; font-size: 18px; font-family: Arial; }";
    QString checkBoxQSS = "QCheckBox { color: %1; font-size: 13px; font-family: Arial; }";
};

struct LibraryItemParams {
    QColor textColor;
    QColor selectedTextColor;
    QColor selectedBackgroundColor;
    QColor libraryIconSelectedColor; // Color for the library icon when selected
    QColor libraryOptionsIconColor; // Color for the options icon (shown only when selected)
};

struct ComicsViewToolbarParams {
    ComicsViewToolbarThemeTemplates t;

    QColor backgroundColor;
    QColor separatorColor;
    QColor checkedBackgroundColor;
    QColor iconColor; // Main icon color (replaces #f0f)
};

struct SearchLineEditParams {
    SearchLineEditThemeTemplates t;

    QColor textColor;
    QColor backgroundColor;
    QColor iconColor; // Main icon color (replaces #f0f)
};

struct ReadingListIconsParams {
    QMap<QString, QColor> labelColors; // Label colors by name (e.g., "red" -> #f67a7b)
    QColor labelShadowColor; // Shadow color for labels (replaces #0ff)

    // Special list icon colors
    QColor readingListMainColor; // default_0 main color (replaces #f0f)
    QColor favoritesMainColor; // default_1 main color (replaces #f0f)
    QColor currentlyReadingMainColor; // default_2 main color (replaces #f0f)
    QColor currentlyReadingOuterColor; // default_2 outer circle (replaces #ff0)
    QColor specialListShadowColor; // Shadow color for special lists (replaces #0ff)

    // List icon colors
    QColor listMainColor; // main color (replaces #f0f)
    QColor listShadowColor; // shadow color (replaces #0ff)
    QColor listDetailColor; // detail/checkbox color (replaces #ff0)
};

struct DialogIconsParams {
    QColor iconColor; // Main icon color (replaces #f0f)
};

struct MenuIconsParams {
    QColor iconColor; // Main icon color (replaces #f0f)
};

struct ShortcutsIconsParams {
    QColor iconColor; // Main icon color (replaces #f0f)
};

struct ServerConfigDialogParams {
    ServerConfigDialogThemeTemplates t;
    QColor backgroundColor;
    QColor titleTextColor;
    QColor qrMessageTextColor;
    QColor propagandaTextColor;
    QColor labelTextColor;
    QColor checkBoxTextColor;
    QColor qrBackgroundColor;
    QColor qrForegroundColor;
    QColor decorationColor;
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

struct ThemeParams {
    QString themeName;
    QColor defaultContentBackgroundColor;

    ComicFlowColors comicFlowColors;
    ComicVineParams comicVineParams;
    HelpAboutDialogTheme helpAboutDialogParams;
    EmptyContainerParams emptyContainerParams;
    SidebarParams sidebarParams;
    SidebarIconsParams sidebarIconsParams;
    LibraryItemParams libraryItemParams;
    ImportWidgetParams importWidgetParams;
    ServerConfigDialogParams serverConfigDialogParams;
    MainToolbarParams mainToolbarParams;
    ContentSplitterParams contentSplitterParams;
    TreeViewParams treeViewParams;
    TableViewParams tableViewParams;
    QmlViewParams qmlViewParams;
    ComicsViewToolbarParams comicsViewToolbarParams;
    SearchLineEditParams searchLineEditParams;
    ReadingListIconsParams readingListIconsParams;
    DialogIconsParams dialogIconsParams;
    MenuIconsParams menuIconsParams;
    ShortcutsIconsParams shortcutsIconsParams;
    WhatsNewDialogParams whatsNewDialogParams;
};

Theme makeTheme(const ThemeParams &params)
{
    Theme theme;

    theme.defaultContentBackgroundColor = params.defaultContentBackgroundColor;

    // Comic Flow
    const auto &cf = params.comicFlowColors;
    theme.comicFlow.backgroundColor = cf.backgroundColor;
    theme.comicFlow.textColor = cf.textColor;

    // Comic Vine
    const auto &cv = params.comicVineParams;
    const auto &t = cv.t;

    auto recolor = [&](const QString &path, const QColor &color) {
        return recoloredSvgToThemeFile(path, color, params.themeName);
    };

    theme.comicVine.defaultLabelQSS = t.defaultLabelQSS.arg(cv.labelTextColor.name());
    theme.comicVine.titleLabelQSS = t.titleLabelQSS.arg(cv.labelTextColor.name());
    theme.comicVine.coverLabelQSS = t.coverLabelQSS.arg(cv.labelBackgroundColor.name(), cv.labelTextColor.name());
    theme.comicVine.radioButtonQSS = t.radioButtonQSS.arg(cv.buttonTextColor.name(), recolor(":/images/comic_vine/radioUnchecked.svg", cv.radioUncheckedColor), recoloredSvgToThemeFile(":/images/comic_vine/radioChecked.svg", cv.radioCheckedBackgroundColor, cv.radioCheckedIndicatorColor, params.themeName));
    theme.comicVine.checkBoxQSS = t.checkBoxQSS.arg(cv.buttonTextColor.name(), cv.buttonBorderColor.name(), cv.buttonBackgroundColor.name(), recolor(":/images/comic_vine/checkBoxTick.svg", cv.checkBoxTickColor));

    theme.comicVine.scraperLineEditTitleLabelQSS = t.scraperLineEditTitleLabelQSS.arg(cv.contentTextColor.name());
    theme.comicVine.scraperLineEditQSS = t.scraperLineEditQSS.arg(cv.contentAltBackgroundColor.name(), cv.contentTextColor.name(), "%1");

    theme.comicVine.scraperToolButtonQSS = t.scraperToolButtonQSS.arg(cv.buttonBackgroundColor.name(), cv.buttonTextColor.name(), cv.toolButtonAccentColor.name());
    theme.comicVine.scraperToolButtonSeparatorQSS = t.scraperToolButtonSeparatorQSS.arg(cv.toolButtonAccentColor.name());
    theme.comicVine.scraperToolButtonFillColor = cv.buttonBackgroundColor;

    theme.comicVine.scraperScrollLabelTextQSS = t.scraperScrollLabelTextQSS.arg(cv.contentBackgroundColor.name(), cv.contentTextColor.name(), cv.hyperlinkColor.name());
    theme.comicVine.scraperScrollLabelScrollAreaQSS = t.scraperScrollLabelScrollAreaQSS.arg(cv.contentBackgroundColor.name(), cv.tableScrollHandleColor.name(), cv.tableScrollBackgroundColor.name());

    theme.comicVine.scraperTableViewQSS = t.scraperTableViewQSS
                                                  .arg(cv.tableHeaderTextColor.name(),
                                                       cv.tableAltBackgroundColor.name(),
                                                       cv.tableBackgroundColor.name(),
                                                       cv.tableSelectedColor.name(),
                                                       cv.tableHeaderBackgroundColor.name(),
                                                       cv.tableHeaderBorderColor.name(),
                                                       cv.tableHeaderTextColor.name(),
                                                       cv.tableSectionBorderDark.name(),
                                                       cv.tableSectionBorderLight.name(),
                                                       cv.tableScrollHandleColor.name(),
                                                       cv.tableScrollBackgroundColor.name(),
                                                       recolor(":/images/comic_vine/downArrow.svg", cv.downArrowColor),
                                                       recolor(":/images/comic_vine/upArrow.svg", cv.upArrowColor));

    theme.comicVine.dialogQSS = t.dialogQSS.arg(cv.dialogBackgroundColor.name());
    theme.comicVine.dialogButtonsQSS = t.dialogButtonsQSS.arg(cv.buttonBorderColor.name(), cv.buttonBackgroundColor.name(), cv.buttonTextColor.name());

    theme.comicVine.busyIndicatorColor = cv.busyIndicatorColor;

    theme.comicVine.nextPageIcon = { QIcon(recolor(t.nextPageIcon, cv.navIconColor)), t.pageIconSize };
    theme.comicVine.previousPageIcon = { QIcon(recolor(t.previousPageIcon, cv.navIconColor)), t.pageIconSize };

    theme.comicVine.rowUpIcon = { QIcon(recolor(t.rowUpIcon, cv.rowIconColor)), t.rowIconSize };
    theme.comicVine.rowDownIcon = { QIcon(recolor(t.rowDownIcon, cv.rowIconColor)), t.rowIconSize };

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

    // EmptyContainer
    const auto &ec = params.emptyContainerParams;
    const auto &ect = ec.t;

    theme.emptyContainer.backgroundColor = ec.backgroundColor;
    theme.emptyContainer.titleLabelQSS = ect.titleLabelQSS.arg(ec.titleTextColor.name());
    theme.emptyContainer.textColor = ec.textColor;
    theme.emptyContainer.descriptionTextColor = ec.descriptionTextColor;
    theme.emptyContainer.noLibrariesIcon = renderSvgToPixmap(recoloredSvgToThemeFile(":/images/noLibrariesIcon.svg", ec.searchIconColor, params.themeName), 165, 160, qApp->devicePixelRatio());
    {
        const qreal dpr = qApp->devicePixelRatio();
        theme.emptyContainer.searchingIcon = renderSvgToPixmap(recoloredSvgToThemeFile(":/images/search_result.svg", ec.searchIconColor, params.themeName, { .suffix = "_searching" }), 97, dpr);
        theme.emptyContainer.noSearchResultsIcon = renderSvgToPixmap(recoloredSvgToThemeFile(":/images/search_result.svg", ec.searchIconColor, params.themeName, { .suffix = "_no_results" }), 239, dpr);

        theme.emptyContainer.emptyFolderIcon = renderSvgToPixmap(recoloredSvgToThemeFile(":/images/empty_container/empty_folder.svg", ec.searchIconColor, params.themeName), 319, 243, dpr);
        theme.emptyContainer.emptyFavoritesIcon = renderSvgToPixmap(recoloredSvgToThemeFile(":/images/empty_container/empty_favorites.svg", QColor(0xe84853), params.themeName), 238, 223, dpr);
        theme.emptyContainer.emptyCurrentReadingsIcon = renderSvgToPixmap(recoloredSvgToThemeFile(":/images/empty_container/empty_current_readings.svg", ec.searchIconColor, params.themeName), 167, 214, dpr);
        theme.emptyContainer.emptyReadingListIcon = renderSvgToPixmap(recoloredSvgToThemeFile(":/images/empty_container/empty_reading_list.svg", ec.searchIconColor, params.themeName), 248, 187, dpr);

        // Generate empty label icons for each label color
        const auto &rli = params.readingListIconsParams;
        for (int c = YACReader::YRed; c <= YACReader::YDark; ++c) {
            auto labelColor = static_cast<YACReader::LabelColors>(c);
            auto colorName = YACReader::colorToName(labelColor);
            auto it = rli.labelColors.find(colorName);
            if (it != rli.labelColors.end()) {
                theme.emptyContainer.emptyLabelIcons[c] = renderSvgToPixmap(
                        recoloredSvgToThemeFile(":/images/empty_container/empty_label.svg", it.value(), params.themeName, { .suffix = "_" + colorName }), 243, 243, dpr);
            }
        }
    }
    // end EmptyContainer

    // Sidebar
    const auto &sb = params.sidebarParams;
    theme.sidebar.backgroundColor = sb.backgroundColor;
    theme.sidebar.separatorColor = sb.separatorColor;
    theme.sidebar.sectionSeparatorColor = sb.sectionSeparatorColor;
    theme.sidebar.uppercaseLabels = sb.uppercaseLabels;
    theme.sidebar.titleTextColor = sb.titleTextColor;
    theme.sidebar.titleDropShadowColor = sb.titleDropShadowColor;
    theme.sidebar.busyIndicatorColor = sb.busyIndicatorColor;
    // end Sidebar

    // ImportWidget
    const auto &iw = params.importWidgetParams;
    const auto &iwt = iw.t;
    theme.importWidget.backgroundColor = iw.backgroundColor;
    theme.importWidget.titleLabelQSS = iwt.titleLabelQSS.arg(iw.titleTextColor.name());
    theme.importWidget.descriptionTextColor = iw.descriptionTextColor;
    theme.importWidget.currentComicTextColor = iw.currentComicTextColor;
    theme.importWidget.coversViewBackgroundColor = iw.coversViewBackgroundColor;
    theme.importWidget.coversLabelColor = iw.coversLabelColor;
    theme.importWidget.topCoversDecoration = QPixmap(recoloredSvgToThemeFile(":/images/import/importTopCoversDecoration.svg", iw.coversDecorationBgColor, iw.coversDecorationShadowColor, params.themeName));
    theme.importWidget.bottomCoversDecoration = QPixmap(recoloredSvgToThemeFile(":/images/import/importBottomCoversDecoration.svg", iw.coversDecorationBgColor, iw.coversDecorationShadowColor, params.themeName));
    theme.importWidget.importingIcon = QPixmap(recoloredSvgToThemeFile(":/images/import/importingIcon.svg", iw.modeIconColor, params.themeName));
    theme.importWidget.updatingIcon = QPixmap(recoloredSvgToThemeFile(":/images/import/updatingIcon.svg", iw.modeIconColor, params.themeName));
    {
        QIcon coversToggle;
        const QString normalPath = recoloredSvgToThemeFile(":/images/import/coversToggle.svg", iw.iconColor, params.themeName);
        const QString checkedPath = recoloredSvgToThemeFile(":/images/import/coversToggle.svg", iw.iconCheckedColor, params.themeName, { .suffix = "_checked" });
        coversToggle.addFile(normalPath, QSize(), QIcon::Normal, QIcon::Off);
        coversToggle.addFile(checkedPath, QSize(), QIcon::Normal, QIcon::On);
        theme.importWidget.coversToggleIcon = coversToggle;
    }
    // end ImportWidget

    // TableView
    const auto &tav = params.tableViewParams;
    const auto &tavt = tav.t;
    theme.tableView.tableViewQSS = tavt.tableViewQSS
                                           .arg(tav.alternateBackgroundColor.name(),
                                                tav.backgroundColor.name(),
                                                tav.cornerButtonBackgroundColor.name(),
                                                tav.cornerButtonBorderColor.name(),
                                                tav.cornerButtonGradientColor.name(),
                                                tav.itemBorderBottomColor.name(),
                                                tav.itemBorderTopColor.name(),
                                                tav.itemTextColor.name(),
                                                tav.selectedColor.name(),
                                                tav.selectedTextColor.name(),
                                                tav.headerTextColor.name());
    theme.tableView.starRatingColor = tav.starRatingColor;
    theme.tableView.starRatingSelectedColor = tav.starRatingSelectedColor;
    // end TableView

    // QmlView
    const auto &qv = params.qmlViewParams;
    theme.qmlView.backgroundColor = qv.backgroundColor;
    theme.qmlView.cellColor = qv.cellColor;
    theme.qmlView.cellColorWithBackground = qv.cellColorWithBackground;
    theme.qmlView.selectedColor = qv.selectedColor;
    theme.qmlView.selectedBorderColor = qv.selectedBorderColor;
    theme.qmlView.borderColor = qv.borderColor;
    theme.qmlView.titleColor = qv.titleColor;
    theme.qmlView.textColor = qv.textColor;
    theme.qmlView.showDropShadow = qv.showDropShadow;
    theme.qmlView.infoBackgroundColor = qv.infoBackgroundColor;
    theme.qmlView.topShadow = recoloredSvgToThemeFile(":/qml/info-top-shadow.svg", qv.infoBackgroundColor, qv.infoBorderColor, qv.infoShadowColor, params.themeName);
    theme.qmlView.infoShadow = recoloredSvgToThemeFile(":/qml/info-shadow.svg", qv.infoBackgroundColor, qv.infoBorderColor, qv.infoShadowColor, params.themeName);
    theme.qmlView.infoIndicator = recoloredSvgToThemeFile(":/qml/info-indicator.svg", qv.infoBackgroundColor, qv.infoBorderColor, qv.infoShadowColor, params.themeName);
    theme.qmlView.infoTextColor = qv.infoTextColor;
    theme.qmlView.infoTitleColor = qv.infoTitleColor;
    theme.qmlView.ratingUnselectedColor = qv.ratingUnselectedColor;
    theme.qmlView.ratingSelectedColor = qv.ratingSelectedColor;
    theme.qmlView.favUncheckedColor = qv.favUncheckedColor;
    theme.qmlView.favCheckedColor = qv.favCheckedColor;
    theme.qmlView.readTickUncheckedColor = qv.readTickUncheckedColor;
    theme.qmlView.readTickCheckedColor = qv.readTickCheckedColor;
    theme.qmlView.currentComicBackgroundColor = qv.currentComicBackgroundColor;
    theme.qmlView.continueReadingBackgroundColor = qv.continueReadingBackgroundColor;
    theme.qmlView.continueReadingColor = qv.continueReadingColor;
    // end QmlView

    // MainToolbar
    const auto &mt = params.mainToolbarParams;
    theme.mainToolbar.backgroundColor = mt.backgroundColor;
    theme.mainToolbar.folderNameColor = mt.folderNameColor;
    theme.mainToolbar.toolbarQSS = mt.t.toolbarQSS;
    theme.mainToolbar.folderNameLabelQSS = mt.t.folderNameLabelQSS.arg(mt.folderNameColor.name());

    // Build divider pixmap with gradient (HiDPI aware)
    {
        const qreal dpr = qApp->devicePixelRatio();
        const int width = 1;
        const int height = 34;

        QPixmap pixmap(width * dpr, height * dpr);
        pixmap.setDevicePixelRatio(dpr);
        pixmap.fill(Qt::transparent);

        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing);

        QLinearGradient gradient(0, 0, 0, height);
        QColor transparentColor = mt.dividerColor;
        transparentColor.setAlpha(0);

        gradient.setColorAt(0.0, transparentColor);
        gradient.setColorAt(0.5, mt.dividerColor);
        gradient.setColorAt(1.0, transparentColor);

        painter.setPen(QPen(QBrush(gradient), 1));
        painter.drawLine(0, 0, 0, height);

        theme.mainToolbar.dividerPixmap = pixmap;
    }

    // Build icons with Normal and Disabled states
    auto makeToolbarIcon = [&](const QString &basePath) {
        QIcon icon;
        const QString normalPath = recoloredSvgToThemeFile(basePath, mt.iconColor, params.themeName);
        const QString disabledPath = recoloredSvgToThemeFile(basePath, mt.iconDisabledColor, params.themeName, { .suffix = "_disabled" });
        icon.addFile(normalPath, QSize(), QIcon::Normal, QIcon::Off);
        icon.addFile(disabledPath, QSize(), QIcon::Disabled, QIcon::Off);
        return icon;
    };

    theme.mainToolbar.backIcon = makeToolbarIcon(":/images/main_toolbar/back.svg");
    theme.mainToolbar.forwardIcon = makeToolbarIcon(":/images/main_toolbar/forward.svg");
    theme.mainToolbar.settingsIcon = makeToolbarIcon(":/images/main_toolbar/settings.svg");
    theme.mainToolbar.serverIcon = makeToolbarIcon(":/images/main_toolbar/server.svg");
    theme.mainToolbar.helpIcon = makeToolbarIcon(":/images/main_toolbar/help.svg");
    theme.mainToolbar.gridIcon = makeToolbarIcon(":/images/main_toolbar/grid.svg");
    theme.mainToolbar.flowIcon = makeToolbarIcon(":/images/main_toolbar/flow.svg");
    theme.mainToolbar.infoIcon = makeToolbarIcon(":/images/main_toolbar/info.svg");
    theme.mainToolbar.fullscreenIcon = makeToolbarIcon(":/images/main_toolbar/fullscreen.svg");
    // end MainToolbar

    // SidebarIcons
    const auto &si = params.sidebarIconsParams;

    // Helper to create icons with shadow (two-color: #f0f main, #0ff shadow)
    // Adds both Normal and Selected modes to prevent Qt from applying a selection tint
    auto makeSidebarIcon = [&](const QString &basePath) {
        const QString path = recoloredSvgToThemeFile(basePath, si.iconColor, si.shadowColor, params.themeName);
        QIcon icon;
        icon.addFile(path, QSize(), QIcon::Normal, QIcon::Off);
        icon.addFile(path, QSize(), QIcon::Selected, QIcon::Off);
        return icon;
    };

    // Helper for icons with extra color (three-color: #f0f main, #0ff shadow, #ff0 extra)
    auto makeSidebarIconWithExtra = [&](const QString &basePath) {
        const QString path = recoloredSvgToThemeFile(basePath, si.iconColor, si.shadowColor, si.extraColor, params.themeName);
        QIcon icon;
        icon.addFile(path, QSize(), QIcon::Normal, QIcon::Off);
        icon.addFile(path, QSize(), QIcon::Selected, QIcon::Off);
        return icon;
    };

    // Helper for single-color icons (only #f0f main)
    auto makeSingleColorIcon = [&](const QString &basePath) {
        const QString path = recoloredSvgToThemeFile(basePath, si.iconColor, params.themeName);
        QIcon icon;
        icon.addFile(path, QSize(), QIcon::Normal, QIcon::Off);
        icon.addFile(path, QSize(), QIcon::Selected, QIcon::Off);
        return icon;
    };

    // System folder icons flag and overlay
    theme.sidebarIcons.useSystemFolderIcons = si.useSystemFolderIcons;
    if (si.useSystemFolderIcons) {
        const QString overlayPath = recoloredSvgToThemeFile(":/images/sidebar/folder_read_overlay.svg", si.folderReadOverlayColor, params.themeName);
        theme.sidebarIcons.folderReadOverlay = QPixmap(overlayPath);
    }

    // Folder icons
    theme.sidebarIcons.folderIcon = makeSidebarIcon(":/images/sidebar/folder.svg");
    theme.sidebarIcons.folderFinishedIcon = makeSidebarIconWithExtra(":/images/sidebar/folder_finished.svg");

    // Library icon (unselected state uses sidebar colors)
    theme.sidebarIcons.libraryIcon = makeSidebarIcon(":/images/sidebar/libraryIcon.svg");

    // Action icons
    theme.sidebarIcons.newLibraryIcon = makeSidebarIcon(":/images/sidebar/newLibraryIcon.svg");
    theme.sidebarIcons.openLibraryIcon = makeSidebarIcon(":/images/sidebar/openLibraryIcon.svg");
    theme.sidebarIcons.addNewIcon = makeSidebarIcon(":/images/sidebar/addNew_sidebar.svg");
    theme.sidebarIcons.deleteIcon = makeSidebarIcon(":/images/sidebar/delete_sidebar.svg");
    theme.sidebarIcons.setRootIcon = makeSidebarIcon(":/images/sidebar/setRoot.svg");
    theme.sidebarIcons.expandIcon = makeSidebarIcon(":/images/sidebar/expand.svg");
    theme.sidebarIcons.colapseIcon = makeSidebarIcon(":/images/sidebar/colapse.svg");
    theme.sidebarIcons.addLabelIcon = makeSidebarIcon(":/images/sidebar/addLabelIcon.svg");
    theme.sidebarIcons.renameListIcon = makeSidebarIcon(":/images/sidebar/renameListIcon.svg");

    // Branch icons (paths for QSS)
    theme.sidebarIcons.branchClosedIconPath = recoloredSvgToThemeFile(":/images/sidebar/branch-closed.svg", si.iconColor, params.themeName);
    theme.sidebarIcons.branchOpenIconPath = recoloredSvgToThemeFile(":/images/sidebar/branch-open.svg", si.iconColor, params.themeName);
    // end SidebarIcons

    // LibraryItem
    const auto &li = params.libraryItemParams;
    theme.libraryItem.textColor = li.textColor;
    theme.libraryItem.selectedTextColor = li.selectedTextColor;
    theme.libraryItem.selectedBackgroundColor = li.selectedBackgroundColor;

    // Library icon when selected (uses its own color to contrast with selected background)
    const QString libraryIconSelectedPath = recoloredSvgToThemeFile(":/images/sidebar/libraryIconSelected.svg", li.libraryIconSelectedColor, params.themeName);
    theme.libraryItem.libraryIconSelected = QIcon(libraryIconSelectedPath);

    // Library options icon (shown only when selected, uses its own color)
    const QString libraryOptionsPath = recoloredSvgToThemeFile(":/images/sidebar/libraryOptions.svg", li.libraryOptionsIconColor, params.themeName);
    theme.libraryItem.libraryOptionsIcon = QIcon(libraryOptionsPath);
    // end LibraryItem

    // TreeView (must come after SidebarIcons for branch icon paths)
    const auto &tv = params.treeViewParams;
    theme.treeView.treeViewQSS = tv.t.styledTreeViewQSS
                                         .arg(tv.textColor.name(),
                                              tv.selectionBackgroundColor.name(),
                                              tv.scrollBackgroundColor.name(),
                                              tv.scrollHandleColor.name(),
                                              tv.selectedTextColor.name(),
                                              theme.sidebarIcons.branchClosedIconPath,
                                              theme.sidebarIcons.branchOpenIconPath);
    theme.treeView.folderIndicatorColor = tv.folderIndicatorColor;
    // end TreeView

    // ContentSplitter
    const auto &cs = params.contentSplitterParams;
    theme.contentSplitter.horizontalSplitterQSS = cs.t.horizontalSplitterQSS
                                                          .arg(cs.handleColor.name())
                                                          .arg(cs.verticalHandleWidth);
    theme.contentSplitter.verticalSplitterQSS = cs.t.verticalSplitterQSS
                                                        .arg(cs.handleColor.name())
                                                        .arg(cs.horizontalHandleHeight);
    // end ContentSplitter

    // ComicsViewToolbar
    const auto &cvt = params.comicsViewToolbarParams;

    // Helper to create single-color icons for comics view toolbar
    auto makeComicsViewIcon = [&](const QString &basePath) {
        const QString path = recoloredSvgToThemeFile(basePath, cvt.iconColor, params.themeName);
        QIcon icon;
        icon.addFile(path, QSize(), QIcon::Normal, QIcon::Off);
        icon.addFile(path, QSize(), QIcon::Selected, QIcon::Off);
        return icon;
    };

    theme.comicsViewToolbar.toolbarQSS = cvt.t.toolbarQSS
            .arg(cvt.backgroundColor.name())
            .arg(cvt.separatorColor.name())
            .arg(cvt.checkedBackgroundColor.name());
    theme.comicsViewToolbar.openInYACReaderIcon = makeComicsViewIcon(":/images/comics_view_toolbar/openInYACReader.svg");
    theme.comicsViewToolbar.setAsReadIcon = makeComicsViewIcon(":/images/comics_view_toolbar/setReadButton.svg");
    theme.comicsViewToolbar.setAsUnreadIcon = makeComicsViewIcon(":/images/comics_view_toolbar/setUnread.svg");
    theme.comicsViewToolbar.showComicInfoIcon = makeComicsViewIcon(":/images/comics_view_toolbar/show_comic_info.svg");
    theme.comicsViewToolbar.setAsNormalIcon = makeComicsViewIcon(":/images/comics_view_toolbar/setNormal.svg");
    theme.comicsViewToolbar.setAsMangaIcon = makeComicsViewIcon(":/images/comics_view_toolbar/setManga.svg");
    theme.comicsViewToolbar.editComicIcon = makeComicsViewIcon(":/images/comics_view_toolbar/editComic.svg");
    theme.comicsViewToolbar.getInfoIcon = makeComicsViewIcon(":/images/comics_view_toolbar/getInfo.svg");
    theme.comicsViewToolbar.assignNumberIcon = makeComicsViewIcon(":/images/comics_view_toolbar/asignNumber.svg");
    theme.comicsViewToolbar.selectAllIcon = makeComicsViewIcon(":/images/comics_view_toolbar/selectAll.svg");
    theme.comicsViewToolbar.deleteIcon = makeComicsViewIcon(":/images/comics_view_toolbar/trash.svg");
    theme.comicsViewToolbar.hideComicFlowIcon = makeComicsViewIcon(":/images/comics_view_toolbar/hideComicFlow.svg");
    theme.comicsViewToolbar.showMarksIcon = makeComicsViewIcon(":/images/comics_view_toolbar/showMarks.svg");
    theme.comicsViewToolbar.showRecentIndicatorIcon = makeComicsViewIcon(":/images/comics_view_toolbar/showRecentIndicator.svg");
    theme.comicsViewToolbar.bigGridZoomIcon = makeComicsViewIcon(":/images/comics_view_toolbar/big_size_grid_zoom.svg");
    theme.comicsViewToolbar.smallGridZoomIcon = makeComicsViewIcon(":/images/comics_view_toolbar/small_size_grid_zoom.svg");
    // end ComicsViewToolbar

    // SearchLineEdit
    const auto &sle = params.searchLineEditParams;

    theme.searchLineEdit.lineEditQSS = sle.t.lineEditQSS
                                               .arg(sle.textColor.name(),
                                                    sle.backgroundColor.name());
    theme.searchLineEdit.searchLabelQSS = sle.t.searchLabelQSS;
    theme.searchLineEdit.clearButtonQSS = sle.t.clearButtonQSS;

    const qreal dpr = qApp->devicePixelRatio();
    theme.searchLineEdit.searchIcon = renderSvgToPixmap(recoloredSvgToThemeFile(":/images/iconSearchNew.svg", sle.iconColor, params.themeName), 15, dpr);
    theme.searchLineEdit.clearIcon = renderSvgToPixmap(recoloredSvgToThemeFile(":/images/clearSearchNew.svg", sle.iconColor, params.themeName), 12, dpr);
    // end SearchLineEdit

    // ReadingListIcons
    const auto &rli = params.readingListIconsParams;

    // Helper to create label icons from template (uses color name to generate label_<color>.svg files)
    auto makeLabelIcon = [&](const QString &colorName, const QColor &mainColor) {
        const QString path = recoloredSvgToThemeFile(":/images/lists/label_template.svg", mainColor, rli.labelShadowColor, params.themeName, { .fileName = "label_" + colorName });
        QIcon icon;
        icon.addFile(path, QSize(), QIcon::Normal, QIcon::Off);
        icon.addFile(path, QSize(), QIcon::Selected, QIcon::Off);
        return icon;
    };

    for (auto it = rli.labelColors.constBegin(); it != rli.labelColors.constEnd(); ++it) {
        theme.readingListIcons.labelIcons[it.key()] = makeLabelIcon(it.key(), it.value());
    }

    // Special list icons
    auto makeSpecialIcon = [&](const QString &basePath, const QColor &mainColor) {
        const QString path = recoloredSvgToThemeFile(basePath, mainColor, rli.specialListShadowColor, params.themeName);
        QIcon icon;
        icon.addFile(path, QSize(), QIcon::Normal, QIcon::Off);
        icon.addFile(path, QSize(), QIcon::Selected, QIcon::Off);
        return icon;
    };

    theme.readingListIcons.readingListIcon = makeSpecialIcon(":/images/lists/default_0.svg", rli.readingListMainColor);
    theme.readingListIcons.favoritesIcon = makeSpecialIcon(":/images/lists/default_1.svg", rli.favoritesMainColor);

    // Currently reading has 3 colors
    {
        const QString path = recoloredSvgToThemeFile(":/images/lists/default_2.svg", rli.currentlyReadingMainColor, rli.specialListShadowColor, rli.currentlyReadingOuterColor, params.themeName);
        QIcon icon;
        icon.addFile(path, QSize(), QIcon::Normal, QIcon::Off);
        icon.addFile(path, QSize(), QIcon::Selected, QIcon::Off);
        theme.readingListIcons.currentlyReadingIcon = icon;
    }

    // List icon (3 colors)
    {
        const QString path = recoloredSvgToThemeFile(":/images/lists/list.svg", rli.listMainColor, rli.listShadowColor, rli.listDetailColor, params.themeName);
        QIcon icon;
        icon.addFile(path, QSize(), QIcon::Normal, QIcon::Off);
        icon.addFile(path, QSize(), QIcon::Selected, QIcon::Off);
        theme.readingListIcons.listIcon = icon;
    }
    // end ReadingListIcons

    // MenuIcons
    const auto &mi = params.menuIconsParams;
    auto makeMenuIcon = [&](const QString &basePath) {
        const QString path = recoloredSvgToThemeFile(basePath, mi.iconColor, params.themeName);
        return QIcon(path);
    };

    theme.menuIcons.exportComicsInfoIcon = makeMenuIcon(":/images/menus_icons/exportComicsInfoIcon.svg");
    theme.menuIcons.importComicsInfoIcon = makeMenuIcon(":/images/menus_icons/importComicsInfoIcon.svg");
    theme.menuIcons.exportLibraryIcon = makeMenuIcon(":/images/menus_icons/exportLibraryIcon.svg");
    theme.menuIcons.importLibraryIcon = makeMenuIcon(":/images/menus_icons/importLibraryIcon.svg");
    theme.menuIcons.updateLibraryIcon = makeMenuIcon(":/images/menus_icons/updateLibraryIcon.svg");
    theme.menuIcons.renameLibraryIcon = makeMenuIcon(":/images/menus_icons/editIcon.svg");
    theme.menuIcons.removeLibraryIcon = makeMenuIcon(":/images/menus_icons/removeLibraryIcon.svg");
    theme.menuIcons.openContainingFolderIcon = makeMenuIcon(":/images/menus_icons/open_containing_folder.svg");
    theme.menuIcons.updateCurrentFolderIcon = makeMenuIcon(":/images/menus_icons/update_current_folder.svg");
    theme.menuIcons.quitIcon = makeMenuIcon(":/images/viewer_toolbar/close.svg");
    // end MenuIcons

    // DialogIcons
    const auto &di = params.dialogIconsParams;
    auto makeDialogIcon = [&](const QString &basePath) {
        const QString path = recoloredSvgToThemeFile(basePath, di.iconColor, params.themeName);
        return QPixmap(path);
    };
    theme.dialogIcons.newLibraryIcon = makeDialogIcon(":/images/library_dialogs/new.svg");
    theme.dialogIcons.openLibraryIcon = makeDialogIcon(":/images/library_dialogs/openLibrary.svg");
    theme.dialogIcons.editIcon = makeDialogIcon(":/images/library_dialogs/edit.svg");
    theme.dialogIcons.exportComicsInfoIcon = makeDialogIcon(":/images/library_dialogs/exportComicsInfo.svg");
    theme.dialogIcons.importComicsInfoIcon = makeDialogIcon(":/images/library_dialogs/importComicsInfo.svg");
    theme.dialogIcons.exportLibraryIcon = makeDialogIcon(":/images/library_dialogs/exportLibrary.svg");
    theme.dialogIcons.importLibraryIcon = makeDialogIcon(":/images/library_dialogs/importLibrary.svg");
    {
        const QString path = recoloredSvgToThemeFile(":/images/find_folder.svg", di.iconColor, params.themeName);
        const qreal dpr = qApp->devicePixelRatio();
        theme.dialogIcons.findFolderIcon = QIcon(renderSvgToPixmap(path, 13, 13, dpr));
    }
    // end DialogIcons

    // ShortcutsIcons
    const auto &sci = params.shortcutsIconsParams;
    auto makeShortcutsIcon = [&](const QString &basePath) {
        const QString path = recoloredSvgToThemeFile(basePath, sci.iconColor, params.themeName);
        return QIcon(path);
    };

    theme.shortcutsIcons.comicsIcon = makeShortcutsIcon(":/images/shortcuts/shortcuts_group_comics.svg");
    theme.shortcutsIcons.foldersIcon = makeShortcutsIcon(":/images/shortcuts/shortcuts_group_folders.svg");
    theme.shortcutsIcons.generalIcon = makeShortcutsIcon(":/images/shortcuts/shortcuts_group_general.svg");
    theme.shortcutsIcons.librariesIcon = makeShortcutsIcon(":/images/shortcuts/shortcuts_group_libraries.svg");
    theme.shortcutsIcons.visualizationIcon = makeShortcutsIcon(":/images/shortcuts/shortcuts_group_visualization.svg");
    // end ShortcutsIcons

    // ServerConfigDialog
    const auto &scd = params.serverConfigDialogParams;
    theme.serverConfigDialog.dialogQSS = scd.t.dialogQSS.arg(scd.backgroundColor.name());
    theme.serverConfigDialog.titleLabelQSS = scd.t.titleLabelQSS.arg(scd.titleTextColor.name());
    theme.serverConfigDialog.qrMessageLabelQSS = scd.t.qrMessageLabelQSS.arg(scd.qrMessageTextColor.name());
    theme.serverConfigDialog.propagandaLabelQSS = scd.t.propagandaLabelQSS.arg(scd.propagandaTextColor.name());
    theme.serverConfigDialog.labelQSS = scd.t.labelQSS.arg(scd.labelTextColor.name());
    theme.serverConfigDialog.checkBoxQSS = scd.t.checkBoxQSS.arg(scd.checkBoxTextColor.name());
    theme.serverConfigDialog.qrBackgroundColor = scd.qrBackgroundColor;
    theme.serverConfigDialog.qrForegroundColor = scd.qrForegroundColor;
    theme.serverConfigDialog.backgroundDecoration = QPixmap(recoloredSvgToThemeFile(":/images/serverConfigBackground.svg", scd.decorationColor, params.themeName));

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

    return makeTheme(classicThemeParams());
}

ThemeParams classicThemeParams()
{
    ThemeParams params;
    params.themeName = "classic";
    params.defaultContentBackgroundColor = QColor(0x2A2A2A);

    ComicFlowColors cf;
    cf.backgroundColor = Qt::black;
    cf.textColor = QColor(0x4C4C4C);

    ComicVineParams cv;
    cv.contentTextColor = Qt::white;
    cv.contentBackgroundColor = QColor(0x2B2B2B);
    cv.contentAltBackgroundColor = QColor(0x2B2B2B);
    cv.dialogBackgroundColor = QColor(0x404040);

    cv.tableBackgroundColor = QColor(0x2B2B2B);
    cv.tableAltBackgroundColor = QColor(0x2E2E2E);
    cv.tableBorderColor = QColor(0x242424);
    cv.tableSelectedColor = QColor(0x555555);
    cv.tableHeaderBackgroundColor = QColor(0x292929);
    cv.tableHeaderBorderColor = QColor(0x1F1F1F);
    cv.tableHeaderTextColor = QColor(0xEBEBEB);
    cv.tableScrollHandleColor = QColor(0xDDDDDD);
    cv.tableScrollBackgroundColor = QColor(0x404040);
    cv.tableSectionBorderLight = QColor(0xFEFEFE);
    cv.tableSectionBorderDark = QColor(0xDFDFDF);

    cv.labelTextColor = Qt::white;
    cv.labelBackgroundColor = QColor(0x2B2B2B);
    cv.hyperlinkColor = QColor(0xFFCC00);

    cv.buttonBackgroundColor = QColor(0x2E2E2E);
    cv.buttonTextColor = Qt::white;
    cv.buttonBorderColor = QColor(0x242424);

    cv.radioUncheckedColor = QColor(0xE5E5E5);

    cv.radioCheckedBackgroundColor = QColor(0xE5E5E5);
    cv.radioCheckedIndicatorColor = QColor(0x5F5F5F);

    cv.checkBoxTickColor = Qt::white;

    cv.toolButtonAccentColor = QColor(0x282828);

    cv.downArrowColor = QColor(0x9F9F9F);
    cv.upArrowColor = QColor(0x9F9F9F);

    cv.busyIndicatorColor = Qt::white;
    cv.navIconColor = Qt::white;
    cv.rowIconColor = QColor(0xE5E5E5);

    cv.t = ComicVineThemeTemplates();

    params.comicFlowColors = cf;
    params.comicVineParams = cv;

    params.helpAboutDialogParams.headingColor = QColor(0x302f2d);
    params.helpAboutDialogParams.linkColor = QColor(0xC19441);

    WhatsNewDialogParams wnp;
    wnp.backgroundColor = QColor(0x2A2A2A);
    wnp.headerTextColor = QColor(0xE0E0E0);
    wnp.versionTextColor = QColor(0x858585);
    wnp.contentTextColor = QColor(0xE0E0E0);
    wnp.linkColor = QColor(0xE8B800);
    wnp.closeButtonColor = QColor(0xDDDDDD);
    wnp.headerDecorationColor = QColor(0xE8B800);
    params.whatsNewDialogParams = wnp;

    EmptyContainerParams ec;
    ec.backgroundColor = QColor(0x2A2A2A);
    ec.titleTextColor = QColor(0xCCCCCC);
    ec.textColor = QColor(0xCCCCCC);
    ec.descriptionTextColor = QColor(0xAAAAAA);
    ec.searchIconColor = QColor(0x4C4C4C);
    ec.t = EmptyContainerThemeTemplates();
    params.emptyContainerParams = ec;

    SidebarParams sb;
    sb.backgroundColor = QColor(0x454545);
    sb.separatorColor = QColor(0xBDBFBF);
    sb.sectionSeparatorColor = QColor(0x575757);
    sb.uppercaseLabels = true;
    sb.titleTextColor = QColor(0xBDBFBF);
    sb.titleDropShadowColor = Qt::black;
    sb.busyIndicatorColor = Qt::white;
    params.sidebarParams = sb;

    ImportWidgetParams iw;
    iw.backgroundColor = QColor(0x2A2A2A);
    iw.titleTextColor = QColor(0xCCCCCC);
    iw.descriptionTextColor = QColor(0xAAAAAA);
    iw.currentComicTextColor = QColor(0xAAAAAA);
    iw.coversViewBackgroundColor = QColor(0x3A3A3A);
    iw.coversLabelColor = QColor(0xAAAAAA);
    iw.coversDecorationBgColor = QColor(0x3A3A3A);
    iw.coversDecorationShadowColor = QColor(0x1A1A1A);
    iw.modeIconColor = QColor(0x4A4A4A);
    iw.iconColor = QColor(0xCCCCCC);
    iw.iconCheckedColor = QColor(0xAAAAAA);
    params.importWidgetParams = iw;

    TreeViewParams tv;
    tv.textColor = QColor(0xDDDFDF);
    tv.selectionBackgroundColor = QColor(0x2E2E2E);
    tv.scrollBackgroundColor = QColor(0x404040);
    tv.scrollHandleColor = QColor(0xDDDDDD);
    tv.selectedTextColor = Qt::white;
    tv.folderIndicatorColor = QColor(237, 197, 24);
    params.treeViewParams = tv;

    TableViewParams tav;
    tav.alternateBackgroundColor = QColor(0xF2F2F2);
    tav.backgroundColor = QColor(0xFAFAFA);
    tav.cornerButtonBackgroundColor = QColor(0xF5F5F5);
    tav.cornerButtonBorderColor = QColor(0xB8BDC4);
    tav.cornerButtonGradientColor = QColor(0xD1D1D1);
    tav.itemBorderBottomColor = QColor(0xDFDFDF);
    tav.itemBorderTopColor = QColor(0xFEFEFE);
    tav.itemTextColor = QColor(0x252626);
    tav.selectedColor = QColor(0xD4D4D4);
    tav.selectedTextColor = QColor(0x252626);
    tav.headerTextColor = QColor(0x313232);
    tav.starRatingColor = QColor(0xE9BE0F);
    tav.starRatingSelectedColor = QColor(0xFFFFFF);
    tav.t = TableViewThemeTemplates();
    params.tableViewParams = tav;

    QmlViewParams qv;
    qv.backgroundColor = QColor(0x2A2A2A);
    qv.cellColor = QColor(0x212121);
    qv.cellColorWithBackground = QColor(0x21, 0x21, 0x21, 0x99);
    qv.selectedColor = QColor(0x121212);
    qv.selectedBorderColor = QColor(0xFFCC00);
    qv.borderColor = QColor(0x121212);
    qv.titleColor = QColor(0xFFFFFF);
    qv.textColor = QColor(0xA8A8A8);
    qv.showDropShadow = true;
    qv.infoBackgroundColor = QColor(0x2E2E2E);
    qv.infoBorderColor = QColor(0x404040);
    qv.infoShadowColor = Qt::black;
    qv.infoTextColor = QColor(0xB0B0B0);
    qv.infoTitleColor = QColor(0xFFFFFF);
    qv.ratingUnselectedColor = QColor(0x1C1C1C);
    qv.ratingSelectedColor = QColor(0xFFFFFF);
    qv.favUncheckedColor = QColor(0x1C1C1C);
    qv.favCheckedColor = QColor(0xE84852);
    qv.readTickUncheckedColor = QColor(0x1C1C1C);
    qv.readTickCheckedColor = QColor(0xE84852);
    qv.currentComicBackgroundColor = QColor(0x00, 0x00, 0x00, 0x88);
    qv.continueReadingBackgroundColor = QColor(0x00, 0x00, 0x00, 0x88);
    qv.continueReadingColor = QColor(0xFFFFFF);
    params.qmlViewParams = qv;

    MainToolbarParams mt;
    mt.backgroundColor = QColor(0xF0F0F0);
    mt.folderNameColor = QColor(0x404040);
    mt.dividerColor = QColor(0xB8BDC4);
    mt.iconColor = QColor(0x404040);
    mt.iconDisabledColor = QColor(0xB0B0B0);
    params.mainToolbarParams = mt;

    ContentSplitterParams cs;
    cs.handleColor = QColor(0xB8B8B8);
    cs.horizontalHandleHeight = 4;
    cs.verticalHandleWidth = 4;
    params.contentSplitterParams = cs;

    SidebarIconsParams si;
    si.iconColor = QColor(0xE0E0E0);
    si.shadowColor = QColor(0xFF000000);
    si.extraColor = QColor(0x464646);
    si.useSystemFolderIcons = false;
    params.sidebarIconsParams = si;

    LibraryItemParams li;
    li.textColor = QColor(0xDDDFDF);
    li.selectedTextColor = Qt::white;
    li.selectedBackgroundColor = QColor(0x2E2E2E);
    li.libraryIconSelectedColor = Qt::white;
    li.libraryOptionsIconColor = Qt::white;
    params.libraryItemParams = li;

    ComicsViewToolbarParams cvt;
    cvt.backgroundColor = QColor(0xF0F0F0);
    cvt.separatorColor = QColor(0xCCCCCC);
    cvt.checkedBackgroundColor = QColor(0xCCCCCC);
    cvt.iconColor = QColor(0x404040);
    params.comicsViewToolbarParams = cvt;

    SearchLineEditParams sle;
    sle.textColor = QColor(0xABABAB);
    sle.backgroundColor = QColor(0x404040);
    sle.iconColor = QColor(0xF7F7F7);
    params.searchLineEditParams = sle;

    ReadingListIconsParams rli;
    rli.labelColors = {
        { "red", QColor(0xf67a7b) },
        { "orange", QColor(0xf5c240) },
        { "yellow", QColor(0xf2e446) },
        { "green", QColor(0xade738) },
        { "cyan", QColor(0xa0fddb) },
        { "blue", QColor(0x82c7ff) },
        { "violet", QColor(0x8f95ff) },
        { "purple", QColor(0xd692fc) },
        { "pink", QColor(0xfd9cda) },
        { "white", QColor(0xfcfcfc) },
        { "light", QColor(0xcbcbcb) },
        { "dark", QColor(0xb7b7b7) }
    };
    rli.labelShadowColor = Qt::black;
    rli.readingListMainColor = QColor(0xe7e7e7);
    rli.favoritesMainColor = QColor(0xe15055);
    rli.currentlyReadingMainColor = QColor(0xffcc00);
    rli.currentlyReadingOuterColor = Qt::black;
    rli.specialListShadowColor = Qt::black;
    rli.listMainColor = QColor(0xe7e7e7);
    rli.listShadowColor = Qt::black;
    rli.listDetailColor = QColor(0x464646);
    params.readingListIconsParams = rli;

    MenuIconsParams mi;
    mi.iconColor = QColor(0xF7F7F7);
    params.menuIconsParams = mi;

    DialogIconsParams dip;
    dip.iconColor = mi.iconColor;
    params.dialogIconsParams = dip;

    ShortcutsIconsParams sci;
    sci.iconColor = QColor(0xF7F7F7);
    params.shortcutsIconsParams = sci;

    ServerConfigDialogParams scd;
    scd.backgroundColor = QColor(0x2A2A2A);
    scd.titleTextColor = QColor(0x474747);
    scd.qrMessageTextColor = QColor(0xA3A3A3);
    scd.propagandaTextColor = QColor(0x4D4D4D);
    scd.labelTextColor = QColor(0x575757);
    scd.checkBoxTextColor = QColor(0x262626);
    scd.qrBackgroundColor = QColor(0x2A2A2A);
    scd.qrForegroundColor = Qt::white;
    scd.decorationColor = QColor(0xF7F7F7);
    params.serverConfigDialogParams = scd;

    return params;
}

ThemeParams lightThemeParams()
{
    ThemeParams params;
    params.themeName = "light";
    params.defaultContentBackgroundColor = QColor(0xFFFFFF);

    ComicFlowColors cf;
    cf.backgroundColor = Qt::white;
    cf.textColor = Qt::black;

    ComicVineParams cv;
    cv.contentTextColor = Qt::black;
    cv.contentBackgroundColor = QColor(0xECECEC);
    cv.contentAltBackgroundColor = QColor(0xE0E0E0);
    cv.dialogBackgroundColor = QColor(0xFBFBFB);

    cv.tableBackgroundColor = QColor(0xF4F4F4);
    cv.tableAltBackgroundColor = QColor(0xFAFAFA);
    cv.tableBorderColor = QColor(0xCCCCCC);
    cv.tableSelectedColor = QColor(0xDDDDDD);
    cv.tableHeaderBackgroundColor = QColor(0xE0E0E0);
    cv.tableHeaderBorderColor = QColor(0xC0C0C0);
    cv.tableHeaderTextColor = QColor(0x333333);
    cv.tableScrollHandleColor = QColor(0x888888);
    cv.tableScrollBackgroundColor = QColor(0xD0D0D0);
    cv.tableSectionBorderLight = QColor(0xFFFFFF);
    cv.tableSectionBorderDark = QColor(0xCCCCCC);

    cv.labelTextColor = Qt::black;
    cv.labelBackgroundColor = QColor(0xECECEC);
    cv.hyperlinkColor = QColor(0xFFCC00);

    cv.buttonBackgroundColor = QColor(0xE0E0E0);
    cv.buttonTextColor = Qt::black;
    cv.buttonBorderColor = QColor(0xCCCCCC);

    cv.radioUncheckedColor = QColor(0xE0E0E0);

    cv.radioCheckedBackgroundColor = QColor(0xE0E0E0);
    cv.radioCheckedIndicatorColor = QColor(0x222222);

    cv.checkBoxTickColor = Qt::black;

    cv.toolButtonAccentColor = QColor(0xA0A0A0);

    cv.downArrowColor = QColor(0x222222);
    cv.upArrowColor = QColor(0x222222);

    cv.busyIndicatorColor = Qt::black;
    cv.navIconColor = QColor(0x222222);
    cv.rowIconColor = QColor(0x222222);

    cv.t = ComicVineThemeTemplates();

    params.comicFlowColors = cf;
    params.comicVineParams = cv;

    params.helpAboutDialogParams.headingColor = QColor(0x302f2d);
    params.helpAboutDialogParams.linkColor = QColor(0xC19441);

    WhatsNewDialogParams wnp;
    wnp.backgroundColor = QColor(0xFFFFFF);
    wnp.headerTextColor = QColor(0x0A0A0A);
    wnp.versionTextColor = QColor(0x858585);
    wnp.contentTextColor = QColor(0x0A0A0A);
    wnp.linkColor = QColor(0xE8B800);
    wnp.closeButtonColor = QColor(0x444444);
    wnp.headerDecorationColor = QColor(0xE8B800);
    params.whatsNewDialogParams = wnp;

    EmptyContainerParams ec;
    ec.backgroundColor = QColor(0xFFFFFF);
    ec.titleTextColor = QColor(0x888888);
    ec.textColor = QColor(0x495252);
    ec.descriptionTextColor = QColor(0x565959);
    ec.searchIconColor = QColor(0xCCCCCC);
    ec.t = EmptyContainerThemeTemplates();
    params.emptyContainerParams = ec;

    SidebarParams sb;
    sb.backgroundColor = QColor(0xF1F1F1);
    sb.separatorColor = QColor(0x808080);
    sb.sectionSeparatorColor = QColor(0xD0D0D0);
    sb.uppercaseLabels = true;
    sb.titleTextColor = QColor(0x808080);
    sb.titleDropShadowColor = QColor(0xFFFFFF);
    sb.busyIndicatorColor = QColor(0x808080);
    params.sidebarParams = sb;

    ImportWidgetParams iw;
    iw.backgroundColor = QColor(0xFAFAFA);
    iw.titleTextColor = QColor(0x495252);
    iw.descriptionTextColor = QColor(0x565959);
    iw.currentComicTextColor = QColor(0x565959);
    iw.coversViewBackgroundColor = QColor(0xE6E6E6);
    iw.coversLabelColor = QColor(0x565959);
    iw.coversDecorationBgColor = QColor(0xE6E6E6);
    iw.coversDecorationShadowColor = QColor(0xA1A1A1);
    iw.modeIconColor = QColor(0xE6E6E6);
    iw.iconColor = QColor(0x495252);
    iw.iconCheckedColor = QColor(0x565959);
    params.importWidgetParams = iw;

    TreeViewParams tv;
    tv.textColor = Qt::black;
    tv.selectionBackgroundColor = QColor(0xD0D0D0);
    tv.scrollBackgroundColor = QColor(0xE0E0E0);
    tv.scrollHandleColor = QColor(0x888888);
    tv.selectedTextColor = QColor(0x1A1A1A);
    tv.folderIndicatorColor = QColor(85, 95, 127);
    params.treeViewParams = tv;

    TableViewParams tav;
    tav.alternateBackgroundColor = QColor(0xF2F2F2);
    tav.backgroundColor = QColor(0xFAFAFA);
    tav.cornerButtonBackgroundColor = QColor(0xF5F5F5);
    tav.cornerButtonBorderColor = QColor(0xB8BDC4);
    tav.cornerButtonGradientColor = QColor(0xD1D1D1);
    tav.itemBorderBottomColor = QColor(0xDFDFDF);
    tav.itemBorderTopColor = QColor(0xFEFEFE);
    tav.itemTextColor = QColor(0x252626);
    tav.selectedColor = QColor(0x3875D7);
    tav.selectedTextColor = QColor(0xFFFFFF);
    tav.headerTextColor = QColor(0x313232);
    tav.starRatingColor = QColor(0xE9BE0F);
    tav.starRatingSelectedColor = QColor(0xFFFFFF);
    tav.t = TableViewThemeTemplates();
    params.tableViewParams = tav;

    QmlViewParams qv;
    qv.backgroundColor = QColor(0xF6F6F6);
    qv.cellColor = QColor(0xFFFFFF);
    qv.cellColorWithBackground = QColor(0xFF, 0xFF, 0xFF, 0x99);
    qv.selectedColor = QColor(0xFFFFFF);
    qv.selectedBorderColor = QColor(0xFFCC00);
    qv.borderColor = QColor(0xDBDBDB);
    qv.titleColor = QColor(0x121212);
    qv.textColor = QColor(0x636363);
    qv.showDropShadow = true;
    qv.infoBackgroundColor = QColor(0xFFFFFF);
    qv.infoBorderColor = QColor(0x808080);
    qv.infoShadowColor = QColor(0x444444);
    qv.infoTextColor = QColor(0x404040);
    qv.infoTitleColor = QColor(0x2E2E2E);
    qv.ratingUnselectedColor = QColor(0xDEDEDE);
    qv.ratingSelectedColor = QColor(0x2B2B2B);
    qv.favUncheckedColor = QColor(0xDEDEDE);
    qv.favCheckedColor = QColor(0xE84852);
    qv.readTickUncheckedColor = QColor(0xDEDEDE);
    qv.readTickCheckedColor = QColor(0xE84852);
    qv.currentComicBackgroundColor = QColor(0xFF, 0xFF, 0xFF, 0x88);
    qv.continueReadingBackgroundColor = QColor(0xE8E8E8);
    qv.continueReadingColor = QColor::fromRgb(0x000000);
    params.qmlViewParams = qv;

    MainToolbarParams mt;
    mt.backgroundColor = QColor(0xF0F0F0);
    mt.folderNameColor = QColor(0x404040);
    mt.dividerColor = QColor(0xB8BDC4);
    mt.iconColor = QColor(0x404040);
    mt.iconDisabledColor = QColor(0xB0B0B0);
    params.mainToolbarParams = mt;

    ContentSplitterParams cs;
    cs.handleColor = QColor(0xB8B8B8);
    cs.horizontalHandleHeight = 4;
    cs.verticalHandleWidth = 4;
    params.contentSplitterParams = cs;

    SidebarIconsParams si;
    si.iconColor = QColor(0x606060);
    si.shadowColor = QColor(0xFFFFFF);
    si.extraColor = QColor(0xFFFFFF);
    si.useSystemFolderIcons = false;
    params.sidebarIconsParams = si;

    LibraryItemParams li;
    li.textColor = QColor(0x404040);
    li.selectedTextColor = QColor(0x1A1A1A);
    li.selectedBackgroundColor = QColor(0xD0D0D0);
    li.libraryIconSelectedColor = QColor(0x404040);
    li.libraryOptionsIconColor = QColor(0x404040);
    params.libraryItemParams = li;

    ComicsViewToolbarParams cvt;
    cvt.backgroundColor = QColor(0xF0F0F0);
    cvt.separatorColor = QColor(0xCCCCCC);
    cvt.checkedBackgroundColor = QColor(0xCCCCCC);
    cvt.iconColor = QColor(0x404040);
    params.comicsViewToolbarParams = cvt;

    SearchLineEditParams sle;
    sle.textColor = QColor(0x606060);
    sle.backgroundColor = QColor(0xE0E0E0);
    sle.iconColor = QColor(0x808080);
    params.searchLineEditParams = sle;

    ReadingListIconsParams rli;
    rli.labelColors = {
        { "red", QColor(0xf67a7b) },
        { "orange", QColor(0xf5c240) },
        { "yellow", QColor(0xf2e446) },
        { "green", QColor(0xade738) },
        { "cyan", QColor(0xa0fddb) },
        { "blue", QColor(0x82c7ff) },
        { "violet", QColor(0x8f95ff) },
        { "purple", QColor(0xd692fc) },
        { "pink", QColor(0xfd9cda) },
        { "white", QColor(0xfcfcfc) },
        { "light", QColor(0xcbcbcb) },
        { "dark", QColor(0xb7b7b7) }
    };
    rli.labelShadowColor = QColor(0xa0a0a0);
    rli.readingListMainColor = QColor(0x808080);
    rli.favoritesMainColor = QColor(0xe15055);
    rli.currentlyReadingMainColor = QColor(0xffcc00);
    rli.currentlyReadingOuterColor = Qt::black;
    rli.specialListShadowColor = QColor(0xa0a0a0);
    rli.listMainColor = QColor(0x808080);
    rli.listShadowColor = QColor(0xc0c0c0);
    rli.listDetailColor = QColor(0xFFFFFF);
    params.readingListIconsParams = rli;

    MenuIconsParams mi;
    mi.iconColor = QColor(0x606060);
    params.menuIconsParams = mi;

    DialogIconsParams dip;
    dip.iconColor = mi.iconColor;
    params.dialogIconsParams = dip;

    ShortcutsIconsParams sci;
    sci.iconColor = QColor(0x606060);
    params.shortcutsIconsParams = sci;

    ServerConfigDialogParams scd;
    scd.backgroundColor = QColor(0xFFFFFF);
    scd.titleTextColor = QColor(0x474747);
    scd.qrMessageTextColor = QColor(0xA3A3A3);
    scd.propagandaTextColor = QColor(0x4D4D4D);
    scd.labelTextColor = QColor(0x575757);
    scd.checkBoxTextColor = QColor(0x262626);
    scd.qrBackgroundColor = Qt::white;
    scd.qrForegroundColor = QColor(0x606060);
    scd.decorationColor = QColor(0x606060);
    params.serverConfigDialogParams = scd;

    return params;
}

ThemeParams darkThemeParams()
{
    ThemeParams params;
    params.themeName = "dark";
    params.defaultContentBackgroundColor = QColor(0x2A2A2A);

    ComicFlowColors cf;
    cf.backgroundColor = QColor(0x111111);
    cf.textColor = QColor(0x888888);

    ComicVineParams cv;
    cv.contentTextColor = Qt::white;
    cv.contentBackgroundColor = QColor(0x2B2B2B);
    cv.contentAltBackgroundColor = QColor(0x2E2E2E);
    cv.dialogBackgroundColor = QColor(0x404040);

    cv.tableBackgroundColor = QColor(0x2B2B2B);
    cv.tableAltBackgroundColor = QColor(0x2E2E2E);
    cv.tableBorderColor = QColor(0x242424);
    cv.tableSelectedColor = QColor(0x555555);
    cv.tableHeaderBackgroundColor = QColor(0x292929);
    cv.tableHeaderBorderColor = QColor(0x1F1F1F);
    cv.tableHeaderTextColor = QColor(0xEBEBEB);
    cv.tableScrollHandleColor = QColor(0xDDDDDD);
    cv.tableScrollBackgroundColor = QColor(0x404040);
    cv.tableSectionBorderLight = QColor(0xFEFEFE);
    cv.tableSectionBorderDark = QColor(0xDFDFDF);

    cv.labelTextColor = Qt::white;
    cv.labelBackgroundColor = QColor(0x2B2B2B);
    cv.hyperlinkColor = QColor(0xFFCC00);

    cv.buttonBackgroundColor = QColor(0x2E2E2E);
    cv.buttonTextColor = Qt::white;
    cv.buttonBorderColor = QColor(0x242424);

    cv.radioUncheckedColor = QColor(0xE5E5E5);

    cv.radioCheckedBackgroundColor = QColor(0xE5E5E5);
    cv.radioCheckedIndicatorColor = QColor(0x5F5F5F);

    cv.checkBoxTickColor = Qt::white;

    cv.toolButtonAccentColor = QColor(0x282828);

    cv.downArrowColor = QColor(0x9F9F9F);
    cv.upArrowColor = QColor(0x9F9F9F);

    cv.busyIndicatorColor = Qt::white;
    cv.navIconColor = Qt::white;
    cv.rowIconColor = QColor(0xE5E5E5);

    cv.t = ComicVineThemeTemplates();

    params.comicFlowColors = cf;
    params.comicVineParams = cv;

    params.helpAboutDialogParams.headingColor = QColor(0xE0E0E0);
    params.helpAboutDialogParams.linkColor = QColor(0xD4A84B);

    WhatsNewDialogParams wnp;
    wnp.backgroundColor = QColor(0x2A2A2A);
    wnp.headerTextColor = QColor(0xE0E0E0);
    wnp.versionTextColor = QColor(0x858585);
    wnp.contentTextColor = QColor(0xE0E0E0);
    wnp.linkColor = QColor(0xE8B800);
    wnp.closeButtonColor = QColor(0xDDDDDD);
    wnp.headerDecorationColor = QColor(0xE8B800);
    params.whatsNewDialogParams = wnp;

    EmptyContainerParams ec;
    ec.backgroundColor = QColor(0x2A2A2A);
    ec.titleTextColor = QColor(0xCCCCCC);
    ec.textColor = QColor(0xCCCCCC);
    ec.descriptionTextColor = QColor(0xAAAAAA);
    ec.searchIconColor = QColor(0x4C4C4C);
    ec.t = EmptyContainerThemeTemplates();
    params.emptyContainerParams = ec;

    SidebarParams sb;
    sb.backgroundColor = QColor(0x454545);
    sb.separatorColor = QColor(0xBDBFBF);
    sb.sectionSeparatorColor = QColor(0x575757);
    sb.uppercaseLabels = true;
    sb.titleTextColor = QColor(0xBDBFBF);
    sb.titleDropShadowColor = Qt::black;
    sb.busyIndicatorColor = Qt::white;
    params.sidebarParams = sb;

    ImportWidgetParams iw;
    iw.backgroundColor = QColor(0x2A2A2A);
    iw.titleTextColor = QColor(0xCCCCCC);
    iw.descriptionTextColor = QColor(0xAAAAAA);
    iw.currentComicTextColor = QColor(0xAAAAAA);
    iw.coversViewBackgroundColor = QColor(0x3A3A3A);
    iw.coversLabelColor = QColor(0xAAAAAA);
    iw.coversDecorationBgColor = QColor(0x3A3A3A);
    iw.coversDecorationShadowColor = QColor(0x1A1A1A);
    iw.modeIconColor = QColor(0x4A4A4A);
    iw.iconColor = QColor(0xCCCCCC);
    iw.iconCheckedColor = QColor(0xAAAAAA);
    params.importWidgetParams = iw;

    TreeViewParams tv;
    tv.textColor = QColor(0xDDDFDF);
    tv.selectionBackgroundColor = QColor(0x2E2E2E);
    tv.scrollBackgroundColor = QColor(0x404040);
    tv.scrollHandleColor = QColor(0xDDDDDD);
    tv.selectedTextColor = Qt::white;
    tv.folderIndicatorColor = QColor(237, 197, 24);
    params.treeViewParams = tv;

    TableViewParams tav;
    tav.alternateBackgroundColor = QColor(0x2E2E2E);
    tav.backgroundColor = QColor(0x2A2A2A);
    tav.cornerButtonBackgroundColor = QColor(0x2A2A2A);
    tav.cornerButtonBorderColor = QColor(0x1F1F1F);
    tav.cornerButtonGradientColor = QColor(0x252525);
    tav.itemBorderBottomColor = QColor(0x1F1F1F);
    tav.itemBorderTopColor = QColor(0x353535);
    tav.itemTextColor = QColor(0xDDDDDD);
    tav.selectedColor = QColor(0x555555);
    tav.selectedTextColor = QColor(0xFFFFFF);
    tav.headerTextColor = QColor(0xDDDDDD);
    tav.starRatingColor = QColor(0xE9BE0F);
    tav.starRatingSelectedColor = QColor(0xFFFFFF);
    tav.t = TableViewThemeTemplates();
    params.tableViewParams = tav;

    QmlViewParams qv;
    qv.backgroundColor = QColor(0x2A2A2A);
    qv.cellColor = QColor(0x212121);
    qv.cellColorWithBackground = QColor(0x21, 0x21, 0x21, 0x99);
    qv.selectedColor = QColor(0x121212);
    qv.selectedBorderColor = QColor(0xFFCC00);
    qv.borderColor = QColor(0x121212);
    qv.titleColor = QColor(0xFFFFFF);
    qv.textColor = QColor(0xA8A8A8);
    qv.showDropShadow = true;
    qv.infoBackgroundColor = QColor(0x2E2E2E);
    qv.infoBorderColor = QColor(0x404040);
    qv.infoShadowColor = Qt::black;
    qv.infoTextColor = QColor(0xB0B0B0);
    qv.infoTitleColor = QColor(0xFFFFFF);
    qv.ratingUnselectedColor = QColor(0x1C1C1C);
    qv.ratingSelectedColor = QColor(0xFFFFFF);
    qv.favUncheckedColor = QColor(0x1C1C1C);
    qv.favCheckedColor = QColor(0xE84852);
    qv.readTickUncheckedColor = QColor(0x1C1C1C);
    qv.readTickCheckedColor = QColor(0xE84852);
    qv.currentComicBackgroundColor = QColor(0x00, 0x00, 0x00, 0x88);
    qv.continueReadingBackgroundColor = QColor(0x00, 0x00, 0x00, 0x88);
    qv.continueReadingColor = QColor(0xFFFFFF);
    params.qmlViewParams = qv;

    MainToolbarParams mt;
    mt.backgroundColor = QColor(0x2A2A2A);
    mt.folderNameColor = QColor(0xDDDDDD);
    mt.dividerColor = QColor(0x555555);
    mt.iconColor = QColor(0xDDDDDD);
    mt.iconDisabledColor = QColor(0x666666);
    params.mainToolbarParams = mt;

    ContentSplitterParams cs;
    cs.handleColor = QColor(0x1F1F1F);
    cs.horizontalHandleHeight = 4;
    cs.verticalHandleWidth = 4;
    params.contentSplitterParams = cs;

    SidebarIconsParams si;
    si.iconColor = QColor(0xE0E0E0);
    si.shadowColor = QColor(0xFF000000);
    si.extraColor = QColor(0x222222);
    si.useSystemFolderIcons = false;
    params.sidebarIconsParams = si;

    LibraryItemParams li;
    li.textColor = QColor(0xDDDFDF);
    li.selectedTextColor = Qt::white;
    li.selectedBackgroundColor = QColor(0x2E2E2E);
    li.libraryIconSelectedColor = Qt::white;
    li.libraryOptionsIconColor = Qt::white;
    params.libraryItemParams = li;

    ComicsViewToolbarParams cvt;
    cvt.backgroundColor = QColor(0x2A2A2A);
    cvt.separatorColor = QColor(0x444444);
    cvt.checkedBackgroundColor = QColor(0x555555);
    cvt.iconColor = QColor(0xDDDDDD);
    params.comicsViewToolbarParams = cvt;

    SearchLineEditParams sle;
    sle.textColor = QColor(0xABABAB);
    sle.backgroundColor = QColor(0x404040);
    sle.iconColor = QColor(0xF7F7F7);
    params.searchLineEditParams = sle;

    ReadingListIconsParams rli;
    rli.labelColors = {
        { "red", QColor(0xf67a7b) },
        { "orange", QColor(0xf5c240) },
        { "yellow", QColor(0xf2e446) },
        { "green", QColor(0xade738) },
        { "cyan", QColor(0xa0fddb) },
        { "blue", QColor(0x82c7ff) },
        { "violet", QColor(0x8f95ff) },
        { "purple", QColor(0xd692fc) },
        { "pink", QColor(0xfd9cda) },
        { "white", QColor(0xfcfcfc) },
        { "light", QColor(0xcbcbcb) },
        { "dark", QColor(0xb7b7b7) }
    };
    rli.labelShadowColor = Qt::black;
    rli.readingListMainColor = QColor(0xe7e7e7);
    rli.favoritesMainColor = QColor(0xe15055);
    rli.currentlyReadingMainColor = QColor(0xffcc00);
    rli.currentlyReadingOuterColor = Qt::black;
    rli.specialListShadowColor = Qt::black;
    rli.listMainColor = QColor(0xe7e7e7);
    rli.listShadowColor = Qt::black;
    rli.listDetailColor = QColor(0x464646);
    params.readingListIconsParams = rli;

    MenuIconsParams mi;
    mi.iconColor = QColor(0xF7F7F7);
    params.menuIconsParams = mi;

    DialogIconsParams dip;
    dip.iconColor = mi.iconColor;
    params.dialogIconsParams = dip;

    ShortcutsIconsParams sci;
    sci.iconColor = QColor(0xF7F7F7);
    params.shortcutsIconsParams = sci;

    ServerConfigDialogParams scd;
    scd.backgroundColor = QColor(0x2A2A2A);
    scd.titleTextColor = QColor(0xD0D0D0);
    scd.qrMessageTextColor = QColor(0xA3A3A3);
    scd.propagandaTextColor = QColor(0xB0B0B0);
    scd.labelTextColor = QColor(0xC0C0C0);
    scd.checkBoxTextColor = QColor(0xDDDDDD);
    scd.qrBackgroundColor = QColor(0x2A2A2A);
    scd.qrForegroundColor = Qt::white;
    scd.decorationColor = QColor(0xF7F7F7);
    params.serverConfigDialogParams = scd;

    return params;
}
