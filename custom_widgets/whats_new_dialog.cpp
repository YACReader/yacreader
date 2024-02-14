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
    text->setText("New update that brings options to enable automatic library updates and more: <br/>"
                  "<br/>"
                  "<span style=\"font-weight:600\">YACReader</span><br/>"
                  "   &#8226; Add a setting to disable scroll animations and scroll smoothing, recommended if you are using a touch pad or if you find the mouse wheel behaviour laggy.<br/>"
                  "   &#8226; Fix missing translation when opening YACReader from YACReaderLibrary.<br/>"
                  "<br/>"
                  "<span style=\"font-weight:600\">YACReaderLibrary</span><br/>"
                  "   &#8226; Fix \"Set type\" context menu the grid view for folders<br/>"
                  "   &#8226; Add a different versioning strategy for databases. DBs version will change only when the structure changes and not when YACReader version changes.<br/>"
                  "   &#8226; Add support for updating libraries automatically with various settings to chose from. During automatic library updates most actions are disabled, you can stop an update by clicking on the busy indicator next to the Libraries title.<br/>"
                  "   &#8226; Improve content reloading. Navigation and selection state is no longer reseted after content changes (e.g. library updates, tags edits, etc.).<br/>"
                  "   &#8226; The app will try to move comics and folders to the trash bin when deletions are requested, if the file system used doesn't support trash bin the files will be removed permanetly.<br/>"
                  "   &#8226; Add menu to choose what columns are displayed in the table comics view (do a right click on the header to show it). The view has new 3 new headers to choose from (Series, Volume and Story arc).<br/>"
                  "   &#8226; Migrate `number` and `arcNumber` data types to `TEXT`. This only affects databases created before 9.13 and it fixes problems with some formats of numbers (e.g. 1.10).<br/>"
                  "   &#8226; Propage update date to all the parent folders.<br/>"
                  "   &#8226; Fix crash when resorting comics in reading lists in table view view and the comic flow is hidden.<br/>"
                  "   &#8226; Fix cover loading in QML views due to malformed URLs.<br/>"
                  "   &#8226; Improve style of the webui status page.<br/>"
                  "   &#8226; Fix type not being propagated to new files in a folder.<br/>"
                  "   &#8226; Mark the current type in the context menu so the user can know the current type.<br/>"
                  "   &#8226; Fix columns in the search results. (new in 9.14.2)<br/>"
                  "   &#8226; Fix type not being propagated to new folders from their parents. (new in 9.14.2)<br/>"
                  "   &#8226; Fix default type set to folders in the root folder when they are added. (new in 9.14.2)<br/>"
                  "   &#8226; Improve and fix comic file size format. (new in 9.14.2)<br/>"
                  "<br/>"
                  "<span style=\"font-weight:600\">YACReaderLibraryServer</span><br/>"
                  "   &#8226; Add `rescan-xml-info` command.<br/>"
                  "   &#8226; API to provide better integration with the clients (Android 1.4.0 and iOS 3.29.0).<br/>"
                  "<br/>"
                  "<span style=\"font-weight:600\">All apps</span><br/>"
                  "   &#8226; New universal apps for macOS.<br/>"
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
