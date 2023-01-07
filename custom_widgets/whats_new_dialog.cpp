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

    auto headerLabel = new QLabel();
    headerLabel->setText("What's New in\nYACReader");
    QFont headerLabelFont("Arial", 34, QFont::ExtraBold);
    headerLabel->setFont(headerLabelFont);
    headerLabel->setAlignment(Qt::AlignCenter);
    headerLabel->setStyleSheet("padding: 18px 0 0 0;"
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
    text->setText("A small update with a bunch of fixes:<br/>"
                  "<br/>"
                  "<span style=\"font-weight:600\">YACReader</span><br/>"
                  "   &#8226; Fix crash when exiting YACReader while it is processing a comic.<br/>"
                  "   &#8226; Fix last read page calculation in double page mode.<br/>"
                  "<br/>"
                  "<span style=\"font-weight:600\">YACReaderLibrary</span><br/>"
                  "   &#8226; Fix drag&drop in the comics grid view.<br/>"
                  "   &#8226; Detect back/forward mouse buttons to move back and forward through the browsing history.<br/>"
                  "   &#8226; Fix crash when disabling the server.<br/>"
                  "<br/>"
                  "<span style=\"font-weight:600\">All apps</span><br/>"
                  "   &#8226; Add support for poppler-qt6 pdf backend (only relevat if you are building YACReader yourself).<br/>"
                  "   &#8226; Remove image allocation limit in Qt6.<br/>"
                  "<br/>"
                  "NOTE: Importing metadata from ComicInfo.XML in now disabled by default, if you want you can enable it Settings -> General.<br/>"
                  "<br/>"
                  "I hope you enjoy the new update. Please, if you like YACReader consider to become a patron in <a href=\"https://www.patreon.com/yacreader\" style=\"color:#E8B800;\">Patreon</a> or donate some money using <a href=\"https://www.paypal.com/donate?business=5TAMNQCDDMVP8&item_name=Support+YACReader\" style=\"color:#E8B800;\">Pay-Pal</a> and help keeping the project alive. Remember that there is an iOS version available in the <a href=\"https://apps.apple.com/app/id635717885\" style=\"color:#E8B800;\">Apple App Store</a>.");
    QFont textLabelFont("Arial", 15, QFont::Light);
    text->setFont(textLabelFont);
    text->setStyleSheet("padding:51px;"
                        "background-color:transparent;"
                        "color:#0A0A0A;");
    text->setWordWrap(true);
    text->setOpenExternalLinks(true);

    contentLayout->addItem(new QSpacerItem(0, 50), 0, 0);
    contentLayout->addWidget(headerImageLabel, 1, 0, Qt::AlignTop | Qt::AlignHCenter);
    contentLayout->addWidget(headerLabel, 1, 0, Qt::AlignTop | Qt::AlignHCenter);
    contentLayout->addWidget(versionLabel, 2, 0, Qt::AlignTop | Qt::AlignHCenter);
    contentLayout->addWidget(text, 3, 0, Qt::AlignTop);
    contentLayout->setRowStretch(3, 1);
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
