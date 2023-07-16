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
    text->setText("New metadata support and better search engine:<br/>"
                  "<br/>"
                  "<span style=\"font-weight:600\">YACReader</span><br/>"
                  "   &#8226; Add shortcuts for moving by one the double page mode to the left/right, by default CTRL + SHIFT + LEFT, and,  CTRL + SHIFT + RIGHT. Remember that you can change any shortcut in the shortcuts dialog. You can also achieve this functionality using the 'Go to page flow' and chossing the page you want to be on the left/right (depending on the reading mode comic/manga).<br/>"
                  "<br/>"
                  "<span style=\"font-weight:600\">YACReaderLibrary</span><br/>"
                  "   &#8226; Avoid showing stale information in the server config dialog by updating the connection information when the dialog is opened.<br/>"
                  "   &#8226; Add new metadata support, it improves compatibility with ComicInfo.xml.<br/>"
                  "   &#8226; Add support for showing a 'recently added/updated' indicator.<br/>"
                  "   &#8226; Improved comic metadata dialog.<br/>"
                  "   &#8226; Add textual tags support that can be queried through the search engine.<br/>"
                  "   &#8226; Make '=' in the search engine work as ':' does.<br/>"
                  "   &#8226; Add new operators to the search engine: exact match ==, <, >, <=, >=.<br/>"
                  "   &#8226; Support filtering by since/before dates in the search engine. e.g. `added > 7` means recent content added since 7 days ago, and `added < 30` means content added before the last 30 days.<br/>"
                  "   &#8226; Show the full library path in the dialog shown to warn about missing libraries.<br/>"
                  "   &#8226; Fix scroll bar in the info comics view in Qt6 builds.<br/>"
                  "   &#8226; New `Recent` smart list, it will show recent comics added.<br/>"
                  "   &#8226; Try to detect changed files with the same name on libraries updates, there is also a new setting to decide if the modified date of a file should be used as an indicator to know if the file has been changed since it was added (disabled by default, it shouldn't be neccessary in most cases).<br/>"
                  "   &#8226; Fix alphanumeric navigation in the folders tree view. Shortcuts were interfering the default behaviour, now some shortcuts will be ignored if the folders tree has the focus.<br/>"
                  "   &#8226; Fix sorting in the Comic Vine series selection dialog.<br/>"
                  "   &#8226; Fix getting only distinct rows when querying folders through the search engine. The fix is noticeable only in the iOS and Android clients.<br/>"
                  "   &#8226; Fix crash when dropping comics in sublists. (new in 9.13.1)<br/>"
                  "<br/>"
                  "I hope you enjoy the new update. Please, if you like YACReader consider to become a patron in <a href=\"https://www.patreon.com/yacreader\" style=\"color:#E8B800;\">Patreon</a> "
                  "or donate some money using <a href=\"https://www.paypal.com/donate?business=5TAMNQCDDMVP8&item_name=Support+YACReader\" style=\"color:#E8B800;\">Pay-Pal</a> and help keeping the project alive. "
                  "Remember that there is an iOS version available in the <a href=\"https://apps.apple.com/app/id635717885\" style=\"color:#E8B800;\">Apple App Store</a>, "
                  "and there is a brand new app for Android that you can get on the <a href=\"https://play.google.com/store/apps/details?id=com.yacreader.yacreader\" style=\"color:#E8B800;\">Google Play Store</a>.");
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
