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
    QColor tableHeaderGradientColor;
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

    // Branch indicator icon colors (independent of sidebarIcons.iconColor)
    QColor branchIndicatorColor;
    QColor branchIndicatorSelectedColor;

    // Folder icon colors (own parameters, independent of sidebarIcons)
    QColor folderIconColor; // Main color for unselected folder (#f0f)
    QColor folderIconShadowColor; // Shadow color for unselected folder (#0ff)
    QColor folderIconSelectedColor; // Main color for selected folder (#f0f)
    QColor folderIconSelectedShadowColor; // Shadow color for selected folder (#0ff)
    QColor folderReadOverlayColor; // Tick/checkmark color for unselected folder (#ff0 in folder_finished, #f0f in folder_read_overlay.svg)
    QColor folderReadOverlaySelectedColor; // Tick/checkmark color for selected folder
};

struct TableViewParams {
    TableViewThemeTemplates t;

    QColor alternateBackgroundColor;
    QColor backgroundColor;
    QColor headerBackgroundColor;
    QColor headerBorderColor;
    QColor headerGradientColor;
    QColor itemBorderBottomColor;
    QColor itemBorderTopColor;
    int itemBorderBottomWidth; // px
    int itemBorderTopWidth; // px
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

    // Blur overlay background (FlowView always, GridView when background image enabled)
    QColor backgroundBlurOverlayColor;
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

    // When true, use QFileIconProvider for folder icons and overlay a tick for finished folders
    bool useSystemFolderIcons;
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
    QColor libraryIconColor;
    QColor libraryIconShadowColor;
    QColor libraryIconSelectedColor;
    QColor libraryOptionsIconColor; // Color for the options icon (shown only when selected)
};

struct ComicsViewToolbarParams {
    ComicsViewToolbarThemeTemplates t;

    QColor backgroundColor;
    QColor separatorColor;
    QColor checkedBackgroundColor;
    QColor iconColor;
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
    QColor labelShadowSelectedColor; // Shadow color for labels when selected/hovered

    // Special list icon colors
    QColor readingListMainColor; // default_0 main color (replaces #f0f)
    QColor readingListMainSelectedColor; // default_0 main color when selected/hovered
    QColor favoritesMainColor; // default_1 main color (replaces #f0f)
    QColor favoritesMainSelectedColor; // default_1 main color when selected/hovered
    QColor currentlyReadingMainColor; // default_2 main color (replaces #f0f)
    QColor currentlyReadingMainSelectedColor; // default_2 main color when selected/hovered
    QColor currentlyReadingOuterColor; // default_2 outer circle (replaces #ff0)
    QColor currentlyReadingOuterSelectedColor; // default_2 outer circle when selected/hovered
    QColor specialListShadowColor; // Shadow color for special lists (replaces #0ff)
    QColor specialListShadowSelectedColor; // Shadow color for special lists when selected/hovered

    // List icon colors
    QColor listMainColor; // main color (replaces #f0f)
    QColor listMainSelectedColor; // main color when selected/hovered
    QColor listShadowColor; // shadow color (replaces #0ff)
    QColor listShadowSelectedColor; // shadow color when selected/hovered
    QColor listDetailColor; // detail/checkbox color (replaces #ff0)
    QColor listDetailSelectedColor; // detail/checkbox color when selected/hovered
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
    ThemeMeta meta;
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
        return recoloredSvgToThemeFile(path, color, params.meta.id);
    };

    theme.comicVine.defaultLabelQSS = t.defaultLabelQSS.arg(cv.labelTextColor.name());
    theme.comicVine.titleLabelQSS = t.titleLabelQSS.arg(cv.labelTextColor.name());
    theme.comicVine.coverLabelQSS = t.coverLabelQSS.arg(cv.labelBackgroundColor.name(), cv.labelTextColor.name());
    theme.comicVine.radioButtonQSS = t.radioButtonQSS.arg(cv.buttonTextColor.name(), recolor(":/images/comic_vine/radioUnchecked.svg", cv.radioUncheckedColor), recoloredSvgToThemeFile(":/images/comic_vine/radioChecked.svg", cv.radioCheckedBackgroundColor, cv.radioCheckedIndicatorColor, params.meta.id));
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
                                                       recolor(":/images/comic_vine/upArrow.svg", cv.upArrowColor),
                                                       cv.tableHeaderGradientColor.name());

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
    theme.whatsNewDialog.closeButtonIcon = QPixmap(recoloredSvgToThemeFile(":/images/custom_dialog/custom_close_button.svg", wn.closeButtonColor, params.meta.id));
    theme.whatsNewDialog.headerDecoration = QPixmap(recoloredSvgToThemeFile(":/images/whats_new/whatsnew_header.svg", wn.headerDecorationColor, params.meta.id));
    // end WhatsNewDialog

    // EmptyContainer
    const auto &ec = params.emptyContainerParams;
    const auto &ect = ec.t;

    theme.emptyContainer.backgroundColor = ec.backgroundColor;
    theme.emptyContainer.titleLabelQSS = ect.titleLabelQSS.arg(ec.titleTextColor.name());
    theme.emptyContainer.textColor = ec.textColor;
    theme.emptyContainer.descriptionTextColor = ec.descriptionTextColor;
    theme.emptyContainer.noLibrariesIcon = renderSvgToPixmap(recoloredSvgToThemeFile(":/images/noLibrariesIcon.svg", ec.searchIconColor, params.meta.id), 165, 160, qApp->devicePixelRatio());
    {
        const qreal dpr = qApp->devicePixelRatio();
        theme.emptyContainer.searchingIcon = renderSvgToPixmap(recoloredSvgToThemeFile(":/images/search_result.svg", ec.searchIconColor, params.meta.id, { .suffix = "_searching" }), 97, dpr);
        theme.emptyContainer.noSearchResultsIcon = renderSvgToPixmap(recoloredSvgToThemeFile(":/images/search_result.svg", ec.searchIconColor, params.meta.id, { .suffix = "_no_results" }), 239, dpr);

        theme.emptyContainer.emptyFolderIcon = renderSvgToPixmap(recoloredSvgToThemeFile(":/images/empty_container/empty_folder.svg", ec.searchIconColor, params.meta.id), 319, 243, dpr);
        theme.emptyContainer.emptyFavoritesIcon = renderSvgToPixmap(recoloredSvgToThemeFile(":/images/empty_container/empty_favorites.svg", QColor(0xe84853), params.meta.id), 238, 223, dpr);
        theme.emptyContainer.emptyCurrentReadingsIcon = renderSvgToPixmap(recoloredSvgToThemeFile(":/images/empty_container/empty_current_readings.svg", ec.searchIconColor, params.meta.id), 167, 214, dpr);
        theme.emptyContainer.emptyReadingListIcon = renderSvgToPixmap(recoloredSvgToThemeFile(":/images/empty_container/empty_reading_list.svg", ec.searchIconColor, params.meta.id), 248, 187, dpr);

        // Generate empty label icons for each label color
        const auto &rli = params.readingListIconsParams;
        for (int c = YACReader::YRed; c <= YACReader::YDark; ++c) {
            auto labelColor = static_cast<YACReader::LabelColors>(c);
            auto colorName = YACReader::colorToName(labelColor);
            auto it = rli.labelColors.find(colorName);
            if (it != rli.labelColors.end()) {
                theme.emptyContainer.emptyLabelIcons[c] = renderSvgToPixmap(
                        recoloredSvgToThemeFile(":/images/empty_container/empty_label.svg", it.value(), params.meta.id, { .suffix = "_" + colorName }), 243, 243, dpr);
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
    theme.importWidget.topCoversDecoration = QPixmap(recoloredSvgToThemeFile(":/images/import/importTopCoversDecoration.svg", iw.coversDecorationBgColor, iw.coversDecorationShadowColor, params.meta.id));
    theme.importWidget.bottomCoversDecoration = QPixmap(recoloredSvgToThemeFile(":/images/import/importBottomCoversDecoration.svg", iw.coversDecorationBgColor, iw.coversDecorationShadowColor, params.meta.id));
    theme.importWidget.importingIcon = QPixmap(recoloredSvgToThemeFile(":/images/import/importingIcon.svg", iw.modeIconColor, params.meta.id));
    theme.importWidget.updatingIcon = QPixmap(recoloredSvgToThemeFile(":/images/import/updatingIcon.svg", iw.modeIconColor, params.meta.id));
    {
        QIcon coversToggle;
        const QString normalPath = recoloredSvgToThemeFile(":/images/import/coversToggle.svg", iw.iconColor, params.meta.id);
        const QString checkedPath = recoloredSvgToThemeFile(":/images/import/coversToggle.svg", iw.iconCheckedColor, params.meta.id, { .suffix = "_checked" });
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
                                                tav.headerBackgroundColor.name(),
                                                tav.headerBorderColor.name(),
                                                tav.headerGradientColor.name(),
                                                tav.itemBorderBottomColor.name(),
                                                tav.itemBorderTopColor.name(),
                                                tav.itemTextColor.name(),
                                                tav.selectedColor.name(),
                                                tav.selectedTextColor.name(),
                                                tav.headerTextColor.name(),
                                                QString::number(tav.itemBorderBottomWidth),
                                                QString::number(tav.itemBorderTopWidth));
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
    theme.qmlView.topShadow = recoloredSvgToThemeFile(":/qml/info-top-shadow.svg", qv.infoBackgroundColor, qv.infoBorderColor, qv.infoShadowColor, params.meta.id);
    theme.qmlView.infoShadow = recoloredSvgToThemeFile(":/qml/info-shadow.svg", qv.infoBackgroundColor, qv.infoBorderColor, qv.infoShadowColor, params.meta.id);
    theme.qmlView.infoIndicator = recoloredSvgToThemeFile(":/qml/info-indicator.svg", qv.infoBackgroundColor, qv.infoBorderColor, qv.infoShadowColor, params.meta.id);
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
    theme.qmlView.backgroundBlurOverlayColor = qv.backgroundBlurOverlayColor;
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
        const QString normalPath = recoloredSvgToThemeFile(basePath, mt.iconColor, params.meta.id);
        const QString disabledPath = recoloredSvgToThemeFile(basePath, mt.iconDisabledColor, params.meta.id, { .suffix = "_disabled" });
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
        const QString path = recoloredSvgToThemeFile(basePath, si.iconColor, si.shadowColor, params.meta.id);
        QIcon icon;
        icon.addFile(path, QSize(), QIcon::Normal, QIcon::Off);
        icon.addFile(path, QSize(), QIcon::Selected, QIcon::Off);
        return icon;
    };

    // Helper for single-color icons (only #f0f main)
    auto makeSingleColorIcon = [&](const QString &basePath) {
        const QString path = recoloredSvgToThemeFile(basePath, si.iconColor, params.meta.id);
        QIcon icon;
        icon.addFile(path, QSize(), QIcon::Normal, QIcon::Off);
        icon.addFile(path, QSize(), QIcon::Selected, QIcon::Off);
        return icon;
    };

    // System folder icons flag and overlay
    theme.sidebarIcons.useSystemFolderIcons = si.useSystemFolderIcons;
    if (si.useSystemFolderIcons) {
        const QString overlayPath = recoloredSvgToThemeFile(":/images/sidebar/folder_read_overlay.svg", params.treeViewParams.folderReadOverlayColor, params.meta.id);
        theme.sidebarIcons.folderReadOverlay = QPixmap(overlayPath);
    }

    // Library icon (unselected state, two-color: #f0f main, #0ff shadow)
    {
        const auto &li = params.libraryItemParams;
        const QString libraryIconPath = recoloredSvgToThemeFile(":/images/sidebar/libraryIcon.svg", li.libraryIconColor, li.libraryIconShadowColor, params.meta.id);
        QIcon icon;
        icon.addFile(libraryIconPath, QSize(), QIcon::Normal, QIcon::Off);
        icon.addFile(libraryIconPath, QSize(), QIcon::Selected, QIcon::Off);
        theme.sidebarIcons.libraryIcon = icon;
    }

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

    // end SidebarIcons

    // LibraryItem
    const auto &li = params.libraryItemParams;
    theme.libraryItem.textColor = li.textColor;
    theme.libraryItem.selectedTextColor = li.selectedTextColor;
    theme.libraryItem.selectedBackgroundColor = li.selectedBackgroundColor;

    // Library icon when selected (uses its own color to contrast with selected background)
    const QString libraryIconSelectedPath = recoloredSvgToThemeFile(":/images/sidebar/libraryIconSelected.svg", li.libraryIconSelectedColor, params.meta.id);
    theme.libraryItem.libraryIconSelected = QIcon(libraryIconSelectedPath);

    // Library options icon (shown only when selected, uses its own color)
    const QString libraryOptionsPath = recoloredSvgToThemeFile(":/images/sidebar/libraryOptions.svg", li.libraryOptionsIconColor, params.meta.id);
    theme.libraryItem.libraryOptionsIcon = QIcon(libraryOptionsPath);
    // end LibraryItem

    // TreeView
    const auto &tv = params.treeViewParams;

    // Branch indicator icons — own colors, independent of the sidebar icon color
    theme.treeView.branchClosedIconPath = recoloredSvgToThemeFile(":/images/sidebar/branch-closed.svg", tv.branchIndicatorColor, params.meta.id);
    theme.treeView.branchOpenIconPath = recoloredSvgToThemeFile(":/images/sidebar/branch-open.svg", tv.branchIndicatorColor, params.meta.id);
    theme.treeView.branchClosedIconSelectedPath = recoloredSvgToThemeFile(":/images/sidebar/branch-closed.svg", tv.branchIndicatorSelectedColor, params.meta.id, { .suffix = "_selected" });
    theme.treeView.branchOpenIconSelectedPath = recoloredSvgToThemeFile(":/images/sidebar/branch-open.svg", tv.branchIndicatorSelectedColor, params.meta.id, { .suffix = "_selected" });

    theme.treeView.treeViewQSS = tv.t.styledTreeViewQSS
                                         .arg(tv.textColor.name(),
                                              tv.selectionBackgroundColor.name(),
                                              tv.scrollBackgroundColor.name(),
                                              tv.scrollHandleColor.name(),
                                              tv.selectedTextColor.name(),
                                              theme.treeView.branchClosedIconPath,
                                              theme.treeView.branchOpenIconPath,
                                              theme.treeView.branchClosedIconSelectedPath,
                                              theme.treeView.branchOpenIconSelectedPath);
    theme.treeView.folderIndicatorColor = tv.folderIndicatorColor;

    // Folder icon — normal and selected states with independent colors
    {
        const QString normalPath = recoloredSvgToThemeFile(":/images/sidebar/folder.svg", tv.folderIconColor, tv.folderIconShadowColor, params.meta.id);
        const QString selectedPath = recoloredSvgToThemeFile(":/images/sidebar/folder.svg", tv.folderIconSelectedColor, tv.folderIconSelectedShadowColor, params.meta.id, { .suffix = "_selected" });
        theme.treeView.folderIcon.addFile(normalPath, QSize(), QIcon::Normal, QIcon::Off);
        theme.treeView.folderIcon.addFile(selectedPath, QSize(), QIcon::Selected, QIcon::Off);
    }

    // Folder finished icon — same but with tick (#ff0) recolored independently for each state
    {
        const QString normalPath = recoloredSvgToThemeFile(":/images/sidebar/folder_finished.svg", tv.folderIconColor, tv.folderIconShadowColor, tv.folderReadOverlayColor, params.meta.id);
        const QString selectedPath = recoloredSvgToThemeFile(":/images/sidebar/folder_finished.svg", tv.folderIconSelectedColor, tv.folderIconSelectedShadowColor, tv.folderReadOverlaySelectedColor, params.meta.id, { .suffix = "_selected" });
        theme.treeView.folderFinishedIcon.addFile(normalPath, QSize(), QIcon::Normal, QIcon::Off);
        theme.treeView.folderFinishedIcon.addFile(selectedPath, QSize(), QIcon::Selected, QIcon::Off);
    }
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
        const QString path = recoloredSvgToThemeFile(basePath, cvt.iconColor, params.meta.id);
        QIcon icon;
        icon.addFile(path, QSize(), QIcon::Normal, QIcon::Off);
        icon.addFile(path, QSize(), QIcon::Selected, QIcon::Off);
        return icon;
    };

    theme.comicsViewToolbar.toolbarQSS = cvt.t.toolbarQSS.arg(
            cvt.backgroundColor.name(),
            cvt.separatorColor.name(),
            cvt.checkedBackgroundColor.name());
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
    theme.searchLineEdit.searchIcon = renderSvgToPixmap(recoloredSvgToThemeFile(":/images/iconSearchNew.svg", sle.iconColor, params.meta.id), 15, dpr);
    theme.searchLineEdit.clearIcon = renderSvgToPixmap(recoloredSvgToThemeFile(":/images/clearSearchNew.svg", sle.iconColor, params.meta.id), 12, dpr);
    // end SearchLineEdit

    // ReadingListIcons
    const auto &rli = params.readingListIconsParams;

    // Helper to create label icons from template (uses color name to generate label_<color>.svg files)
    auto makeLabelIcon = [&](const QString &colorName, const QColor &mainColor) {
        const QString normalPath = recoloredSvgToThemeFile(":/images/lists/label_template.svg", mainColor, rli.labelShadowColor, params.meta.id, { .fileName = "label_" + colorName });
        const QString selectedPath = recoloredSvgToThemeFile(":/images/lists/label_template.svg", mainColor, rli.labelShadowSelectedColor, params.meta.id, { .suffix = "_selected", .fileName = "label_" + colorName });
        QIcon icon;
        icon.addFile(normalPath, QSize(), QIcon::Normal, QIcon::Off);
        icon.addFile(selectedPath, QSize(), QIcon::Selected, QIcon::Off);
        return icon;
    };

    for (auto it = rli.labelColors.constBegin(); it != rli.labelColors.constEnd(); ++it) {
        theme.readingListIcons.labelIcons[it.key()] = makeLabelIcon(it.key(), it.value());
    }

    // Special list icons
    auto makeSpecialIcon = [&](const QString &basePath, const QColor &mainColor, const QColor &mainSelectedColor) {
        const QString normalPath = recoloredSvgToThemeFile(basePath, mainColor, rli.specialListShadowColor, params.meta.id);
        const QString selectedPath = recoloredSvgToThemeFile(basePath, mainSelectedColor, rli.specialListShadowSelectedColor, params.meta.id, { .suffix = "_selected" });
        QIcon icon;
        icon.addFile(normalPath, QSize(), QIcon::Normal, QIcon::Off);
        icon.addFile(selectedPath, QSize(), QIcon::Selected, QIcon::Off);
        return icon;
    };

    theme.readingListIcons.readingListIcon = makeSpecialIcon(":/images/lists/default_0.svg", rli.readingListMainColor, rli.readingListMainSelectedColor);
    theme.readingListIcons.favoritesIcon = makeSpecialIcon(":/images/lists/default_1.svg", rli.favoritesMainColor, rli.favoritesMainSelectedColor);

    // Currently reading has 3 colors
    {
        const QString normalPath = recoloredSvgToThemeFile(":/images/lists/default_2.svg", rli.currentlyReadingMainColor, rli.specialListShadowColor, rli.currentlyReadingOuterColor, params.meta.id);
        const QString selectedPath = recoloredSvgToThemeFile(":/images/lists/default_2.svg", rli.currentlyReadingMainSelectedColor, rli.specialListShadowSelectedColor, rli.currentlyReadingOuterSelectedColor, params.meta.id, { .suffix = "_selected" });
        QIcon icon;
        icon.addFile(normalPath, QSize(), QIcon::Normal, QIcon::Off);
        icon.addFile(selectedPath, QSize(), QIcon::Selected, QIcon::Off);
        theme.readingListIcons.currentlyReadingIcon = icon;
    }

    // List icon (3 colors)
    {
        const QString normalPath = recoloredSvgToThemeFile(":/images/lists/list.svg", rli.listMainColor, rli.listShadowColor, rli.listDetailColor, params.meta.id);
        const QString selectedPath = recoloredSvgToThemeFile(":/images/lists/list.svg", rli.listMainSelectedColor, rli.listShadowSelectedColor, rli.listDetailSelectedColor, params.meta.id, { .suffix = "_selected" });
        QIcon icon;
        icon.addFile(normalPath, QSize(), QIcon::Normal, QIcon::Off);
        icon.addFile(selectedPath, QSize(), QIcon::Selected, QIcon::Off);
        theme.readingListIcons.listIcon = icon;
    }
    // end ReadingListIcons

    // MenuIcons
    const auto &mi = params.menuIconsParams;
    auto makeMenuIcon = [&](const QString &basePath) {
        const QString path = recoloredSvgToThemeFile(basePath, mi.iconColor, params.meta.id);
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
        const QString path = recoloredSvgToThemeFile(basePath, di.iconColor, params.meta.id);
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
        const QString path = recoloredSvgToThemeFile(":/images/find_folder.svg", di.iconColor, params.meta.id);
        const qreal dpr = qApp->devicePixelRatio();
        theme.dialogIcons.findFolderIcon = QIcon(renderSvgToPixmap(path, 13, 13, dpr));
    }
    // end DialogIcons

    // ShortcutsIcons
    const auto &sci = params.shortcutsIconsParams;
    auto makeShortcutsIcon = [&](const QString &basePath) {
        const QString path = recoloredSvgToThemeFile(basePath, sci.iconColor, params.meta.id);
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
    theme.serverConfigDialog.backgroundDecoration = QPixmap(recoloredSvgToThemeFile(":/images/serverConfigBackground.svg", scd.decorationColor, params.meta.id));

    theme.meta = params.meta;

    return theme;
}

// JSON helpers ---------------------------------------------------------------

static QColor colorFromJson(const QJsonObject &obj, const QString &key, const QColor &fallback)
{
    if (!obj.contains(key))
        return fallback;
    QColor c(obj[key].toString());
    return c.isValid() ? c : fallback;
}

Theme makeTheme(const QJsonObject &json)
{
    ThemeParams p;

    if (json.contains("defaultContentBackgroundColor")) {
        QColor c(json["defaultContentBackgroundColor"].toString());
        if (c.isValid())
            p.defaultContentBackgroundColor = c;
    }

    if (json.contains("comicFlow")) {
        const auto o = json["comicFlow"].toObject();
        p.comicFlowColors.backgroundColor = colorFromJson(o, "backgroundColor", p.comicFlowColors.backgroundColor);
        p.comicFlowColors.textColor = colorFromJson(o, "textColor", p.comicFlowColors.textColor);
    }

    if (json.contains("comicVine")) {
        const auto o = json["comicVine"].toObject();
        auto &cv = p.comicVineParams;
        cv.contentTextColor = colorFromJson(o, "contentTextColor", cv.contentTextColor);
        cv.contentBackgroundColor = colorFromJson(o, "contentBackgroundColor", cv.contentBackgroundColor);
        cv.contentAltBackgroundColor = colorFromJson(o, "contentAltBackgroundColor", cv.contentAltBackgroundColor);
        cv.dialogBackgroundColor = colorFromJson(o, "dialogBackgroundColor", cv.dialogBackgroundColor);
        cv.tableBackgroundColor = colorFromJson(o, "tableBackgroundColor", cv.tableBackgroundColor);
        cv.tableAltBackgroundColor = colorFromJson(o, "tableAltBackgroundColor", cv.tableAltBackgroundColor);
        cv.tableBorderColor = colorFromJson(o, "tableBorderColor", cv.tableBorderColor);
        cv.tableSelectedColor = colorFromJson(o, "tableSelectedColor", cv.tableSelectedColor);
        cv.tableHeaderBackgroundColor = colorFromJson(o, "tableHeaderBackgroundColor", cv.tableHeaderBackgroundColor);
        cv.tableHeaderGradientColor = colorFromJson(o, "tableHeaderGradientColor", cv.tableHeaderGradientColor);
        cv.tableHeaderBorderColor = colorFromJson(o, "tableHeaderBorderColor", cv.tableHeaderBorderColor);
        cv.tableHeaderTextColor = colorFromJson(o, "tableHeaderTextColor", cv.tableHeaderTextColor);
        cv.tableScrollHandleColor = colorFromJson(o, "tableScrollHandleColor", cv.tableScrollHandleColor);
        cv.tableScrollBackgroundColor = colorFromJson(o, "tableScrollBackgroundColor", cv.tableScrollBackgroundColor);
        cv.tableSectionBorderLight = colorFromJson(o, "tableSectionBorderLight", cv.tableSectionBorderLight);
        cv.tableSectionBorderDark = colorFromJson(o, "tableSectionBorderDark", cv.tableSectionBorderDark);
        cv.labelTextColor = colorFromJson(o, "labelTextColor", cv.labelTextColor);
        cv.labelBackgroundColor = colorFromJson(o, "labelBackgroundColor", cv.labelBackgroundColor);
        cv.hyperlinkColor = colorFromJson(o, "hyperlinkColor", cv.hyperlinkColor);
        cv.buttonBackgroundColor = colorFromJson(o, "buttonBackgroundColor", cv.buttonBackgroundColor);
        cv.buttonTextColor = colorFromJson(o, "buttonTextColor", cv.buttonTextColor);
        cv.buttonBorderColor = colorFromJson(o, "buttonBorderColor", cv.buttonBorderColor);
        cv.radioUncheckedColor = colorFromJson(o, "radioUncheckedColor", cv.radioUncheckedColor);
        cv.radioCheckedBackgroundColor = colorFromJson(o, "radioCheckedBackgroundColor", cv.radioCheckedBackgroundColor);
        cv.radioCheckedIndicatorColor = colorFromJson(o, "radioCheckedIndicatorColor", cv.radioCheckedIndicatorColor);
        cv.checkBoxTickColor = colorFromJson(o, "checkBoxTickColor", cv.checkBoxTickColor);
        cv.toolButtonAccentColor = colorFromJson(o, "toolButtonAccentColor", cv.toolButtonAccentColor);
        cv.downArrowColor = colorFromJson(o, "downArrowColor", cv.downArrowColor);
        cv.upArrowColor = colorFromJson(o, "upArrowColor", cv.upArrowColor);
        cv.busyIndicatorColor = colorFromJson(o, "busyIndicatorColor", cv.busyIndicatorColor);
        cv.navIconColor = colorFromJson(o, "navIconColor", cv.navIconColor);
        cv.rowIconColor = colorFromJson(o, "rowIconColor", cv.rowIconColor);
    }

    if (json.contains("helpAboutDialog")) {
        const auto o = json["helpAboutDialog"].toObject();
        p.helpAboutDialogParams.headingColor = colorFromJson(o, "headingColor", p.helpAboutDialogParams.headingColor);
        p.helpAboutDialogParams.linkColor = colorFromJson(o, "linkColor", p.helpAboutDialogParams.linkColor);
    }

    if (json.contains("whatsNewDialog")) {
        const auto o = json["whatsNewDialog"].toObject();
        auto &wn = p.whatsNewDialogParams;
        wn.backgroundColor = colorFromJson(o, "backgroundColor", wn.backgroundColor);
        wn.headerTextColor = colorFromJson(o, "headerTextColor", wn.headerTextColor);
        wn.versionTextColor = colorFromJson(o, "versionTextColor", wn.versionTextColor);
        wn.contentTextColor = colorFromJson(o, "contentTextColor", wn.contentTextColor);
        wn.linkColor = colorFromJson(o, "linkColor", wn.linkColor);
        wn.closeButtonColor = colorFromJson(o, "closeButtonColor", wn.closeButtonColor);
        wn.headerDecorationColor = colorFromJson(o, "headerDecorationColor", wn.headerDecorationColor);
    }

    if (json.contains("emptyContainer")) {
        const auto o = json["emptyContainer"].toObject();
        auto &ec = p.emptyContainerParams;
        ec.backgroundColor = colorFromJson(o, "backgroundColor", ec.backgroundColor);
        ec.titleTextColor = colorFromJson(o, "titleTextColor", ec.titleTextColor);
        ec.textColor = colorFromJson(o, "textColor", ec.textColor);
        ec.descriptionTextColor = colorFromJson(o, "descriptionTextColor", ec.descriptionTextColor);
        ec.searchIconColor = colorFromJson(o, "searchIconColor", ec.searchIconColor);
    }

    if (json.contains("sidebar")) {
        const auto o = json["sidebar"].toObject();
        auto &sb = p.sidebarParams;
        sb.backgroundColor = colorFromJson(o, "backgroundColor", sb.backgroundColor);
        sb.separatorColor = colorFromJson(o, "separatorColor", sb.separatorColor);
        sb.sectionSeparatorColor = colorFromJson(o, "sectionSeparatorColor", sb.sectionSeparatorColor);
        if (o.contains("uppercaseLabels"))
            sb.uppercaseLabels = o["uppercaseLabels"].toBool(sb.uppercaseLabels);
        sb.titleTextColor = colorFromJson(o, "titleTextColor", sb.titleTextColor);
        sb.titleDropShadowColor = colorFromJson(o, "titleDropShadowColor", sb.titleDropShadowColor);
        sb.busyIndicatorColor = colorFromJson(o, "busyIndicatorColor", sb.busyIndicatorColor);
    }

    if (json.contains("sidebarIcons")) {
        const auto o = json["sidebarIcons"].toObject();
        auto &si = p.sidebarIconsParams;
        si.iconColor = colorFromJson(o, "iconColor", si.iconColor);
        si.shadowColor = colorFromJson(o, "shadowColor", si.shadowColor);
        if (o.contains("useSystemFolderIcons"))
            si.useSystemFolderIcons = o["useSystemFolderIcons"].toBool(si.useSystemFolderIcons);
    }

    if (json.contains("libraryItem")) {
        const auto o = json["libraryItem"].toObject();
        auto &li = p.libraryItemParams;
        li.textColor = colorFromJson(o, "textColor", li.textColor);
        li.libraryIconColor = colorFromJson(o, "libraryIconColor", li.libraryIconColor);
        li.libraryIconShadowColor = colorFromJson(o, "libraryIconShadowColor", li.libraryIconShadowColor);
        li.selectedTextColor = colorFromJson(o, "selectedTextColor", li.selectedTextColor);
        li.selectedBackgroundColor = colorFromJson(o, "selectedBackgroundColor", li.selectedBackgroundColor);
        li.libraryIconSelectedColor = colorFromJson(o, "libraryIconSelectedColor", li.libraryIconSelectedColor);
        li.libraryOptionsIconColor = colorFromJson(o, "libraryOptionsIconColor", li.libraryOptionsIconColor);
    }

    if (json.contains("importWidget")) {
        const auto o = json["importWidget"].toObject();
        auto &iw = p.importWidgetParams;
        iw.backgroundColor = colorFromJson(o, "backgroundColor", iw.backgroundColor);
        iw.titleTextColor = colorFromJson(o, "titleTextColor", iw.titleTextColor);
        iw.descriptionTextColor = colorFromJson(o, "descriptionTextColor", iw.descriptionTextColor);
        iw.currentComicTextColor = colorFromJson(o, "currentComicTextColor", iw.currentComicTextColor);
        iw.coversViewBackgroundColor = colorFromJson(o, "coversViewBackgroundColor", iw.coversViewBackgroundColor);
        iw.coversLabelColor = colorFromJson(o, "coversLabelColor", iw.coversLabelColor);
        iw.coversDecorationBgColor = colorFromJson(o, "coversDecorationBgColor", iw.coversDecorationBgColor);
        iw.coversDecorationShadowColor = colorFromJson(o, "coversDecorationShadowColor", iw.coversDecorationShadowColor);
        iw.modeIconColor = colorFromJson(o, "modeIconColor", iw.modeIconColor);
        iw.iconColor = colorFromJson(o, "iconColor", iw.iconColor);
        iw.iconCheckedColor = colorFromJson(o, "iconCheckedColor", iw.iconCheckedColor);
    }

    if (json.contains("serverConfigDialog")) {
        const auto o = json["serverConfigDialog"].toObject();
        auto &scd2 = p.serverConfigDialogParams;
        scd2.backgroundColor = colorFromJson(o, "backgroundColor", scd2.backgroundColor);
        scd2.titleTextColor = colorFromJson(o, "titleTextColor", scd2.titleTextColor);
        scd2.qrMessageTextColor = colorFromJson(o, "qrMessageTextColor", scd2.qrMessageTextColor);
        scd2.propagandaTextColor = colorFromJson(o, "propagandaTextColor", scd2.propagandaTextColor);
        scd2.labelTextColor = colorFromJson(o, "labelTextColor", scd2.labelTextColor);
        scd2.checkBoxTextColor = colorFromJson(o, "checkBoxTextColor", scd2.checkBoxTextColor);
        scd2.qrBackgroundColor = colorFromJson(o, "qrBackgroundColor", scd2.qrBackgroundColor);
        scd2.qrForegroundColor = colorFromJson(o, "qrForegroundColor", scd2.qrForegroundColor);
        scd2.decorationColor = colorFromJson(o, "decorationColor", scd2.decorationColor);
    }

    if (json.contains("mainToolbar")) {
        const auto o = json["mainToolbar"].toObject();
        auto &mt = p.mainToolbarParams;
        mt.backgroundColor = colorFromJson(o, "backgroundColor", mt.backgroundColor);
        mt.folderNameColor = colorFromJson(o, "folderNameColor", mt.folderNameColor);
        mt.dividerColor = colorFromJson(o, "dividerColor", mt.dividerColor);
        mt.iconColor = colorFromJson(o, "iconColor", mt.iconColor);
        mt.iconDisabledColor = colorFromJson(o, "iconDisabledColor", mt.iconDisabledColor);
    }

    if (json.contains("contentSplitter")) {
        const auto o = json["contentSplitter"].toObject();
        auto &cs = p.contentSplitterParams;
        cs.handleColor = colorFromJson(o, "handleColor", cs.handleColor);
        if (o.contains("horizontalHandleHeight"))
            cs.horizontalHandleHeight = o["horizontalHandleHeight"].toInt(cs.horizontalHandleHeight);
        if (o.contains("verticalHandleWidth"))
            cs.verticalHandleWidth = o["verticalHandleWidth"].toInt(cs.verticalHandleWidth);
    }

    if (json.contains("treeView")) {
        const auto o = json["treeView"].toObject();
        auto &tv = p.treeViewParams;
        tv.textColor = colorFromJson(o, "textColor", tv.textColor);
        tv.selectionBackgroundColor = colorFromJson(o, "selectionBackgroundColor", tv.selectionBackgroundColor);
        tv.scrollBackgroundColor = colorFromJson(o, "scrollBackgroundColor", tv.scrollBackgroundColor);
        tv.scrollHandleColor = colorFromJson(o, "scrollHandleColor", tv.scrollHandleColor);
        tv.selectedTextColor = colorFromJson(o, "selectedTextColor", tv.selectedTextColor);
        tv.folderIndicatorColor = colorFromJson(o, "folderIndicatorColor", tv.folderIndicatorColor);
        tv.branchIndicatorColor = colorFromJson(o, "branchIndicatorColor", tv.branchIndicatorColor);
        tv.branchIndicatorSelectedColor = colorFromJson(o, "branchIndicatorSelectedColor", tv.branchIndicatorSelectedColor);
        tv.folderIconColor = colorFromJson(o, "folderIconColor", tv.folderIconColor);
        tv.folderIconShadowColor = colorFromJson(o, "folderIconShadowColor", tv.folderIconShadowColor);
        tv.folderIconSelectedColor = colorFromJson(o, "folderIconSelectedColor", tv.folderIconSelectedColor);
        tv.folderIconSelectedShadowColor = colorFromJson(o, "folderIconSelectedShadowColor", tv.folderIconSelectedShadowColor);
        tv.folderReadOverlayColor = colorFromJson(o, "folderReadOverlayColor", tv.folderReadOverlayColor);
        tv.folderReadOverlaySelectedColor = colorFromJson(o, "folderReadOverlaySelectedColor", tv.folderReadOverlaySelectedColor);
    }

    if (json.contains("tableView")) {
        const auto o = json["tableView"].toObject();
        auto &tbv = p.tableViewParams;
        tbv.alternateBackgroundColor = colorFromJson(o, "alternateBackgroundColor", tbv.alternateBackgroundColor);
        tbv.backgroundColor = colorFromJson(o, "backgroundColor", tbv.backgroundColor);
        tbv.headerBackgroundColor = colorFromJson(o, "headerBackgroundColor", tbv.headerBackgroundColor);
        tbv.headerBorderColor = colorFromJson(o, "headerBorderColor", tbv.headerBorderColor);
        tbv.headerGradientColor = colorFromJson(o, "headerGradientColor", tbv.headerGradientColor);
        tbv.itemBorderBottomColor = colorFromJson(o, "itemBorderBottomColor", tbv.itemBorderBottomColor);
        tbv.itemBorderTopColor = colorFromJson(o, "itemBorderTopColor", tbv.itemBorderTopColor);
        tbv.itemBorderBottomWidth = o["itemBorderBottomWidth"].toInt(tbv.itemBorderBottomWidth);
        tbv.itemBorderTopWidth = o["itemBorderTopWidth"].toInt(tbv.itemBorderTopWidth);
        tbv.itemTextColor = colorFromJson(o, "itemTextColor", tbv.itemTextColor);
        tbv.selectedColor = colorFromJson(o, "selectedColor", tbv.selectedColor);
        tbv.selectedTextColor = colorFromJson(o, "selectedTextColor", tbv.selectedTextColor);
        tbv.headerTextColor = colorFromJson(o, "headerTextColor", tbv.headerTextColor);
        tbv.starRatingColor = colorFromJson(o, "starRatingColor", tbv.starRatingColor);
        tbv.starRatingSelectedColor = colorFromJson(o, "starRatingSelectedColor", tbv.starRatingSelectedColor);
    }

    if (json.contains("qmlView")) {
        const auto o = json["qmlView"].toObject();
        auto &qv = p.qmlViewParams;
        qv.backgroundColor = colorFromJson(o, "backgroundColor", qv.backgroundColor);
        qv.cellColor = colorFromJson(o, "cellColor", qv.cellColor);
        qv.cellColorWithBackground = colorFromJson(o, "cellColorWithBackground", qv.cellColorWithBackground);
        qv.selectedColor = colorFromJson(o, "selectedColor", qv.selectedColor);
        qv.selectedBorderColor = colorFromJson(o, "selectedBorderColor", qv.selectedBorderColor);
        qv.borderColor = colorFromJson(o, "borderColor", qv.borderColor);
        qv.titleColor = colorFromJson(o, "titleColor", qv.titleColor);
        qv.textColor = colorFromJson(o, "textColor", qv.textColor);
        if (o.contains("showDropShadow"))
            qv.showDropShadow = o["showDropShadow"].toBool(qv.showDropShadow);
        qv.infoBackgroundColor = colorFromJson(o, "infoBackgroundColor", qv.infoBackgroundColor);
        qv.infoBorderColor = colorFromJson(o, "infoBorderColor", qv.infoBorderColor);
        qv.infoShadowColor = colorFromJson(o, "infoShadowColor", qv.infoShadowColor);
        qv.infoTextColor = colorFromJson(o, "infoTextColor", qv.infoTextColor);
        qv.infoTitleColor = colorFromJson(o, "infoTitleColor", qv.infoTitleColor);
        qv.ratingUnselectedColor = colorFromJson(o, "ratingUnselectedColor", qv.ratingUnselectedColor);
        qv.ratingSelectedColor = colorFromJson(o, "ratingSelectedColor", qv.ratingSelectedColor);
        qv.favUncheckedColor = colorFromJson(o, "favUncheckedColor", qv.favUncheckedColor);
        qv.favCheckedColor = colorFromJson(o, "favCheckedColor", qv.favCheckedColor);
        qv.readTickUncheckedColor = colorFromJson(o, "readTickUncheckedColor", qv.readTickUncheckedColor);
        qv.readTickCheckedColor = colorFromJson(o, "readTickCheckedColor", qv.readTickCheckedColor);
        qv.currentComicBackgroundColor = colorFromJson(o, "currentComicBackgroundColor", qv.currentComicBackgroundColor);
        qv.continueReadingBackgroundColor = colorFromJson(o, "continueReadingBackgroundColor", qv.continueReadingBackgroundColor);
        qv.continueReadingColor = colorFromJson(o, "continueReadingColor", qv.continueReadingColor);
        qv.backgroundBlurOverlayColor = colorFromJson(o, "backgroundBlurOverlayColor", qv.backgroundBlurOverlayColor);
    }

    if (json.contains("comicsViewToolbar")) {
        const auto o = json["comicsViewToolbar"].toObject();
        auto &cvt = p.comicsViewToolbarParams;
        cvt.backgroundColor = colorFromJson(o, "backgroundColor", cvt.backgroundColor);
        cvt.separatorColor = colorFromJson(o, "separatorColor", cvt.separatorColor);
        cvt.checkedBackgroundColor = colorFromJson(o, "checkedBackgroundColor", cvt.checkedBackgroundColor);
        cvt.iconColor = colorFromJson(o, "iconColor", cvt.iconColor);
    }

    if (json.contains("searchLineEdit")) {
        const auto o = json["searchLineEdit"].toObject();
        auto &sle = p.searchLineEditParams;
        sle.textColor = colorFromJson(o, "textColor", sle.textColor);
        sle.backgroundColor = colorFromJson(o, "backgroundColor", sle.backgroundColor);
        sle.iconColor = colorFromJson(o, "iconColor", sle.iconColor);
    }

    if (json.contains("readingListIcons")) {
        const auto o = json["readingListIcons"].toObject();
        auto &rli = p.readingListIconsParams;
        if (o.contains("labelColors")) {
            const auto lc = o["labelColors"].toObject();
            for (auto it = lc.constBegin(); it != lc.constEnd(); ++it) {
                QColor c(it.value().toString());
                if (c.isValid())
                    rli.labelColors[it.key()] = c;
            }
        }
        rli.labelShadowColor = colorFromJson(o, "labelShadowColor", rli.labelShadowColor);
        rli.labelShadowSelectedColor = colorFromJson(o, "labelShadowSelectedColor", rli.labelShadowSelectedColor);
        rli.readingListMainColor = colorFromJson(o, "readingListMainColor", rli.readingListMainColor);
        rli.readingListMainSelectedColor = colorFromJson(o, "readingListMainSelectedColor", rli.readingListMainSelectedColor);
        rli.favoritesMainColor = colorFromJson(o, "favoritesMainColor", rli.favoritesMainColor);
        rli.favoritesMainSelectedColor = colorFromJson(o, "favoritesMainSelectedColor", rli.favoritesMainSelectedColor);
        rli.currentlyReadingMainColor = colorFromJson(o, "currentlyReadingMainColor", rli.currentlyReadingMainColor);
        rli.currentlyReadingMainSelectedColor = colorFromJson(o, "currentlyReadingMainSelectedColor", rli.currentlyReadingMainSelectedColor);
        rli.currentlyReadingOuterColor = colorFromJson(o, "currentlyReadingOuterColor", rli.currentlyReadingOuterColor);
        rli.currentlyReadingOuterSelectedColor = colorFromJson(o, "currentlyReadingOuterSelectedColor", rli.currentlyReadingOuterSelectedColor);
        rli.specialListShadowColor = colorFromJson(o, "specialListShadowColor", rli.specialListShadowColor);
        rli.specialListShadowSelectedColor = colorFromJson(o, "specialListShadowSelectedColor", rli.specialListShadowSelectedColor);
        rli.listMainColor = colorFromJson(o, "listMainColor", rli.listMainColor);
        rli.listMainSelectedColor = colorFromJson(o, "listMainSelectedColor", rli.listMainSelectedColor);
        rli.listShadowColor = colorFromJson(o, "listShadowColor", rli.listShadowColor);
        rli.listShadowSelectedColor = colorFromJson(o, "listShadowSelectedColor", rli.listShadowSelectedColor);
        rli.listDetailColor = colorFromJson(o, "listDetailColor", rli.listDetailColor);
        rli.listDetailSelectedColor = colorFromJson(o, "listDetailSelectedColor", rli.listDetailSelectedColor);
    }

    if (json.contains("dialogIcons")) {
        const auto o = json["dialogIcons"].toObject();
        p.dialogIconsParams.iconColor = colorFromJson(o, "iconColor", p.dialogIconsParams.iconColor);
    }

    if (json.contains("menuIcons")) {
        const auto o = json["menuIcons"].toObject();
        p.menuIconsParams.iconColor = colorFromJson(o, "iconColor", p.menuIconsParams.iconColor);
    }

    if (json.contains("shortcutsIcons")) {
        const auto o = json["shortcutsIcons"].toObject();
        p.shortcutsIconsParams.iconColor = colorFromJson(o, "iconColor", p.shortcutsIconsParams.iconColor);
    }

    if (json.contains("meta")) {
        const auto o = json["meta"].toObject();
        p.meta.id = o["id"].toString(p.meta.id);
        p.meta.displayName = o["displayName"].toString(p.meta.displayName);
        const QString variantStr = o["variant"].toString();
        if (variantStr == "light")
            p.meta.variant = ThemeVariant::Light;
        else if (variantStr == "dark")
            p.meta.variant = ThemeVariant::Dark;
    }

    Theme theme = makeTheme(p);
    theme.sourceJson = json;
    return theme;
}
