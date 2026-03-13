#ifndef THEME_H
#define THEME_H

#include "help_about_dialog_theme.h"
#include "theme_meta.h"
#include "whats_new_dialog_theme.h"

#include <QIcon>
#include <QJsonObject>

struct ToolbarThemeTemplates {
    QString toolbarQSS = "QToolBar { border: none; background: %1; }\n"
                         "QToolBar::separator { background: %2; width: 1px; margin: 5px 4px; }\n"
                         "QToolButton:checked { background-color: %3; }\n"
                         "QToolButton::menu-button {border: none; width: 18px; }\n"
                         "QToolButton::menu-arrow { image: url(%4); width: 8px; height: 8px; }\n";
    QString menuArrowPath = ":/images/viewer_toolbar/menuArrow.svg";
};

struct ViewerThemeTemplates {
    QString infoLabelQSS = "QLabel { color : %1; font-size:%2px; }";
};

struct TranslatorThemeTemplates {
    // %1 = track color, %2 = handle color
    QString scrollBarQSS = "QScrollBar:vertical { border: none; background: %1; width: 7px; margin: 0 3px 0 0; }"
                           "QScrollBar::handle:vertical { background: %2; width: 7px; min-height: 20px; }"
                           "QScrollBar::add-line:vertical { border: none; background: %1; height: 10px; subcontrol-position: bottom; subcontrol-origin: margin; margin: 0 3px 0 0;}"
                           "QScrollBar::sub-line:vertical { border: none; background: %1; height: 10px; subcontrol-position: top; subcontrol-origin: margin; margin: 0 3px 0 0;}"
                           "QScrollBar::up-arrow:vertical {border:none;width: 9px;height: 6px;background: url(':/images/folders_view/line-up.png') center top no-repeat;}"
                           "QScrollBar::down-arrow:vertical {border:none;width: 9px;height: 6px;background: url(':/images/folders_view/line-down.png') center top no-repeat;}"
                           "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {background: none; }";
    // %1 = background, %2 = text color
    QString textEditQSS = "QTextEdit{border:none;background:%1;color:%2; font-size:12px; padding:6px;}";
    // %1 = background, %2 = text color, %3 = arrow icon path, %4 = list background, %5 = selection background
    QString comboBoxQSS = "QComboBox {border:none;background:%1;color:%2;font-size:12px;font-family:Arial;padding-left:8px;}"
                          "QComboBox::down-arrow {image: url(%3);}"
                          "QComboBox::drop-down {border:none; padding-right:10px;}"
                          "QComboBox QAbstractItemView {border: none; background:%4; color:%2; selection-background-color: %5; outline:none;}"
                          "QComboBox QAbstractItemView::item {padding-left:8px;}";
    // %1 = border color, %2 = background, %3 = text color
    QString clearButtonQSS = "QPushButton {border:1px solid %1; background:%2; color:%3; font-family:Arial; font-size:12px; padding-top:5px; padding-bottom:5px;}";
    // %1 = text color
    QString titleQSS = "QLabel {font-size:18px; font-family:Arial; color:%1;}";
    QString resultsTitleQSS = "QLabel {font-family:Arial;font-size:14px;color:%1;}";
    QString resultTextQSS = "QLabel {color:%1;font-size:12px;}";
};

struct GoToFlowWidgetThemeTemplates {
    QString sliderQSS = "QSlider::groove:horizontal {"
                        "  border: 1px solid %1;"
                        "  border-radius: 1px;"
                        "  background: %2;"
                        "  margin: 2px 0;"
                        "  padding: 1px;"
                        "}"
                        "QSlider::handle:horizontal {"
                        "  background: %3;"
                        "  width: 48px;"
                        "  border-radius: 1px;"
                        "}";
    QString editQSS = "QLineEdit {border: 1px solid %1; background: %2; color: %3; padding: 3px 5px 5px 5px; margin: 13px 5px 12px 5px; font-weight:bold}";
    QString buttonQSS = "QPushButton { border: none; padding: 0px; } "
                        "QPushButton:focus { border: none; outline: none; } "
                        "QPushButton:pressed { padding-top: 1px; padding-left: 1px; padding-bottom: -1px; padding-right: -1px; }";
    QString labelQSS = "QLabel { color: %1; }";
};

struct TranslatorTheme {
    QColor backgroundColor;
    QColor inputBackgroundColor;
    QString scrollBarQSS;
    QString textEditQSS;
    QString comboBoxQSS;
    QString clearButtonQSS;
    QString titleQSS;
    QString resultsTitleQSS;
    QString resultTextQSS;
    QIcon closeIcon;
    QIcon speakerIcon;
    QIcon searchIcon;
    QPixmap fromToPixmap;
};

struct ToolbarTheme {
    QString toolbarQSS;

    QIcon openAction;
    QIcon openAction18x18;
    QIcon openFolderAction;
    QIcon openFolderAction18x18;
    QIcon openLatestComicAction;
    QIcon openLatestComicAction18x18;
    QIcon saveImageAction;
    QIcon saveImageAction18x18;
    QIcon openComicOnTheLeftAction;
    QIcon openComicOnTheLeftAction18x18;
    QIcon openComicOnTheRightAction;
    QIcon openComicOnTheRightAction18x18;
    QIcon goToPageOnTheLeftAction;
    QIcon goToPageOnTheLeftAction18x18;
    QIcon goToPageOnTheRightAction;
    QIcon goToPageOnTheRightAction18x18;
    QIcon adjustHeightAction;
    QIcon adjustHeightAction18x18;
    QIcon adjustWidthAction;
    QIcon adjustWidthAction18x18;
    QIcon leftRotationAction;
    QIcon leftRotationAction18x18;
    QIcon rightRotationAction;
    QIcon rightRotationAction18x18;
    QIcon doublePageAction;
    QIcon doublePageAction18x18;
    QIcon doubleMangaPageAction;
    QIcon doubleMangaPageAction18x18;
    QIcon increasePageZoomAction;
    QIcon increasePageZoomAction18x18;
    QIcon decreasePageZoomAction;
    QIcon decreasePageZoomAction18x18;
    QIcon resetZoomAction;
    QIcon resetZoomAction18x18;
    QIcon showZoomSliderlAction;
    QIcon showZoomSliderlAction18x18;
    QIcon goToPageAction;
    QIcon goToPageAction18x18;
    QIcon optionsAction;
    QIcon optionsAction18x18;
    QIcon helpAboutAction;
    QIcon helpAboutAction18x18;
    QIcon showMagnifyingGlassAction;
    QIcon showMagnifyingGlassAction18x18;
    QIcon setBookmarkAction;
    QIcon setBookmarkAction18x18;
    QIcon showBookmarksAction;
    QIcon showBookmarksAction18x18;
    QIcon showShorcutsAction;
    QIcon showShorcutsAction18x18;
    QIcon showInfoAction;
    QIcon showInfoAction18x18;
    QIcon closeAction;
    QIcon closeAction18x18;
    QIcon showDictionaryAction;
    QIcon showDictionaryAction18x18;
    QIcon adjustToFullSizeAction;
    QIcon adjustToFullSizeAction18x18;
    QIcon fitToPageAction;
    QIcon fitToPageAction18x18;
    QIcon continuousScrollAction;
    QIcon continuousScrollAction18x18;
    QIcon showFlowAction;
    QIcon showFlowAction18x18;
};

struct ViewerTheme {
    QColor defaultBackgroundColor;
    QColor defaultTextColor;

    QColor infoBackgroundColor;

    QString infoLabelQSS;
};

struct GoToFlowWidgetTheme {
    QColor flowBackgroundColor;
    QColor flowTextColor;
    QColor toolbarBackgroundColor;
    QString sliderQSS;
    QString editQSS;
    QString buttonQSS;
    QString labelQSS;
    QIcon centerIcon;
    QIcon goToIcon;
};

struct ShortcutsIconsTheme {
    QIcon comicsIcon;
    QIcon generalIcon;
    QIcon magnifyingGlassIcon;
    QIcon pageIcon;
    QIcon readingIcon;
};

struct DialogIconsTheme {
    QIcon findFolderIcon;
};

struct Theme {
    ThemeMeta meta;
    QJsonObject sourceJson;

    TranslatorTheme translator;
    ToolbarTheme toolbar;
    ViewerTheme viewer;
    GoToFlowWidgetTheme goToFlowWidget;
    HelpAboutDialogTheme helpAboutDialog;
    WhatsNewDialogTheme whatsNewDialog;
    ShortcutsIconsTheme shortcutsIcons;
    DialogIconsTheme dialogIcons;
};

#endif // THEME_H
