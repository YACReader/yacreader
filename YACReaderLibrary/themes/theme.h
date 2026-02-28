#ifndef THEME_H
#define THEME_H

#include <QtGui>

#include "yacreader_icon.h"
#include "help_about_dialog_theme.h"
#include "whats_new_dialog_theme.h"

struct ComicVineThemeTemplates {
    QString defaultLabelQSS = "QLabel {color:%1; font-size:12px;font-family:Arial;}";
    QString titleLabelQSS = "QLabel {color:%1; font-size:18px;font-family:Arial;}";
    QString coverLabelQSS = "QLabel {background-color: %1; color:%2; font-size:12px; font-family:Arial; }";

    QString radioButtonQSS = "QRadioButton {margin-left:27px; margin-top:5px; color:%1;font-size:12px;font-family:Arial;}"
                             "QRadioButton::indicator {width:11px;height:11px;}"
                             "QRadioButton::indicator::unchecked {image : url(%2);}"
                             "QRadioButton::indicator::checked {image : url(%3);}";

    QString checkBoxQSS = "QCheckBox {"
                          "    color: %1;"
                          "    font-size: 12px;"
                          "    font-family: Arial;"
                          "    spacing: 10px;"
                          "}"
                          "QCheckBox::indicator {"
                          "    width: 13px;"
                          "    height: 13px;"
                          "    border: 1px solid %2;"
                          "    background: %3;"
                          "}"
                          "QCheckBox::indicator:checked {"
                          "    image: url(%4);"
                          "    background: %3;"
                          "}"
                          "QCheckBox::indicator:unchecked {"
                          "    background: %3;"
                          "}";

    QString scraperLineEditTitleLabelQSS = "QLabel {color:%1;}";
    QString scraperLineEditQSS = "QLineEdit {"
                                 "border:none; background-color: %1; color : %2; padding-left: %3; padding-bottom: 1px; margin-bottom: 0px;"
                                 "}";

    QString scraperToolButtonQSS = "QPushButton {border: none; background: %1; color:%2; border-radius:2px;}"
                                   "QPushButton::pressed {border: none; background: %3; color:%2; border-radius:2px;}";
    QString scraperToolButtonSeparatorQSS = "QWidget {background:%1;}";

    QString scraperScrollLabelTextQSS = "QLabel {background-color: %1; color:%2; font-size:12px; font-family:Arial; } QLabel::link { color: %3; font-size:12px; font-family:Arial; }";
    QString scraperScrollLabelScrollAreaQSS = "QScrollArea {background-color:%1; border:none;}"
                                              "QScrollBar:vertical { border: none; background: %1; width: 3px; margin: 0; }"
                                              "QScrollBar:horizontal { border: none; background: %1; height: 3px; margin: 0; }"
                                              "QScrollBar::handle:vertical { background: %2; width: 7px; min-height: 20px; }"
                                              "QScrollBar::handle:horizontal { background: %2; width: 7px; min-height: 20px; }"
                                              "QScrollBar::add-line:vertical { border: none; background: %3; height: 10px; subcontrol-position: bottom; subcontrol-origin: margin; margin: 0 3px 0 0;}"
                                              "QScrollBar::sub-line:vertical {  border: none; background: %3; height: 10px; subcontrol-position: top; subcontrol-origin: margin; margin: 0 3px 0 0;}"
                                              "QScrollBar::add-line:horizontal { border: none; background: %3; width: 10px; subcontrol-position: bottom; subcontrol-origin: margin; margin: 0 0 3px 0;}"
                                              "QScrollBar::sub-line:horizontal {  border: none; background: %3; width: 10px; subcontrol-position: top; subcontrol-origin: margin; margin: 0 0 3px 0;}"
                                              "QScrollBar::up-arrow:vertical {border:none;width: 9px;height: 6px;background: url(':/images/folders_view/line-up.png') center top no-repeat;}"
                                              "QScrollBar::down-arrow:vertical {border:none;width: 9px;height: 6px;background: url(':/images/folders_view/line-down.png') center top no-repeat;}"
                                              "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical, QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal {background: none; }";

    QString scraperTableViewQSS = "QTableView {color:%1; border:0px;alternate-background-color: %2;background-color: %3; outline: 0px;}"
                                  "QTableView::item {outline: 0px; border: 0px; color:%1;}"
                                  "QTableView::item:selected {outline: 0px; background-color: %4;  }"
                                  "QHeaderView::section:horizontal {background-color:%5; border-bottom:1px solid %6; border-right:1px solid qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 %14, stop: 1 %6); border-left:none; border-top:none; padding:4px; color:%7;}"
                                  "QHeaderView::section:vertical {border-bottom: 1px solid %8;border-top: 1px solid %9;}"
                                  "QHeaderView::down-arrow {image: url('%12');width: 8px;height: 7px;padding-right: 10px;}"
                                  "QHeaderView::up-arrow {image: url('%13');width: 8px;height: 7px; padding-right: 10px;}"
                                  "QScrollBar:vertical { border: none; background: %3; width: 3px; margin: 0; }"
                                  "QScrollBar:horizontal { border: none; background: %3; height: 3px; margin: 0; }"
                                  "QScrollBar::handle:vertical { background: %10; width: 7px; min-height: 20px; }"
                                  "QScrollBar::handle:horizontal { background: %10; width: 7px; min-height: 20px; }"
                                  "QScrollBar::add-line:vertical { border: none; background: %11; height: 10px; subcontrol-position: bottom; subcontrol-origin: margin; margin: 0 3px 0 0;}"
                                  "QScrollBar::sub-line:vertical {  border: none; background: %11; height: 10px; subcontrol-position: top; subcontrol-origin: margin; margin: 0 3px 0 0;}"
                                  "QScrollBar::add-line:horizontal { border: none; background: %11; width: 10px; subcontrol-position: bottom; subcontrol-origin: margin; margin: 0 0 3px 0;}"
                                  "QScrollBar::sub-line:horizontal {  border: none; background: %11; width: 10px; subcontrol-position: top; subcontrol-origin: margin; margin: 0 0 3px 0;}"
                                  "QScrollBar::up-arrow:vertical {border:none;width: 9px;height: 6px;background: url(':/images/folders_view/line-up.png') center top no-repeat;}"
                                  "QScrollBar::down-arrow:vertical {border:none;width: 9px;height: 6px;background: url(':/images/folders_view/line-down.png') center top no-repeat;}"
                                  "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical, QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal {background: none; }";

    QString dialogQSS = "QDialog {background-color: %1; }";
    QString dialogButtonsQSS = "QPushButton {border: 1px solid %1; background: %2; color:%3; padding: 5px 26px 5px 26px; font-size:12px;font-family:Arial; font-weight:bold;}";

    QString nextPageIcon = ":/images/comic_vine/nextPage.svg";
    QString previousPageIcon = ":/images/comic_vine/previousPage.svg";
    QSize pageIconSize = QSize(7, 8);

    QString rowUpIcon = ":/images/comic_vine/rowUp.svg";
    QString rowDownIcon = ":/images/comic_vine/rowDown.svg";
    QSize rowIconSize = QSize(8, 7);
};

struct ComicFlowColors {
    QColor backgroundColor;
    QColor textColor;
};

struct TableViewThemeTemplates {
    QString tableViewQSS = "QTableView {alternate-background-color: %1; background-color: %2; outline: 0px; border: none;}"
                           "QTableCornerButton::section {background-color:%3; border:none; border-bottom:1px solid %4; border-right:1px solid qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 %5, stop: 1 %4);}"
                           "QTableView::item {outline: 0px; border-bottom: %12px solid %6; border-top: %13px solid %7; padding-bottom:1px; color:%8;}"
                           "QTableView::item:selected {outline: 0px; border-bottom: %12px solid %9; border-top: %13px solid %9; padding-bottom:1px; background-color: %9; color: %10; }"
                           "QHeaderView::section:horizontal {background-color:%3; border-bottom:1px solid %4; border-right:1px solid qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 %5, stop: 1 %4); border-left:none; border-top:none; padding:4px; color:%11;}"
                           "QHeaderView::section:vertical {border-bottom: 1px solid %6; border-top: 1px solid %7;}";
};

struct TableViewTheme {
    QString tableViewQSS;
    QColor starRatingColor;
    QColor starRatingSelectedColor;
};

struct EmptyContainerThemeTemplates {
    QString titleLabelQSS = "QLabel {color:%1; font-size:24px;font-family:Arial;font-weight:bold;}";
};

struct EmptyContainerTheme {
    QColor backgroundColor;
    QString titleLabelQSS;

    // For NoLibrariesWidget
    QColor textColor;
    QColor descriptionTextColor;
    QPixmap noLibrariesIcon;

    // Search-related icons (themed from search_result.svg)
    QPixmap searchingIcon; // For ClassicComicsView searching state
    QPixmap noSearchResultsIcon; // For NoSearchResultsWidget empty state

    // Empty container icons (themed from SVGs)
    QPixmap emptyFolderIcon;
    QPixmap emptyFavoritesIcon;
    QPixmap emptyCurrentReadingsIcon;
    QPixmap emptyReadingListIcon;
    QMap<int, QPixmap> emptyLabelIcons; // Keyed by YACReader::LabelColors enum value
};

struct SidebarTheme {
    QColor backgroundColor;
    QColor separatorColor;
    QColor sectionSeparatorColor; // Horizontal separators between sidebar sections
    QString splitterQSS = "QSplitter::handle { image: none; background-color: transparent; }"
                          "QSplitter::handle:vertical { height: 39px; }";

    // When true, section title strings are uppercased after translation
    bool uppercaseLabels;

    // Title bar colors
    QColor titleTextColor;
    QColor titleDropShadowColor;
    QColor busyIndicatorColor;
};

struct ImportWidgetThemeTemplates {
    QString titleLabelQSS = "QLabel {color:%1; font-size:25px;font-weight:bold;}";
};

struct ImportWidgetTheme {
    QColor backgroundColor;
    QString titleLabelQSS;
    QColor descriptionTextColor;
    QColor currentComicTextColor;
    QColor coversViewBackgroundColor;
    QColor coversLabelColor;

    QPixmap topCoversDecoration;
    QPixmap bottomCoversDecoration;
    QPixmap importingIcon;
    QPixmap updatingIcon;
    QIcon coversToggleIcon;
};

struct TreeViewThemeTemplates {
    // Styled tree view template with custom scroll bars
    // %1 = text color, %2 = selection/hover background, %3 = scroll background, %4 = scroll handle, %5 = selected text color
    QString styledTreeViewQSS = "QTreeView {background-color:transparent; border: none; color:%1; outline:0; show-decoration-selected: 0;}"
                                "QTreeView::item:selected {background-color: %2; color:%5; font:bold;}"
                                "QTreeView::item:hover {background-color:%2; color:%5; font:bold;}"
                                "QTreeView::branch:selected {background-color:%2;}"
                                "QScrollBar:vertical { border: none; background: %3; width: 7px; margin: 0 3px 0 0; }"
                                "QScrollBar::handle:vertical { background: %4; width: 7px; min-height: 20px; }"
                                "QScrollBar::add-line:vertical { border: none; background: %3; height: 10px; subcontrol-position: bottom; subcontrol-origin: margin; margin: 0 3px 0 0;}"
                                "QScrollBar::sub-line:vertical {  border: none; background: %3; height: 10px; subcontrol-position: top; subcontrol-origin: margin; margin: 0 3px 0 0;}"
                                "QScrollBar::up-arrow:vertical {border:none;width: 9px;height: 6px;background: url(':/images/folders_view/line-up.png') center top no-repeat;}"
                                "QScrollBar::down-arrow:vertical {border:none;width: 9px;height: 6px;background: url(':/images/folders_view/line-down.png') center top no-repeat;}"
                                "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {background: none; }"
                                "QTreeView::branch:has-children:!has-siblings:closed,QTreeView::branch:closed:has-children:has-siblings {border-image: none;image: url('%6');}"
                                "QTreeView::branch:has-children:selected:!has-siblings:closed,QTreeView::branch:closed:selected:has-children:has-siblings {border-image: none;image: url('%8');}"
                                "QTreeView::branch:open:has-children:!has-siblings,QTreeView::branch:open:has-children:has-siblings  {border-image: none;image: url('%7');}"
                                "QTreeView::branch:open:has-children:selected:!has-siblings,QTreeView::branch:open:has-children:selected:has-siblings {border-image: none;image: url('%9');}";
};

struct TreeViewTheme {
    QString treeViewQSS;
    QColor folderIndicatorColor; // For incomplete folders and recently updated folders

    // Branch indicator icon paths (used by QSS url())
    QString branchClosedIconPath;
    QString branchOpenIconPath;
    QString branchClosedIconSelectedPath;
    QString branchOpenIconSelectedPath;

    // Folder icons (normal and selected states, including finished/read tick variant)
    QIcon folderIcon;
    QIcon folderFinishedIcon;
};

// QML view theme colors (used by GridComicsView, FolderContentView, InfoComicsView)
struct QmlViewTheme {
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
    QString topShadow; // Recolored SVG path
    QString infoShadow; // Recolored SVG path
    QString infoIndicator; // Recolored SVG path
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

struct MainToolbarThemeTemplates {
    // Toolbar QSS: %1 = background color
    QString toolbarQSS = "QToolButton {border:none;}";

    // Folder name label QSS: %1 = text color
    QString folderNameLabelQSS = "QLabel {color:%1; font-size:22px; font-weight:bold;}";
};

struct ContentSplitterThemeTemplates {
    QString horizontalSplitterQSS = "QSplitter::handle { image: none; background-color: %1; height: %2px; }";
    QString verticalSplitterQSS = "QSplitter::handle { image: none; background-color: %1; height: %2px; }";
};

struct ComicsViewToolbarThemeTemplates {
    QString toolbarQSS = "QToolBar { border: none; background: %1; }\n"
                         "QToolBar::separator { background: %2; width: 1px; margin: 5px 4px; }\n"
                         "QToolButton:checked { background-color: %3; }\n";
};

struct SearchLineEditThemeTemplates {
    // %1 = text color, %2 = background color, %3 = padding-left, %4 = padding-right
    QString lineEditQSS = "QLineEdit {color: %1; border:none; border-radius: 4px; background-color:%2; padding-left: %3px; padding-right: %4px; padding-bottom: 1px; margin-right: 9px;} ";
    QString searchLabelQSS = "QLabel { border: none; padding: 0px; }";
    QString clearButtonQSS = "QToolButton { border: none; padding: 0px; }";
};

struct ContentSplitterTheme {
    QString horizontalSplitterQSS; // For Qt::Horizontal splitters (vertical handle bar)
    QString verticalSplitterQSS; // For Qt::Vertical splitters (horizontal handle bar)
};

struct SidebarIconsTheme {
    // When true, use QFileIconProvider for folder icons and overlay folderReadOverlay for finished folders
    // When false, use the themed folderIcon and folderFinishedIcon from TreeViewTheme
    bool useSystemFolderIcons;
    QPixmap folderReadOverlay; // Tick overlay drawn on system folder icons when useSystemFolderIcons is true

    // Library icon (for YACReaderLibraryItemWidget - unselected state)
    QIcon libraryIcon;

    // Action icons (for LibraryWindowActions - toolbar buttons)
    QIcon newLibraryIcon;
    QIcon openLibraryIcon;
    QIcon addNewIcon;
    QIcon deleteIcon;
    QIcon setRootIcon;
    QIcon expandIcon;
    QIcon colapseIcon;
    QIcon addLabelIcon;
    QIcon renameListIcon;
};

struct LibraryItemTheme {
    QColor textColor;
    QColor selectedTextColor;
    QColor selectedBackgroundColor;
    QIcon libraryIconSelected; // Library icon when selected
    QIcon libraryOptionsIcon; // Options icon (shown only when selected)
};

struct SearchLineEditTheme {
    QString lineEditQSS;
    QString searchLabelQSS;
    QString clearButtonQSS;
    QPixmap searchIcon;
    QPixmap clearIcon;
};

struct ReadingListIconsTheme {
    // Label icons by color name (red, orange, yellow, green, cyan, blue, violet, purple, pink, white, light, dark)
    QMap<QString, QIcon> labelIcons;

    // Special list icons (Reading, Favorites, Currently Reading)
    QIcon readingListIcon; // default_0 - bookmark
    QIcon favoritesIcon; // default_1 - heart
    QIcon currentlyReadingIcon; // default_2 - circle with star

    // Reading list icon
    QIcon listIcon;
};

struct DialogIconsTheme {
    QPixmap newLibraryIcon;
    QPixmap openLibraryIcon;
    QPixmap editIcon;
    QPixmap exportComicsInfoIcon;
    QPixmap importComicsInfoIcon;
    QPixmap exportLibraryIcon;
    QPixmap importLibraryIcon;
    QIcon findFolderIcon;
};

struct MenuIconsTheme {
    // Library context menu
    QIcon exportComicsInfoIcon;
    QIcon importComicsInfoIcon;
    QIcon exportLibraryIcon;
    QIcon importLibraryIcon;
    QIcon updateLibraryIcon;
    QIcon renameLibraryIcon; // editIcon
    QIcon removeLibraryIcon;

    // Folder/Comic context menu
    QIcon openContainingFolderIcon;
    QIcon updateCurrentFolderIcon;

    // App
    QIcon quitIcon;
};

struct ShortcutsIconsTheme {
    QIcon comicsIcon;
    QIcon foldersIcon;
    QIcon generalIcon;
    QIcon librariesIcon;
    QIcon visualizationIcon;
};

struct ServerConfigDialogTheme {
    QString dialogQSS;
    QString titleLabelQSS;
    QString qrMessageLabelQSS;
    QString propagandaLabelQSS;
    QString labelQSS;
    QString checkBoxQSS;
    QColor qrBackgroundColor;
    QColor qrForegroundColor;
    QPixmap backgroundDecoration;
};

struct ComicsViewToolbarTheme {
    QString toolbarQSS;

    QIcon openInYACReaderIcon;
    QIcon setAsReadIcon;
    QIcon setAsUnreadIcon;
    QIcon showComicInfoIcon;
    QIcon setAsNormalIcon;
    QIcon setAsMangaIcon;
    QIcon editComicIcon;
    QIcon getInfoIcon;
    QIcon assignNumberIcon;
    QIcon selectAllIcon;
    QIcon deleteIcon;
    QIcon hideComicFlowIcon;
    QIcon showMarksIcon;
    QIcon showRecentIndicatorIcon;
    QIcon bigGridZoomIcon;
    QIcon smallGridZoomIcon;
};

struct MainToolbarTheme {
    QColor backgroundColor;
    QColor folderNameColor;
    QPixmap dividerPixmap;

    // Icons (contain both Normal and Disabled states)
    QIcon backIcon;
    QIcon forwardIcon;
    QIcon settingsIcon;
    QIcon serverIcon;
    QIcon helpIcon;
    QIcon gridIcon;
    QIcon flowIcon;
    QIcon infoIcon;
    QIcon fullscreenIcon;

    // QSS
    QString toolbarQSS;
    QString folderNameLabelQSS;
};

struct ComicVineTheme {
    QString defaultLabelQSS;
    QString titleLabelQSS;
    QString coverLabelQSS;
    QString radioButtonQSS;
    QString checkBoxQSS;

    QString scraperLineEditTitleLabelQSS;
    QString scraperLineEditQSS;

    QString scraperToolButtonQSS;
    QString scraperToolButtonSeparatorQSS;
    QColor scraperToolButtonFillColor;

    QString scraperScrollLabelTextQSS;
    QString scraperScrollLabelScrollAreaQSS;

    QString scraperTableViewQSS;

    QString dialogQSS;
    QString dialogButtonsQSS;

    QString noBorderToolButtonQSS = "QToolButton { border: none; }";

    QColor busyIndicatorColor;

    YACReaderIcon nextPageIcon;
    YACReaderIcon previousPageIcon;

    YACReaderIcon rowUpIcon;
    YACReaderIcon rowDownIcon;
};

struct Theme {
    QColor defaultContentBackgroundColor;

    ComicFlowColors comicFlow;
    ComicVineTheme comicVine;
    HelpAboutDialogTheme helpAboutDialog;
    WhatsNewDialogTheme whatsNewDialog;
    EmptyContainerTheme emptyContainer;
    SidebarTheme sidebar;
    SidebarIconsTheme sidebarIcons;
    LibraryItemTheme libraryItem;
    ImportWidgetTheme importWidget;
    ServerConfigDialogTheme serverConfigDialog;
    TreeViewTheme treeView;
    TableViewTheme tableView;
    QmlViewTheme qmlView;
    MainToolbarTheme mainToolbar;
    ContentSplitterTheme contentSplitter;
    ComicsViewToolbarTheme comicsViewToolbar;
    SearchLineEditTheme searchLineEdit;
    ReadingListIconsTheme readingListIcons;
    DialogIconsTheme dialogIcons;
    MenuIconsTheme menuIcons;
    ShortcutsIconsTheme shortcutsIcons;
};

#endif // THEME_H
