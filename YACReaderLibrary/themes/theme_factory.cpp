#include "theme_factory.h"

#include <QApplication>

#include "icon_utils.h"
#include "yacreader_global.h"

struct MetadataScraperDialogParams {
    MetadataScraperDialogThemeTemplates t;

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

struct NavigationTreeParams {
    NavigationTreeThemeTemplates t;

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

struct ComicsViewTableParams {
    ComicsViewTableThemeTemplates t;

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

struct GridAndInfoViewParams {
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

    ComicFlowColors comicFlowColors;
    MetadataScraperDialogParams metadataScraperDialogParams;
    HelpAboutDialogTheme helpAboutDialogParams;
    EmptyContainerParams emptyContainerParams;
    SidebarParams sidebarParams;
    SidebarIconsParams sidebarIconsParams;
    LibraryItemParams libraryItemParams;
    ImportWidgetParams importWidgetParams;
    ServerConfigDialogParams serverConfigDialogParams;
    MainToolbarParams mainToolbarParams;
    ContentSplitterParams contentSplitterParams;
    NavigationTreeParams navigationTreeParams;
    ComicsViewTableParams comicsViewTableParams;
    GridAndInfoViewParams gridAndInfoViewParams;
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

    // Comic Flow
    const auto &cf = params.comicFlowColors;
    theme.comicFlow.backgroundColor = cf.backgroundColor;
    theme.comicFlow.textColor = cf.textColor;

    // MetadataScraperDialog
    const auto &msd = params.metadataScraperDialogParams;
    const auto &t = msd.t;

    auto recolor = [&](const QString &path, const QColor &color) {
        return recoloredSvgToThemeFile(path, color, params.meta.id);
    };

    theme.metadataScraperDialog.defaultLabelQSS = t.defaultLabelQSS.arg(msd.labelTextColor.name());
    theme.metadataScraperDialog.titleLabelQSS = t.titleLabelQSS.arg(msd.labelTextColor.name());
    theme.metadataScraperDialog.coverLabelQSS = t.coverLabelQSS.arg(msd.labelBackgroundColor.name(), msd.labelTextColor.name());
    theme.metadataScraperDialog.radioButtonQSS = t.radioButtonQSS.arg(msd.buttonTextColor.name(), recolor(":/images/comic_vine/radioUnchecked.svg", msd.radioUncheckedColor), recoloredSvgToThemeFile(":/images/comic_vine/radioChecked.svg", msd.radioCheckedBackgroundColor, msd.radioCheckedIndicatorColor, params.meta.id));
    theme.metadataScraperDialog.checkBoxQSS = t.checkBoxQSS.arg(msd.buttonTextColor.name(), msd.buttonBorderColor.name(), msd.buttonBackgroundColor.name(), recolor(":/images/comic_vine/checkBoxTick.svg", msd.checkBoxTickColor));

    theme.metadataScraperDialog.scraperLineEditTitleLabelQSS = t.scraperLineEditTitleLabelQSS.arg(msd.contentTextColor.name());
    theme.metadataScraperDialog.scraperLineEditQSS = t.scraperLineEditQSS.arg(msd.contentAltBackgroundColor.name(), msd.contentTextColor.name(), "%1");

    theme.metadataScraperDialog.scraperToolButtonQSS = t.scraperToolButtonQSS.arg(msd.buttonBackgroundColor.name(), msd.buttonTextColor.name(), msd.toolButtonAccentColor.name());
    theme.metadataScraperDialog.scraperToolButtonSeparatorQSS = t.scraperToolButtonSeparatorQSS.arg(msd.toolButtonAccentColor.name());
    theme.metadataScraperDialog.scraperToolButtonFillColor = msd.buttonBackgroundColor;

    theme.metadataScraperDialog.scraperScrollLabelTextQSS = t.scraperScrollLabelTextQSS.arg(msd.contentBackgroundColor.name(), msd.contentTextColor.name(), msd.hyperlinkColor.name());
    theme.metadataScraperDialog.scraperScrollLabelScrollAreaQSS = t.scraperScrollLabelScrollAreaQSS.arg(msd.contentBackgroundColor.name(), msd.tableScrollHandleColor.name(), msd.tableScrollBackgroundColor.name());

    theme.metadataScraperDialog.scraperTableViewQSS = t.scraperTableViewQSS
                                                              .arg(msd.tableHeaderTextColor.name(),
                                                                   msd.tableAltBackgroundColor.name(),
                                                                   msd.tableBackgroundColor.name(),
                                                                   msd.tableSelectedColor.name(),
                                                                   msd.tableHeaderBackgroundColor.name(),
                                                                   msd.tableHeaderBorderColor.name(),
                                                                   msd.tableHeaderTextColor.name(),
                                                                   msd.tableSectionBorderDark.name(),
                                                                   msd.tableSectionBorderLight.name(),
                                                                   msd.tableScrollHandleColor.name(),
                                                                   msd.tableScrollBackgroundColor.name(),
                                                                   recolor(":/images/comic_vine/downArrow.svg", msd.downArrowColor),
                                                                   recolor(":/images/comic_vine/upArrow.svg", msd.upArrowColor),
                                                                   msd.tableHeaderGradientColor.name());

    theme.metadataScraperDialog.dialogQSS = t.dialogQSS.arg(msd.dialogBackgroundColor.name());
    theme.metadataScraperDialog.dialogButtonsQSS = t.dialogButtonsQSS.arg(msd.buttonBorderColor.name(), msd.buttonBackgroundColor.name(), msd.buttonTextColor.name());

    theme.metadataScraperDialog.busyIndicatorColor = msd.busyIndicatorColor;

    theme.metadataScraperDialog.nextPageIcon = { QIcon(recolor(t.nextPageIcon, msd.navIconColor)), t.pageIconSize };
    theme.metadataScraperDialog.previousPageIcon = { QIcon(recolor(t.previousPageIcon, msd.navIconColor)), t.pageIconSize };

    theme.metadataScraperDialog.rowUpIcon = { QIcon(recolor(t.rowUpIcon, msd.rowIconColor)), t.rowIconSize };
    theme.metadataScraperDialog.rowDownIcon = { QIcon(recolor(t.rowDownIcon, msd.rowIconColor)), t.rowIconSize };

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

    // ComicsViewTable
    const auto &cvta = params.comicsViewTableParams;
    theme.comicsViewTable.tableViewQSS = cvta.t.tableViewQSS
                                                 .arg(cvta.alternateBackgroundColor.name(),
                                                      cvta.backgroundColor.name(),
                                                      cvta.headerBackgroundColor.name(),
                                                      cvta.headerBorderColor.name(),
                                                      cvta.headerGradientColor.name(),
                                                      cvta.itemBorderBottomColor.name(),
                                                      cvta.itemBorderTopColor.name(),
                                                      cvta.itemTextColor.name(),
                                                      cvta.selectedColor.name(),
                                                      cvta.selectedTextColor.name(),
                                                      cvta.headerTextColor.name(),
                                                      QString::number(cvta.itemBorderBottomWidth),
                                                      QString::number(cvta.itemBorderTopWidth));
    theme.comicsViewTable.starRatingColor = cvta.starRatingColor;
    theme.comicsViewTable.starRatingSelectedColor = cvta.starRatingSelectedColor;
    // end ComicsViewTable

    // GridAndInfoView
    const auto &giv = params.gridAndInfoViewParams;
    theme.gridAndInfoView.backgroundColor = giv.backgroundColor;
    theme.gridAndInfoView.cellColor = giv.cellColor;
    theme.gridAndInfoView.cellColorWithBackground = giv.cellColorWithBackground;
    theme.gridAndInfoView.selectedColor = giv.selectedColor;
    theme.gridAndInfoView.selectedBorderColor = giv.selectedBorderColor;
    theme.gridAndInfoView.borderColor = giv.borderColor;
    theme.gridAndInfoView.titleColor = giv.titleColor;
    theme.gridAndInfoView.textColor = giv.textColor;
    theme.gridAndInfoView.showDropShadow = giv.showDropShadow;
    theme.gridAndInfoView.infoBackgroundColor = giv.infoBackgroundColor;
    theme.gridAndInfoView.topShadow = recoloredSvgToThemeFile(":/qml/info-top-shadow.svg", giv.infoBackgroundColor, giv.infoBorderColor, giv.infoShadowColor, params.meta.id);
    theme.gridAndInfoView.infoShadow = recoloredSvgToThemeFile(":/qml/info-shadow.svg", giv.infoBackgroundColor, giv.infoBorderColor, giv.infoShadowColor, params.meta.id);
    theme.gridAndInfoView.infoIndicator = recoloredSvgToThemeFile(":/qml/info-indicator.svg", giv.infoBackgroundColor, giv.infoBorderColor, giv.infoShadowColor, params.meta.id);
    theme.gridAndInfoView.infoTextColor = giv.infoTextColor;
    theme.gridAndInfoView.infoTitleColor = giv.infoTitleColor;
    theme.gridAndInfoView.ratingUnselectedColor = giv.ratingUnselectedColor;
    theme.gridAndInfoView.ratingSelectedColor = giv.ratingSelectedColor;
    theme.gridAndInfoView.favUncheckedColor = giv.favUncheckedColor;
    theme.gridAndInfoView.favCheckedColor = giv.favCheckedColor;
    theme.gridAndInfoView.readTickUncheckedColor = giv.readTickUncheckedColor;
    theme.gridAndInfoView.readTickCheckedColor = giv.readTickCheckedColor;
    theme.gridAndInfoView.currentComicBackgroundColor = giv.currentComicBackgroundColor;
    theme.gridAndInfoView.continueReadingBackgroundColor = giv.continueReadingBackgroundColor;
    theme.gridAndInfoView.continueReadingColor = giv.continueReadingColor;
    theme.gridAndInfoView.backgroundBlurOverlayColor = giv.backgroundBlurOverlayColor;
    // end GridAndInfoView

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
        const QString overlayPath = recoloredSvgToThemeFile(":/images/sidebar/folder_read_overlay.svg", params.navigationTreeParams.folderReadOverlayColor, params.meta.id);
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

    // NavigationTree
    const auto &nt = params.navigationTreeParams;

    // Branch indicator icons — own colors, independent of the sidebar icon color
    theme.navigationTree.branchClosedIconPath = recoloredSvgToThemeFile(":/images/sidebar/branch-closed.svg", nt.branchIndicatorColor, params.meta.id);
    theme.navigationTree.branchOpenIconPath = recoloredSvgToThemeFile(":/images/sidebar/branch-open.svg", nt.branchIndicatorColor, params.meta.id);
    theme.navigationTree.branchClosedIconSelectedPath = recoloredSvgToThemeFile(":/images/sidebar/branch-closed.svg", nt.branchIndicatorSelectedColor, params.meta.id, { .suffix = "_selected" });
    theme.navigationTree.branchOpenIconSelectedPath = recoloredSvgToThemeFile(":/images/sidebar/branch-open.svg", nt.branchIndicatorSelectedColor, params.meta.id, { .suffix = "_selected" });

    theme.navigationTree.navigationTreeQSS = nt.t.navigationTreeQSS
                                                     .arg(nt.textColor.name(),
                                                          nt.selectionBackgroundColor.name(),
                                                          nt.scrollBackgroundColor.name(),
                                                          nt.scrollHandleColor.name(),
                                                          nt.selectedTextColor.name(),
                                                          theme.navigationTree.branchClosedIconPath,
                                                          theme.navigationTree.branchOpenIconPath,
                                                          theme.navigationTree.branchClosedIconSelectedPath,
                                                          theme.navigationTree.branchOpenIconSelectedPath);
    theme.navigationTree.folderIndicatorColor = nt.folderIndicatorColor;

    // Folder icon — normal and selected states with independent colors
    {
        const QString normalPath = recoloredSvgToThemeFile(":/images/sidebar/folder.svg", nt.folderIconColor, nt.folderIconShadowColor, params.meta.id);
        const QString selectedPath = recoloredSvgToThemeFile(":/images/sidebar/folder.svg", nt.folderIconSelectedColor, nt.folderIconSelectedShadowColor, params.meta.id, { .suffix = "_selected" });
        theme.navigationTree.folderIcon.addFile(normalPath, QSize(), QIcon::Normal, QIcon::Off);
        theme.navigationTree.folderIcon.addFile(selectedPath, QSize(), QIcon::Selected, QIcon::Off);
    }

    // Folder finished icon — same but with tick (#ff0) recolored independently for each state
    {
        const QString normalPath = recoloredSvgToThemeFile(":/images/sidebar/folder_finished.svg", nt.folderIconColor, nt.folderIconShadowColor, nt.folderReadOverlayColor, params.meta.id);
        const QString selectedPath = recoloredSvgToThemeFile(":/images/sidebar/folder_finished.svg", nt.folderIconSelectedColor, nt.folderIconSelectedShadowColor, nt.folderReadOverlaySelectedColor, params.meta.id, { .suffix = "_selected" });
        theme.navigationTree.folderFinishedIcon.addFile(normalPath, QSize(), QIcon::Normal, QIcon::Off);
        theme.navigationTree.folderFinishedIcon.addFile(selectedPath, QSize(), QIcon::Selected, QIcon::Off);
    }
    // end NavigationTree

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

    if (json.contains("comicFlow")) {
        const auto o = json["comicFlow"].toObject();
        p.comicFlowColors.backgroundColor = colorFromJson(o, "backgroundColor", p.comicFlowColors.backgroundColor);
        p.comicFlowColors.textColor = colorFromJson(o, "textColor", p.comicFlowColors.textColor);
    }

    if (json.contains("metadataScraperDialog")) {
        const auto o = json["metadataScraperDialog"].toObject();
        auto &msd = p.metadataScraperDialogParams;
        msd.contentTextColor = colorFromJson(o, "contentTextColor", msd.contentTextColor);
        msd.contentBackgroundColor = colorFromJson(o, "contentBackgroundColor", msd.contentBackgroundColor);
        msd.contentAltBackgroundColor = colorFromJson(o, "contentAltBackgroundColor", msd.contentAltBackgroundColor);
        msd.dialogBackgroundColor = colorFromJson(o, "dialogBackgroundColor", msd.dialogBackgroundColor);
        msd.tableBackgroundColor = colorFromJson(o, "tableBackgroundColor", msd.tableBackgroundColor);
        msd.tableAltBackgroundColor = colorFromJson(o, "tableAltBackgroundColor", msd.tableAltBackgroundColor);
        msd.tableBorderColor = colorFromJson(o, "tableBorderColor", msd.tableBorderColor);
        msd.tableSelectedColor = colorFromJson(o, "tableSelectedColor", msd.tableSelectedColor);
        msd.tableHeaderBackgroundColor = colorFromJson(o, "tableHeaderBackgroundColor", msd.tableHeaderBackgroundColor);
        msd.tableHeaderGradientColor = colorFromJson(o, "tableHeaderGradientColor", msd.tableHeaderGradientColor);
        msd.tableHeaderBorderColor = colorFromJson(o, "tableHeaderBorderColor", msd.tableHeaderBorderColor);
        msd.tableHeaderTextColor = colorFromJson(o, "tableHeaderTextColor", msd.tableHeaderTextColor);
        msd.tableScrollHandleColor = colorFromJson(o, "tableScrollHandleColor", msd.tableScrollHandleColor);
        msd.tableScrollBackgroundColor = colorFromJson(o, "tableScrollBackgroundColor", msd.tableScrollBackgroundColor);
        msd.tableSectionBorderLight = colorFromJson(o, "tableSectionBorderLight", msd.tableSectionBorderLight);
        msd.tableSectionBorderDark = colorFromJson(o, "tableSectionBorderDark", msd.tableSectionBorderDark);
        msd.labelTextColor = colorFromJson(o, "labelTextColor", msd.labelTextColor);
        msd.labelBackgroundColor = colorFromJson(o, "labelBackgroundColor", msd.labelBackgroundColor);
        msd.hyperlinkColor = colorFromJson(o, "hyperlinkColor", msd.hyperlinkColor);
        msd.buttonBackgroundColor = colorFromJson(o, "buttonBackgroundColor", msd.buttonBackgroundColor);
        msd.buttonTextColor = colorFromJson(o, "buttonTextColor", msd.buttonTextColor);
        msd.buttonBorderColor = colorFromJson(o, "buttonBorderColor", msd.buttonBorderColor);
        msd.radioUncheckedColor = colorFromJson(o, "radioUncheckedColor", msd.radioUncheckedColor);
        msd.radioCheckedBackgroundColor = colorFromJson(o, "radioCheckedBackgroundColor", msd.radioCheckedBackgroundColor);
        msd.radioCheckedIndicatorColor = colorFromJson(o, "radioCheckedIndicatorColor", msd.radioCheckedIndicatorColor);
        msd.checkBoxTickColor = colorFromJson(o, "checkBoxTickColor", msd.checkBoxTickColor);
        msd.toolButtonAccentColor = colorFromJson(o, "toolButtonAccentColor", msd.toolButtonAccentColor);
        msd.downArrowColor = colorFromJson(o, "downArrowColor", msd.downArrowColor);
        msd.upArrowColor = colorFromJson(o, "upArrowColor", msd.upArrowColor);
        msd.busyIndicatorColor = colorFromJson(o, "busyIndicatorColor", msd.busyIndicatorColor);
        msd.navIconColor = colorFromJson(o, "navIconColor", msd.navIconColor);
        msd.rowIconColor = colorFromJson(o, "rowIconColor", msd.rowIconColor);
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

    if (json.contains("navigationTree")) {
        const auto o = json["navigationTree"].toObject();
        auto &nt = p.navigationTreeParams;
        nt.textColor = colorFromJson(o, "textColor", nt.textColor);
        nt.selectionBackgroundColor = colorFromJson(o, "selectionBackgroundColor", nt.selectionBackgroundColor);
        nt.scrollBackgroundColor = colorFromJson(o, "scrollBackgroundColor", nt.scrollBackgroundColor);
        nt.scrollHandleColor = colorFromJson(o, "scrollHandleColor", nt.scrollHandleColor);
        nt.selectedTextColor = colorFromJson(o, "selectedTextColor", nt.selectedTextColor);
        nt.folderIndicatorColor = colorFromJson(o, "folderIndicatorColor", nt.folderIndicatorColor);
        nt.branchIndicatorColor = colorFromJson(o, "branchIndicatorColor", nt.branchIndicatorColor);
        nt.branchIndicatorSelectedColor = colorFromJson(o, "branchIndicatorSelectedColor", nt.branchIndicatorSelectedColor);
        nt.folderIconColor = colorFromJson(o, "folderIconColor", nt.folderIconColor);
        nt.folderIconShadowColor = colorFromJson(o, "folderIconShadowColor", nt.folderIconShadowColor);
        nt.folderIconSelectedColor = colorFromJson(o, "folderIconSelectedColor", nt.folderIconSelectedColor);
        nt.folderIconSelectedShadowColor = colorFromJson(o, "folderIconSelectedShadowColor", nt.folderIconSelectedShadowColor);
        nt.folderReadOverlayColor = colorFromJson(o, "folderReadOverlayColor", nt.folderReadOverlayColor);
        nt.folderReadOverlaySelectedColor = colorFromJson(o, "folderReadOverlaySelectedColor", nt.folderReadOverlaySelectedColor);
    }

    if (json.contains("comicsViewTable")) {
        const auto o = json["comicsViewTable"].toObject();
        auto &cvta = p.comicsViewTableParams;
        cvta.alternateBackgroundColor = colorFromJson(o, "alternateBackgroundColor", cvta.alternateBackgroundColor);
        cvta.backgroundColor = colorFromJson(o, "backgroundColor", cvta.backgroundColor);
        cvta.headerBackgroundColor = colorFromJson(o, "headerBackgroundColor", cvta.headerBackgroundColor);
        cvta.headerBorderColor = colorFromJson(o, "headerBorderColor", cvta.headerBorderColor);
        cvta.headerGradientColor = colorFromJson(o, "headerGradientColor", cvta.headerGradientColor);
        cvta.itemBorderBottomColor = colorFromJson(o, "itemBorderBottomColor", cvta.itemBorderBottomColor);
        cvta.itemBorderTopColor = colorFromJson(o, "itemBorderTopColor", cvta.itemBorderTopColor);
        cvta.itemBorderBottomWidth = o["itemBorderBottomWidth"].toInt(cvta.itemBorderBottomWidth);
        cvta.itemBorderTopWidth = o["itemBorderTopWidth"].toInt(cvta.itemBorderTopWidth);
        cvta.itemTextColor = colorFromJson(o, "itemTextColor", cvta.itemTextColor);
        cvta.selectedColor = colorFromJson(o, "selectedColor", cvta.selectedColor);
        cvta.selectedTextColor = colorFromJson(o, "selectedTextColor", cvta.selectedTextColor);
        cvta.headerTextColor = colorFromJson(o, "headerTextColor", cvta.headerTextColor);
        cvta.starRatingColor = colorFromJson(o, "starRatingColor", cvta.starRatingColor);
        cvta.starRatingSelectedColor = colorFromJson(o, "starRatingSelectedColor", cvta.starRatingSelectedColor);
    }

    if (json.contains("gridAndInfoView")) {
        const auto o = json["gridAndInfoView"].toObject();
        auto &giv = p.gridAndInfoViewParams;
        giv.backgroundColor = colorFromJson(o, "backgroundColor", giv.backgroundColor);
        giv.cellColor = colorFromJson(o, "cellColor", giv.cellColor);
        giv.cellColorWithBackground = colorFromJson(o, "cellColorWithBackground", giv.cellColorWithBackground);
        giv.selectedColor = colorFromJson(o, "selectedColor", giv.selectedColor);
        giv.selectedBorderColor = colorFromJson(o, "selectedBorderColor", giv.selectedBorderColor);
        giv.borderColor = colorFromJson(o, "borderColor", giv.borderColor);
        giv.titleColor = colorFromJson(o, "titleColor", giv.titleColor);
        giv.textColor = colorFromJson(o, "textColor", giv.textColor);
        if (o.contains("showDropShadow"))
            giv.showDropShadow = o["showDropShadow"].toBool(giv.showDropShadow);
        giv.infoBackgroundColor = colorFromJson(o, "infoBackgroundColor", giv.infoBackgroundColor);
        giv.infoBorderColor = colorFromJson(o, "infoBorderColor", giv.infoBorderColor);
        giv.infoShadowColor = colorFromJson(o, "infoShadowColor", giv.infoShadowColor);
        giv.infoTextColor = colorFromJson(o, "infoTextColor", giv.infoTextColor);
        giv.infoTitleColor = colorFromJson(o, "infoTitleColor", giv.infoTitleColor);
        giv.ratingUnselectedColor = colorFromJson(o, "ratingUnselectedColor", giv.ratingUnselectedColor);
        giv.ratingSelectedColor = colorFromJson(o, "ratingSelectedColor", giv.ratingSelectedColor);
        giv.favUncheckedColor = colorFromJson(o, "favUncheckedColor", giv.favUncheckedColor);
        giv.favCheckedColor = colorFromJson(o, "favCheckedColor", giv.favCheckedColor);
        giv.readTickUncheckedColor = colorFromJson(o, "readTickUncheckedColor", giv.readTickUncheckedColor);
        giv.readTickCheckedColor = colorFromJson(o, "readTickCheckedColor", giv.readTickCheckedColor);
        giv.currentComicBackgroundColor = colorFromJson(o, "currentComicBackgroundColor", giv.currentComicBackgroundColor);
        giv.continueReadingBackgroundColor = colorFromJson(o, "continueReadingBackgroundColor", giv.continueReadingBackgroundColor);
        giv.continueReadingColor = colorFromJson(o, "continueReadingColor", giv.continueReadingColor);
        giv.backgroundBlurOverlayColor = colorFromJson(o, "backgroundBlurOverlayColor", giv.backgroundBlurOverlayColor);
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
        p.meta.targetApp = o["targetApp"].toString(p.meta.targetApp);
        p.meta.version = o["version"].toString(p.meta.version);
    }

    Theme theme = makeTheme(p);
    theme.sourceJson = json;
    return theme;
}
