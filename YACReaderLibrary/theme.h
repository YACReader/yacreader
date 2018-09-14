#ifndef THEME_H
#define THEME_H

#include <QtCore>
#include <QtWidgets>
#include <QtQml>

class Theme
{
public:
    Theme();

    static Theme currentTheme() {
        Theme t;

#ifdef Q_OS_MAC
        bool macosNative = false;
#else
        bool macosNative = false;
#endif

        if (macosNative) { //native macos theme
            t.isMacosNative = true;
            t.useNativeFullScreen = true;

            //
            t.disableClassicViewCollapsing = true;

            //
            t.comicsViewTransitionBackground = "#FFFFFF";

            //
            t.itemLibrarySelectedStyleSheet = "color: white; background-color:#91c4f4; border-bottom:1px solid #91c4f4;";
            t.itemLibraryNoSelectedStyleSheet = "background-color:transparent;";

            //
            t.useNativeFolderIcons = true;

            //
            t.noComicsContentBackgroundColor = "#FFFFFF";
            t.noComicsContentTitleLabelStyle = "QLabel {color:#888888; font-size:24px;font-family:Arial;font-weight:bold;}";

            //
            t.emptyFolderWidgetImage = ":/images/empty_folder.png";
            t.emptyFolderWidgetStyle = "QListView {background-color:transparent; border: none; color:#959595; outline:0; font-size: 18px; show-decoration-selected: 0; margin:0}"
                                       "QListView::item:selected {background-color: #EFEFEF; color:#CCCCCC;}"
                                       "QListView::item:hover {background-color:#F4F4F8; color:#757575; }"

                                       "QScrollBar:vertical { border-radius:3px; background: #FFFFFF; width: 14px; margin: 0 10px 0 0; }"
                                       "QScrollBar::handle:vertical { border: 1px solid #999999; background: #999999; width: 14px; min-height: 20px; border-radius: 2px; }"
                                       "QScrollBar::add-line:vertical { border: none; background: #999999; height: 0px; subcontrol-position: bottom; subcontrol-origin: margin; margin: 0 3px 0 0;}"

                                       "QScrollBar::sub-line:vertical {  border: none; background: #999999; height: 0px; subcontrol-position: top; subcontrol-origin: margin; margin: 0 3px 0 0;}"
                                       "QScrollBar::up-arrow:vertical {border:none;width: 9px;height: 6px;background: url(':/images/folders_view/line-up.png') center top no-repeat;}"
                                       "QScrollBar::down-arrow:vertical {border:none;width: 9px;height: 6px;background: url(':/images/folders_view/line-down.png') center top no-repeat;}"

                                       "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {background: none; }"
                                       "QScrollBar:horizontal{height:0px;}";

            //
            t.sidebarBackgroundColor = "#F1F1F1";
            t.sidebarSplitterStyle = "QSplitter::handle:vertical { height: 29px; background-color: transparent;}";

            //
            t.titledToolBarSeparatorColor = "#AFAFAF";
            t.titledToolBarTitleColor = "#808080";
            t.titledToolBarTitleShadowColor = "#00000000";

            //
            t.foldersViewCompletedColor = "#AABFFF";

            //
            t.gridComicsViewBackgroundColor = "#F6F6F6";
            t.gridComicsViewCellColor = "#FFFFFF";
            t.gridComicsViewCellColorWhenBackgroundImageIsUsed = "#99FFFFFF";
            t.gridComicsViewSelectedColor = "#FFFFFF";
            t.gridComicsViewSelectedBorderColor = "#007AFF";
            t.gridComicsViewBorderColor = "#DBDBDB";
            t.gridComicsViewTitleColor = "#121212";
            t.gridComicsViewTextColor = "#636363";
            t.gridComicsViewFontSize = 11;
            t.gridComicsViewFontSpacing = 0.5;
            t.useDropShadows = false;
            t.gridComicsViewCurrentComicBackground = "#88FFFFFF";

            //
            t.infoViewInfoBackgroundColor = "#FFFFFF";
            t.infoViewTopShadow = QUrl();
            t.infoViewInfoShadow = "info-shadow-light.png";
            t.infoViewInfoIndicator = "info-indicator-light.png";

            t.infoViewInfoTextColor = "#404040";
            t.infoViewInfoTitleColor = "#2E2E2E";

            t.infoViewRatingUnselectedColor = "#DEDEDE";
            t.infoViewRatingSelectedColor = "#2B2B2B";

            t.infoViewFavUncheckedColor = "#DEDEDE";
            t.infoViewFavCheckedColor = "#E84852";

            t.infoViewReadTickUncheckedColor = "#DEDEDE";
            t.infoViewReadTickCheckedColor = "#E84852";

            t.treeViewStyle = "QTreeView {background-color:transparent; border: none;}"
                              "QTreeView::item:selected {background-color:#91c4f4; border-top: 1px solid #91c4f4; border-left:none;border-right:none;border-bottom:1px solid #91c4f4;}"
                              "QTreeView::branch:selected {background-color:#91c4f4; border-top: 1px solid #91c4f4; border-left:none;border-right:none;border-bottom:1px solid #91c4f4;}"
                              "QTreeView::branch:open:selected:has-children {image: url(':/images/sidebar/expanded_branch_osx.png');}"
                              "QTreeView::branch:closed:selected:has-children {image: url(':/images/sidebar/collapsed_branch_osx.png');}";

            t.tableViewStyle = "QTableView {alternate-background-color: #F2F2F2;background-color: #FAFAFA; outline: 0px;}"
                               "QTableCornerButton::section {background-color:#F5F5F5; border:none; border-bottom:1px solid #B8BDC4; border-right:1px solid qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #D1D1D1, stop: 1 #B8BDC4);}"
                               "QTableView::item {outline: 0px; border-bottom: 1px solid #DFDFDF;border-top: 1px solid #FEFEFE; padding-bottom:1px; color:#252626;}"
                               "QTableView {border-top:1px solid #B8B8B8;border-bottom:none;border-left:1px solid #B8B8B8;border-right:none;}"

                               "QTableView {border-top:1px solid #B8B8B8;border-bottom:none;border-left:none;border-right:none;}"
                               "QTableView::item:selected {outline: 0px; border-bottom: 1px solid #3875D7;border-top: 1px solid #3875D7; padding-bottom:1px; background-color: #3875D7; color: #FFFFFF; }"

                               "QHeaderView::section:horizontal {background-color:#F5F5F5; border-bottom:1px solid #B8BDC4; border-right:1px solid qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #D1D1D1, stop: 1 #B8BDC4); border-left:none; border-top:none; padding:4px; color:#313232;}"
                               "QHeaderView::section:vertical {border-bottom: 1px solid #DFDFDF;border-top: 1px solid #FEFEFE;}";

            t.mainWindowHorizontalSpliterStyle = "QSplitter::handle{image:none;background-color:#B8B8B8;} QSplitter::handle:vertical {height:1px;}";
        } else {
            t.isMacosNative = false;
            #ifdef Q_OS_MAC
            t.useNativeFullScreen = true;
            #else
            t.useNativeFullScreen = false;
            #endif

            t.disableClassicViewCollapsing = false;

            //
            t.comicsViewTransitionBackground = "#2A2A2A";

            //
            t.itemLibrarySelectedStyleSheet = "color: white; background-color:#2E2E2E; font-weight:bold;";
            t.itemLibraryNoSelectedStyleSheet = "background-color:transparent; color:#DDDFDF;";

            //
            t.useNativeFolderIcons = false;

            //
            t.noComicsContentBackgroundColor = "#2A2A2A";
            t.noComicsContentTitleLabelStyle = "QLabel {color:#CCCCCC; font-size:24px;font-family:Arial;font-weight:bold;}";

            //
            t.emptyFolderWidgetImage = ":/images/empty_folder.png";
            t.emptyFolderWidgetStyle = "QListView {background-color:transparent; border: none; color:#858585; outline:0; font-size: 18px; font:bold; show-decoration-selected: 0; margin:0}"
                                       "QListView::item:selected {background-color: #212121; color:#CCCCCC;}"
                                       "QListView::item:hover {background-color:#212121; color:#CCCCCC; }"

                                       "QScrollBar:vertical { border: none; background: #212121; width: 14px; margin: 0 10px 0 0; }"
                                       "QScrollBar::handle:vertical { background: #858585; width: 14px; min-height: 20px; }"
                                       "QScrollBar::add-line:vertical { border: none; background: #212121; height: 0px; subcontrol-position: bottom; subcontrol-origin: margin; margin: 0 3px 0 0;}"

                                       "QScrollBar::sub-line:vertical {  border: none; background: #212121; height: 0px; subcontrol-position: top; subcontrol-origin: margin; margin: 0 3px 0 0;}"
                                       "QScrollBar::up-arrow:vertical {border:none;width: 9px;height: 6px;background: url(':/images/folders_view/line-up.png') center top no-repeat;}"
                                       "QScrollBar::down-arrow:vertical {border:none;width: 9px;height: 6px;background: url(':/images/folders_view/line-down.png') center top no-repeat;}"

                                       "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {background: none; }"
                                       "QScrollBar:horizontal{height:0px;}";

            //
            t.sidebarBackgroundColor = "#454545";
            t.sidebarSplitterStyle = "QSplitter::handle:vertical { height: 39px; background-color: transparent;}";/*"QSplitter::handle { "
                            " image: none; background-color = black; "
                            " }"
                            "QSplitter::handle:vertical { height: 39px;}"*/

            //
            t.titledToolBarSeparatorColor = "#6F6F6F";
            t.titledToolBarTitleColor = "#BDBFBF";
            t.titledToolBarTitleShadowColor = "#000000";

            //
            t.foldersViewCompletedColor = "#EDC542";

            //
            t.gridComicsViewBackgroundColor = "#2A2A2A";
            t.gridComicsViewCellColor = "#212121";
            t.gridComicsViewCellColorWhenBackgroundImageIsUsed = "#99212121";
            t.gridComicsViewSelectedColor = "#121212";
            t.gridComicsViewSelectedBorderColor = "#ffcc00";
            t.gridComicsViewBorderColor = "#121212";
            t.gridComicsViewTitleColor = "#FFFFFF";
            t.gridComicsViewTextColor = "#A8A8A8";
            t.gridComicsViewFontSize = 0;
            t.gridComicsViewFontSpacing = 0.5;
            t.useDropShadows = true;
            t.gridComicsViewCurrentComicBackground = "#88000000";

            //
            t.infoViewInfoBackgroundColor = "#2E2E2E";
            t.infoViewTopShadow = "info-top-shadow.png";
            t.infoViewInfoShadow = "info-shadow.png";
            t.infoViewInfoIndicator = "info-indicator.png";

            t.infoViewInfoTextColor = "#B0B0B0";
            t.infoViewInfoTitleColor = "#FFFFFF";

            t.infoViewRatingUnselectedColor = "#1C1C1C";
            t.infoViewRatingSelectedColor = "#FFFFFF";

            t.infoViewFavUncheckedColor = "#1C1C1C";
            t.infoViewFavCheckedColor = "#E84852";

            t.infoViewReadTickUncheckedColor = "#1C1C1C";
            t.infoViewReadTickCheckedColor = "#E84852";

            t.treeViewStyle = "QTreeView {background-color:transparent; border: none; color:#DDDFDF; outline:0; show-decoration-selected: 0;}"
                              "QTreeView::item:selected {background-color: #2E2E2E; color:white; font:bold;}"
                              "QTreeView::item:hover {background-color:#2E2E2E; color:white; font:bold;}"
                              "QTreeView::branch:selected {background-color:#2E2E2E;}"

                              "QScrollBar:vertical { border: none; background: #404040; width: 7px; margin: 0 3px 0 0; }"
                              "QScrollBar::handle:vertical { background: #DDDDDD; width: 7px; min-height: 20px; }"
                              "QScrollBar::add-line:vertical { border: none; background: #404040; height: 10px; subcontrol-position: bottom; subcontrol-origin: margin; margin: 0 3px 0 0;}"

                              "QScrollBar::sub-line:vertical {  border: none; background: #404040; height: 10px; subcontrol-position: top; subcontrol-origin: margin; margin: 0 3px 0 0;}"
                              "QScrollBar::up-arrow:vertical {border:none;width: 9px;height: 6px;background: url(':/images/folders_view/line-up.png') center top no-repeat;}"
                              "QScrollBar::down-arrow:vertical {border:none;width: 9px;height: 6px;background: url(':/images/folders_view/line-down.png') center top no-repeat;}"

                              "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {background: none; }"

                              "QTreeView::branch:has-children:!has-siblings:closed,QTreeView::branch:closed:has-children:has-siblings {border-image: none;image: url(':/images/sidebar/branch-closed.png');}"
                              "QTreeView::branch:has-children:selected:!has-siblings:closed,QTreeView::branch:closed:selected:has-children:has-siblings {border-image: none;image: url(':/images/sidebar/collapsed_branch_selected.png');}"

                              "QTreeView::branch:open:has-children:!has-siblings,QTreeView::branch:open:has-children:has-siblings  {border-image: none;image: url(':/images/sidebar/branch-open.png');}"
                              "QTreeView::branch:open:has-children:selected:!has-siblings,QTreeView::branch:open:has-children:selected:has-siblings {border-image: none;image: url(':/images/sidebar/expanded_branch_selected.png');}";

            t.tableViewStyle = "QTableView {alternate-background-color: #F2F2F2;background-color: #FAFAFA; outline: 0px;}"
                               "QTableCornerButton::section {background-color:#F5F5F5; border:none; border-bottom:1px solid #B8BDC4; border-right:1px solid qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #D1D1D1, stop: 1 #B8BDC4);}"
                               "QTableView::item {outline: 0px; border-bottom: 1px solid #DFDFDF;border-top: 1px solid #FEFEFE; padding-bottom:1px; color:#252626;}"
                               "QTableView {border-top:1px solid #B8B8B8;border-bottom:none;border-left:1px solid #B8B8B8;border-right:none;}"

                               "QTableView::item:selected {outline: 0px; border-bottom: 1px solid #D4D4D4;border-top: 1px solid #D4D4D4; padding-bottom:1px; background-color: #D4D4D4;  }"
                                         "QHeaderView::section:horizontal {background-color:#F5F5F5; border-bottom:1px solid #B8BDC4; border-right:1px solid qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #D1D1D1, stop: 1 #B8BDC4); border-left:none; border-top:none; padding:4px; color:#313232;}"
                               "QHeaderView::section:vertical {border-bottom: 1px solid #DFDFDF;border-top: 1px solid #FEFEFE;}";

            t.mainWindowHorizontalSpliterStyle = "QSplitter::handle:vertical {height:4px;}";
        }

        return t;
    }

    void configureComicsGridView(QQmlContext *ctxt) {
        ctxt->setContextProperty("backgroundColor", gridComicsViewBackgroundColor );
        ctxt->setContextProperty("cellColor", gridComicsViewCellColor);
        ctxt->setContextProperty("selectedColor", gridComicsViewSelectedColor);
        ctxt->setContextProperty("selectedBorderColor", gridComicsViewSelectedBorderColor);
        ctxt->setContextProperty("borderColor", gridComicsViewBorderColor);
        ctxt->setContextProperty("titleColor", gridComicsViewTitleColor);
        ctxt->setContextProperty("textColor", gridComicsViewTextColor);
        ctxt->setContextProperty("useDropShadows",useDropShadows);
        //fonts settings
        if (gridComicsViewFontSize < 1) {
            int fontSize = QApplication::font().pointSize();
            if(fontSize == -1)
                fontSize = QApplication::font().pixelSize();

            ctxt->setContextProperty("fontSize", fontSize);
        } else {
            ctxt->setContextProperty("fontSize", gridComicsViewFontSize);
        }

        ctxt->setContextProperty("fontFamily", QApplication::font().family());
        ctxt->setContextProperty("fontSpacing", 0.5);

        ctxt->setContextProperty("currentComicBackgroundColor", gridComicsViewCurrentComicBackground);
    }

    void configureInfoView(QQmlContext *ctxt) {
        ctxt->setContextProperty("infoBackgroundColor", infoViewInfoBackgroundColor);
        ctxt->setContextProperty("topShadow", infoViewTopShadow);
        ctxt->setContextProperty("infoShadow", infoViewInfoShadow);
        ctxt->setContextProperty("infoIndicator", infoViewInfoIndicator);

        ctxt->setContextProperty("infoTextColor", infoViewInfoTextColor);
        ctxt->setContextProperty("infoTitleColor", infoViewInfoTitleColor);

        ctxt->setContextProperty("ratingUnselectedColor", infoViewRatingUnselectedColor);
        ctxt->setContextProperty("ratingSelectedColor", infoViewRatingSelectedColor);

        ctxt->setContextProperty("favUncheckedColor", infoViewFavUncheckedColor);
        ctxt->setContextProperty("favCheckedColor", infoViewFavCheckedColor);

        ctxt->setContextProperty("readTickUncheckedColor", infoViewReadTickUncheckedColor);
        ctxt->setContextProperty("readTickCheckedColor", infoViewReadTickCheckedColor);

        ctxt->setContextProperty("useDropShadows",useDropShadows);
    }

    bool isMacosNative;
    bool useNativeFullScreen;

    //
    bool disableClassicViewCollapsing;

    //
    QString comicsViewTransitionBackground;

    //library item
    QString itemLibrarySelectedStyleSheet;
    QString itemLibraryNoSelectedStyleSheet;

    //tree folders
    bool useNativeFolderIcons;

    //EmptyContainerInfo & NoSearchResultsWidget
    QString noComicsContentBackgroundColor;
    QString noComicsContentTitleLabelStyle;

    //EmptyFolderWidget
    QString emptyFolderWidgetImage;
    QString emptyFolderWidgetStyle;

    //Sidebar
    QString sidebarBackgroundColor;
    QString sidebarSplitterStyle;

    //YACReaderTitledToolBar
    QString titledToolBarSeparatorColor;
    QString titledToolBarTitleColor;
    QString titledToolBarTitleShadowColor;

    //YACReaderFoldersView (tree view)
    QString foldersViewCompletedColor;

    //GridComicsView
    QString gridComicsViewBackgroundColor;
    QString gridComicsViewCellColor;
    QString gridComicsViewCellColorWhenBackgroundImageIsUsed;
    QString gridComicsViewSelectedColor;
    QString gridComicsViewSelectedBorderColor;
    QString gridComicsViewBorderColor;
    QString gridComicsViewTitleColor;
    QString gridComicsViewTextColor;
    float gridComicsViewFontSize;
    float gridComicsViewFontSpacing;
    bool useDropShadows;
    QString gridComicsViewCurrentComicBackground;

    //InfoView
    QString infoViewInfoBackgroundColor;
    QVariant infoViewTopShadow;
    QString infoViewInfoShadow;
    QString infoViewInfoIndicator;

    QString infoViewInfoTextColor;
    QString infoViewInfoTitleColor;

    QString infoViewRatingUnselectedColor;
    QString infoViewRatingSelectedColor;

    QString infoViewFavUncheckedColor;
    QString infoViewFavCheckedColor;

    QString infoViewReadTickUncheckedColor;
    QString infoViewReadTickCheckedColor;

    //TreeView (folders)
    QString treeViewStyle;

    //TableView (comics)
    QString tableViewStyle;

    //Main window
    QString mainWindowHorizontalSpliterStyle;
};

#endif // THEME_H
