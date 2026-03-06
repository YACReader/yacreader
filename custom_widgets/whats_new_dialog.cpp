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
    htmlTemplate = "YACReader 10 is finally here!! This is all the exciting new features and improvements: <br/>"
                   "<br/>"
                   "<span style=\"font-weight:600\">YACReader</span><br/>"
                   "   &#8226; Add support for continuous scroll mode<br/>"
                   "   &#8226; Fix the translator<br/>"
                   "   &#8226; Add Lanczos interpolation for image scaling. You can control the method used via the settings under <i>Image adjustments</i><br/>"
                   "<br/>"
                   "<span style=\"font-weight:600\">All GUI Apps</span><br/>"
                   "   &#8226; Migrate Flow implementation from OpenGL to QRhi. This is a full new implementation with better performance and compatibility with operating systems and hardware<br/>"
                   "   &#8226; Add light/dark themes support that follow the system configuration<br/>"
                   "   &#8226; Add a theme editor and support for custom themes<br/>"
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
