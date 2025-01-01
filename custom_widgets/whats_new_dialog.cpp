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
    text->setText("This version brings new features, improved functionality, enhanced customization options, bug fixes, and performance improvements across all apps: <br/>"
                  "<br/>"
                  "<span style=\"font-weight:600\">YACReader</span><br/>"
                  "   &#8226; Save magnifying glass size and zoom level<br/>"
                  "   &#8226; Add shortcut to reset the magnifying glass to its defaults (size and zoom), it is `slash` by default but it can be reassigned<br/>"
                  "   &#8226; Bump PDF render size<br/>"
                  "   &#8226; Fix trackpad scrolling, it makes using trackpads more responsive and natural<br/>"
                  "   &#8226; Added more info to Help -> System info<br/>"
                  "<br/>"
                  "<span style=\"font-weight:600\">YACReaderLibrary</span><br/>"
                  "   &#8226; Fix headers in the table view getting stuck in a non-movable state<br/>"
                  "   &#8226; Add option to set the type of a library. It will convert all the content to the desired type (comic, manga, etc.) and set that type as the default for the library. Available in the library context menu<br/>"
                  "   &#8226; Added more info to Help -> System info<br/>"
                  "   &#8226; New setting to open comics in third-party reader apps by entering a command that launches the app, e.g., \"/path/to/the/app {comic_file_path}\". Use `{comic_file_path}` as a placeholder where `YACReaderLibrary` places the path to the comic file<br/>"
                  "   &#8226; Purge covers and metadata not being used after a full library update<br/>"
                  "   &#8226; Fix crash when updating the current folder content after a library update<br/>"
                  "   &#8226; Fix crash when current folder is empty after an update<br/>"
                  "   &#8226; Enable dropping content on the FolderContentView<br/>"
                  "   &#8226; Fix `open containing folder...` shortcut for comics<br/>"
                  "   &#8226; Add a dialog to show information about a library, including the number of folders, comics, and read comics<br/>"
                  "   &#8226; Fix occasional crashes when using automatic library updates<br/>"
                  "   &#8226; Add setting to hide the \"Continue Reading...\" banner from the home view<br/>"
                  "   &#8226; Improve Grid and Flow Info comics view scroll performance<br/>"
                  "<br/>"
                  "<span style=\"font-weight:600\">YACReaderLibraryServer</span><br/>"
                  "   &#8226; New command --system-info to print information about the execution environment and available resources (including what image formats are supported and what libraries are used by the app).<br/>"
                  "   &#8226; Fix automatic libraries updates not being triggered.<br/>"
                  "<br/>"
                  "<span style=\"font-weight:600\">All apps</span><br/>"
                  "   &#8226; Sorting heuristic to try to find spreads in the content of a comic is now only used for files with less than 1000 pages to avoid false positives.<br/>"
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
