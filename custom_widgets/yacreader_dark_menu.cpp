#include "yacreader_dark_menu.h"

#include <QLabel>
#include <QPixmap>
#include <QHBoxLayout>

YACReaderDarkMenu::YACReaderDarkMenu(QWidget *parent)
    : QMenu(parent)
{
    // solid color: #454545
    QString style = "QMenu {background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #6B6B6B, stop: 1 #424242); "
                    "border-left: 1px solid qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #BCBCBC, stop: 1 #4C4C4C);"
                    "border-right: 1px solid qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #BCBCBC, stop: 1 #4C4C4C);"
                    "border-top: 1px solid #BCBCBC;"
                    "border-bottom: 1px solid #4C4C4C;"
                    "padding-top:5px;padding-bottom:5px;}"
                    "QMenu::separator {height:0px;border-top: 1px solid #292929; border-bottom:1px solid #737373; margin-left:-1px; margin-right:-1px;}"
                    "QMenu::item {color:#CFD1D1;padding: 5px 25px 5px 32px;}"
                    "QMenu::item::selected {background-color:#242424;border-top: 1px solid #151515; border-bottom:1px solid #737373;}"
                    "QMenu::icon {padding-left:15px;}";

    setStyleSheet(style);

    /*
        QPixmap p(":/images/icon.png");
        QLabel * l = new QLabel();
        l->setPixmap(p);
        l->move(0,-10);

        //test
        YACReaderDarkMenu * customMenu = new YACReaderDarkMenu(this);
        customMenu->addAction(toggleFullScreenAction);
        customMenu->addAction(createLibraryAction);
        customMenu->addSeparator();
        customMenu->addAction(openComicAction);
        customMenu->show();
        */
}