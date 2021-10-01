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

    auto headerImageLabel = new QLabel();
    QPixmap headerImage(":/images/whats_new/whatsnew_header.svg");
    headerImageLabel->setPixmap(headerImage);
    headerImageLabel->setFixedSize(headerImage.size());
    headerImageLabel->move(50, 20);

    auto headerLabel = new QLabel();
    headerLabel->setText("What's New in\nYACReader");
    QFont headerLabelFont("Arial", 34, QFont::ExtraBold);
    headerLabel->setFont(headerLabelFont);
    headerLabel->setAlignment(Qt::AlignCenter);
    headerLabel->setStyleSheet("padding:18px 0 0 0;"
                               "background-color:transparent;"
                               "color:#0A0A0A;");

    auto versionLabel = new QLabel();
    versionLabel->setText(VERSION);
    QFont versionLabelFont("Arial", 12, QFont::Normal);
    versionLabel->setFont(versionLabelFont);
    versionLabel->setAlignment(Qt::AlignCenter);
    versionLabel->setStyleSheet("padding:0 0 0 0;"
                                "background-color:transparent;"
                                "color:#858585;");

    auto text = new QLabel();
    text->setText("Hey, a new version is here, check the new stuff including this <i>What's new</i> dialog:<br/>"
                  "<br/>"
                  "<span style=\"font-weight:600\">YACReader</span><br/>"
                  "   &#8226; Add support for full manga mode.<br/>"
                  "   &#8226; Fix UP, DOWN, LEFT, RIGHT shortcuts for moving a zoomed in page around.<br/>"
                  "<br/>"
                  "<span style=\"font-weight:600\">YACReaderLibrary</span><br/>"
                  "   &#8226; New fuzzy search engine. Type to search into any field of the database, or target specific content <i>\"read:false manga:true filename:3x3\"</i>, read more about how it works in the <a href=\"https://www.yacreader.com\" style=\"color:#E8B800;\">web site</a>. <br/>"
                  "   &#8226; New `manga` field added to comics and folders to tag content as manga, any content added to a manga folder will become manga automatically.<br/>"
                  "   &#8226; Support for HTML in comic synopsis, this fixes the synopsis when it comes from Comic Vine with HTML tags.<br/>"
                  "   &#8226; Improve keyboard navigation in Comic Vine dialog. Enter will trigger next or search and Backspace will go back to the previous section.<br/>"
                  "   &#8226; Fixed opening comics from readings lists, now YACReader will follow the right order and it will open the right comics in the list. (new in 9.8.1)<br/>"
                  "   &#8226; Fixed opening comics from the continue reading banner. (new in 9.8.2)<br/>"
                  "   &#8226; Make available next/prev comic covers in the iOS app while reading. (new in 9.8.2)<br/>"
                  "<br/>"
                  "<span style=\"font-weight:600\">Server</span><br/>"
                  "   &#8226; New `manga` field is sent to YACReader for iOS, so comics tagged as manga will be recognized as such when reading remotely or importing comics.<br/>"
                  "   &#8226; Fixed opening comics from readings lists, now YACReader for iOS will follow the right order and it will open the right comics in the list, it needs YACReader for iOS 3.15.0 or newer. (new in 9.8.1).<br/>"
                  "   &#8226; Make available next/prev comic covers in the iOS app while reading. (new in 9.8.2)<br/>"
                  "<br/>"
                  "I hope you enjoy the new update. Please, if you like YACReader consider to become a patron in <a href=\"https://www.patreon.com/yacreader\" style=\"color:#E8B800;\">Patreon</a> or donate some money using <a href=\"https://www.paypal.com/donate?business=5TAMNQCDDMVP8&item_name=Support+YACReader\" style=\"color:#E8B800;\">Pay-Pal</a> and help keeping the project alive. Remember that there is an iOS version available in the <a href=\"https://apps.apple.com/app/id635717885\" style=\"color:#E8B800;\">Apple App Store</a>.");
    QFont textLabelFont("Arial", 15, QFont::Light);
    text->setFont(textLabelFont);
    text->setStyleSheet("padding:51px;"
                        "background-color:transparent;"
                        "color:#0A0A0A;");
    text->setWordWrap(true);
    text->setOpenExternalLinks(true);

    contentLayout->addWidget(headerImageLabel, 0, 0, Qt::AlignTop | Qt::AlignHCenter);
    contentLayout->addWidget(headerLabel, 0, 0, Qt::AlignTop | Qt::AlignHCenter);
    contentLayout->addWidget(versionLabel, 1, 0, Qt::AlignTop | Qt::AlignHCenter);
    contentLayout->addWidget(text, 2, 0, Qt::AlignTop);
    contentLayout->setRowStretch(2, 1);
    content->setLayout(contentLayout);

    mainLayout->addWidget(scrollArea);

    // containerLayout->addWidget(content);
    scrollArea->setWidget(content);
    scrollArea->setWidgetResizable(true);

    this->setLayout(mainLayout);

    auto closeButton = new QPushButton(this);
    closeButton->setFlat(true);
    closeButton->setStyleSheet("background-color:transparent;");
    auto closeIcon = QPixmap(":/images/custom_dialog/custom_close_button.svg");
    if (!closeIcon.isNull()) {
        closeButton->setIcon(QPixmap(":/images/custom_dialog/custom_close_button.svg"));
        closeButton->setIconSize(QSize(44, 44));
        closeButton->setFixedSize(44, 44);
        closeButton->move(656, 20);
    } else {
        closeButton->setText(tr("Close"));
        auto font = closeButton->font();
        font.setPointSize(16);
        closeButton->setFont(font);
        closeButton->move(616, 20);
    }

    scrollArea->setFixedSize(720, 640);
    setFixedSize(720, 640);
    setModal(true);

    connect(closeButton, &QPushButton::clicked, this, &QDialog::close);
}
