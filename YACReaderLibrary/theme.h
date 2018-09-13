#ifndef THEME_H
#define THEME_H

#include <QtCore>

class Theme
{
public:
    Theme();

    static Theme currentTheme() {
        Theme t;

#ifdef Q_OS_MAC
        bool macosNative = true;
#else
        bool macosNative = false;
#endif

        if (macosNative) { //native macos theme
            t.isMacosNative = true;
            t.useNativeFullScreen = true;

            t.disableClassicViewCollapsing = true;

            t.comicsViewTransitionBackground = "#FFFFFF";

            t.itemLibrarySelectedStyleSheet = "color: white; background-color:#91c4f4; border-bottom:1px solid #91c4f4;";
            t.itemLibraryNoSelectedStyleSheet = "background-color:transparent;";

            t.useNativeFolderIcons = true;

            t.noComicsContentBackgroundColor = "#FFFFFF";
            t.noComicsContentTitleLabelStyle = "QLabel {color:#888888; font-size:24px;font-family:Arial;font-weight:bold;}";

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

            t.sidebarBackgroundColor = "#F1F1F1";
            t.sidebarSplitterStyle = "QSplitter::handle:vertical { height: 29px; background-color: transparent;}";

            t.titledToolBarSeparatorColor = "#AFAFAF";
            t.titledToolBarTitleColor = "#808080";
            t.titledToolBarTitleShadowColor = "#00000000";

            t.foldersViewCompletedColor = "#AABFFF";
        } else {
            t.isMacosNative = false;
            #ifdef Q_OS_MAC
            t.useNativeFullScreen = true;
            #else
            t.useNativeFullScreen = false;
            #endif

            t.disableClassicViewCollapsing = false;

            t.comicsViewTransitionBackground = "#2A2A2A";

            t.itemLibrarySelectedStyleSheet = "color: white; background-color:#2E2E2E; font-weight:bold;";
            t.itemLibraryNoSelectedStyleSheet = "background-color:transparent; color:#DDDFDF;";

            t.useNativeFolderIcons = false;

            t.noComicsContentBackgroundColor = "#2A2A2A";
            t.noComicsContentTitleLabelStyle = "QLabel {color:#CCCCCC; font-size:24px;font-family:Arial;font-weight:bold;}";

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

            t.sidebarBackgroundColor = "#454545";
            t.sidebarSplitterStyle = "QSplitter::handle:vertical { height: 39px; background-color: transparent;}";/*"QSplitter::handle { "
                            " image: none; background-color = black; "
                            " }"
                            "QSplitter::handle:vertical { height: 39px;}"*/

            t.titledToolBarSeparatorColor = "#6F6F6F";
            t.titledToolBarTitleColor = "#BDBFBF";
            t.titledToolBarTitleShadowColor = "#000000";

            t.foldersViewCompletedColor = "#EDC542";
        }

        return t;
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
};

#endif // THEME_H
