#ifndef THEME_H
#define THEME_H

#include <QtGui>

#include "yacreader_icon.h"

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
                                  "QHeaderView::section:horizontal {background-color:%5; border-bottom:1px solid %6; border-right:1px solid qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 %5, stop: 1 %6); border-left:none; border-top:none; padding:4px; color:%7;}"
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
    ComicVineTheme comicVine;
};

#endif // THEME_H
