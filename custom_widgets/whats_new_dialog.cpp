#include "whats_new_dialog.h"
#include "yacreader_global.h"

#include <QtWidgets>

YACReader::WhatsNewDialog::WhatsNewDialog(QWidget *parent)
    : RoundedCornersDialog(parent)
{
    auto scrollArea = new QScrollArea(this);
    scrollArea->setStyleSheet("background-color:transparent;"
                              "border:none;");
    scrollArea->horizontalScrollBar()->setStyleSheet("QScrollBar {height:0px;}");
    scrollArea->verticalScrollBar()->setStyleSheet("QScrollBar {width:0px;}");
    scrollArea->setContentsMargins(0, 0, 0, 0);

    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    auto contentLayout = new QGridLayout();

    auto content = new QFrame();

    content->setStyleSheet("background-color:transparent;"
                           "border:none;");

    headerImageLabel = new QLabel();

    headerLabel = new QLabel();
    headerLabel->setText("What's New in\nYACReader");
    QFont headerLabelFont("Arial", 34, QFont::ExtraBold);
    headerLabel->setFont(headerLabelFont);
    headerLabel->setAlignment(Qt::AlignCenter);

    versionLabel = new QLabel();
    versionLabel->setText(VERSION);
    QFont versionLabelFont("Arial", 12, QFont::Normal);
    versionLabel->setFont(versionLabelFont);
    versionLabel->setAlignment(Qt::AlignCenter);

    textLabel = new QLabel();
    htmlTemplate = "This version brings exciting new features, improved functionality, enhanced customization options, bug fixes, and performance improvements across all apps: <br/>"
                   "<br/>"
                   "<span style=\"font-weight:600\">YACReader</span><br/>"
                   "   &#8226; Don't use scroll animations on macOS by default, where hdpi scroll is most likely to be used<br/>"
                   "   &#8226; New toolbar on macOS<br/>"
                   "   &#8226; New mouse modes to turn pages - you can setup the app to use the left/right buttons to turn pages directly or click on the left/right part of the screen to turn pages<br/>"
                   "   &#8226; Fix current page/time label content when the content is too long. (new in 9.16.3)<br/>"
                   "<br/>"
                   "<span style=\"font-weight:600\">YACReaderLibrary</span><br/>"
                   "   &#8226; Improve flexibility of the open comic in third party app setting so more complex commands can be used, e.g. `open -a \"/Applications/My Reader.app\" \"{comic_file_path}\"`<br/>"
                   "   &#8226; Fix setting the comic rating in the table view<br/>"
                   "   &#8226; Log libraries validation when the app starts<br/>"
                   "   &#8226; New toolbar on macOS<br/>"
                   "   &#8226; New setting in Comic Vine scraper to force exact volume matches<br/>"
                   "   &#8226; Better default search query in the Comic Vine scraper<br/>"
                   "   &#8226; Improved navigation in Comic Vine scraper, including keeping the current query around to make edits and refined searches easier<br/>"
                   "   &#8226; Add support for custom covers for any folder using the context menu<br/>"
                   "   &#8226; The edit cover buttons now support looping through pages, going forward from the last returns to the first, and going backward from the first jumps to the last<br/>"
                   "   &#8226; Add support for custom covers for comics using the edit metadata dialog, you can use a pick file button or drag&drop an image into the cover view in the dialog<br/>"
                   "   &#8226; Covers can be set in bulk for various comics at once<br/>"
                   "   &#8226; New button to reset to the default cover of a comic<br/>"
                   "   &#8226; Support for storing the new image filters from iOS and Android apps<br/>"
                   "   &#8226; Fixed cover loading in Comic Vine scraper (new in 9.16.1)<br/>"
                   "   &#8226; Added a customizable User Agent string to use it with Comic Vine. It can be set in YACReaderLibrary.ini in the [ComicVine] section using the COMIC_VINE_USER_AGENT key (new in 9.16.2)<br/>"
                   "   &#8226; Prevent crash when opening the folders context menu if a folder is not selected. (new in 9.16.2)<br/>"
                   "   &#8226; Fix crash when using the `Set type` menu on libraries. (new in 9.16.2)<br/>"
                   "   &#8226; Fix table view last section stretch. Before it was only working randomly. (new in 9.16.3)<br/>"
                   "   &#8226; Fix empty table view caused by wront state being saved. You'll need to reconfigure the table view headers to your liking after this udpate. (new in 9.16.3)<br/>"
                   "<br/>"
                   "<span style=\"font-weight:600\">YACReaderLibraryServer</span><br/>"
                   "   &#8226; Log libraries validation when the app starts<br/>"
                   "<br/>"
                   "<span style=\"font-weight:600\">All apps</span><br/>"
                   "   &#8226; PDF support has been updated in all Windows apps<br/>"
                   "<br/>"
                   "I hope you enjoy the new update. Please, if you like YACReader consider to become a patron in <a href=\"https://www.patreon.com/yacreader\" style=\"color:%1;\">Patreon</a> "
                   "or donate some money using <a href=\"https://www.paypal.com/donate?business=5TAMNQCDDMVP8&item_name=Support+YACReader\" style=\"color:%1;\">Pay-Pal</a> and help keeping the project alive. "
                   "Remember that there is an iOS version available in the <a href=\"https://apps.apple.com/app/id635717885\" style=\"color:%1;\">Apple App Store</a>, "
                   "and there is a brand new app for Android that you can get on the <a href=\"https://play.google.com/store/apps/details?id=com.yacreader.yacreader\" style=\"color:%1;\">Google Play Store</a>.";

    QFont textLabelFont("Arial", 15, QFont::Light);
    textLabel->setFont(textLabelFont);
    textLabel->setWordWrap(true);
    textLabel->setOpenExternalLinks(true);

    contentLayout->addItem(new QSpacerItem(0, 50), 0, 0);
    contentLayout->addWidget(headerImageLabel, 1, 0, Qt::AlignTop | Qt::AlignHCenter);
    contentLayout->addWidget(headerLabel, 1, 0, Qt::AlignTop | Qt::AlignHCenter);
    contentLayout->addWidget(versionLabel, 2, 0, Qt::AlignTop | Qt::AlignHCenter);
    contentLayout->addWidget(textLabel, 3, 0, Qt::AlignTop);
    contentLayout->setRowStretch(3, 1);
    content->setLayout(contentLayout);

    mainLayout->addWidget(scrollArea);

    scrollArea->setWidget(content);
    scrollArea->setWidgetResizable(true);

    this->setLayout(mainLayout);

    closeButton = new QPushButton(this);
    closeButton->setFlat(true);
    closeButton->setStyleSheet("background-color:transparent;");
    closeButton->setIconSize(QSize(44, 44));
    closeButton->setFixedSize(44, 44);
    closeButton->move(656, 20);

    scrollArea->setFixedSize(720, 640);
    setFixedSize(720, 640);
    setModal(true);

    connect(closeButton, &QPushButton::clicked, this, &QDialog::close);

    initTheme(this);
}

void YACReader::WhatsNewDialog::applyTheme(const Theme &theme)
{
    auto whatsNewTheme = theme.whatsNewDialog;

    setBackgroundColor(whatsNewTheme.backgroundColor);

    headerImageLabel->setPixmap(whatsNewTheme.headerDecoration);
    headerImageLabel->setFixedSize(whatsNewTheme.headerDecoration.size());

    headerLabel->setStyleSheet(QString("padding: 18px 0 0 0;"
                                       "background-color:transparent;"
                                       "color:%1;")
                                       .arg(whatsNewTheme.headerTextColor.name()));

    versionLabel->setStyleSheet(QString("padding:0 0 0 0;"
                                        "background-color:transparent;"
                                        "color:%1;")
                                        .arg(whatsNewTheme.versionTextColor.name()));

    textLabel->setStyleSheet(QString("padding:51px;"
                                     "background-color:transparent;"
                                     "color:%1;")
                                     .arg(whatsNewTheme.contentTextColor.name()));
    textLabel->setText(htmlTemplate.arg(whatsNewTheme.linkColor.name()));

    closeButton->setIcon(whatsNewTheme.closeButtonIcon);
}
