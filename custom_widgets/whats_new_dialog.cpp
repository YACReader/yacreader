#include "whats_new_dialog.h"

#include "yacreader_global.h"

#include <QFile>
#include <QFrame>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QStringList>
#include <QVBoxLayout>

namespace {

QString renderInlineMarkdown(QString text)
{
    QString html;
    bool inlineCode = false;

    for (const auto character : text) {
        if (character == '`') {
            html += inlineCode ? "</i>" : "<i>";
            inlineCode = !inlineCode;
        } else {
            html += QString(character).toHtmlEscaped();
        }
    }

    return html;
}

} // namespace

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
    QFont textLabelFont("Arial", 15, QFont::Light);
    textLabel->setFont(textLabelFont);
    textLabel->setWordWrap(true);
    textLabel->setOpenExternalLinks(true);
    textLabel->setTextFormat(Qt::RichText);
    textLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);

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

    loadChangelog();
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

    contentTextColor = whatsNewTheme.contentTextColor.name();
    backgroundColor = whatsNewTheme.backgroundColor.name();
    textLabel->setStyleSheet(QString("padding:51px 51px 51px 51px;"
                                     "background-color:transparent;"
                                     "color:%1;")
                                     .arg(contentTextColor));
    linkColor = whatsNewTheme.linkColor.name();
    renderChangelog();

    closeButton->setIcon(whatsNewTheme.closeButtonIcon);
}

void YACReader::WhatsNewDialog::loadChangelog()
{
    QFile changelogFile(":/files/CHANGELOG.md");
    if (!changelogFile.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    const QString changelog = QString::fromUtf8(changelogFile.readAll());
    const auto lines = changelog.split('\n');
    QString currentEntry;

    for (const auto &line : lines) {
        if (line.startsWith("## ")) {
            if (!currentEntry.trimmed().isEmpty())
                changelogEntries.append(currentEntry.trimmed());

            currentEntry = line;
            continue;
        }

        if (!currentEntry.isEmpty())
            currentEntry += "\n" + line;
    }

    if (!currentEntry.trimmed().isEmpty())
        changelogEntries.append(currentEntry.trimmed());

    if (changelogEntries.isEmpty())
        return;

    const auto latestVersionSeries = versionSeriesFromEntry(changelogEntries.first());
    while (latestEntryCount < changelogEntries.size() && versionSeriesFromEntry(changelogEntries.at(latestEntryCount)) == latestVersionSeries)
        ++latestEntryCount;
}

void YACReader::WhatsNewDialog::renderChangelog()
{
    textLabel->setText(renderHtmlDocument(renderBody()));
}

QString YACReader::WhatsNewDialog::renderBody() const
{
    QString html;
    if (changelogEntries.isEmpty()) {
        html = QString("<div class=\"intro\">%1</div>").arg(tr("Release notes are not available.").toHtmlEscaped());
    } else {
        html = QString("<div class=\"intro\">%1</div>%2<div class=\"footer\">%3</div>")
                       .arg(renderIntro(), renderLatestChangelogEntries(), renderFooter());
    }

    if (changelogEntries.size() > latestEntryCount)
        html += renderPreviousChangelogEntries();

    return html;
}

QString YACReader::WhatsNewDialog::renderChangelogEntry(const QString &entry, bool includeVersionHeader, bool flushVersionTopMargin) const
{
    QString html;
    const auto lines = entry.split('\n');
    bool hasSection = false;

    for (const auto &line : lines) {
        const auto trimmedLine = line.trimmed();

        if (trimmedLine.isEmpty()) {
            continue;
        } else if (trimmedLine.startsWith("## ")) {
            if (includeVersionHeader) {
                const QString versionClass = flushVersionTopMargin ? "version version-flush" : "version";
                html += QString("<div class=\"%1\">%2</div>").arg(versionClass, renderInlineMarkdown(trimmedLine.mid(3)));
            }
        } else if (trimmedLine.startsWith("### ")) {
            html += QString("<div class=\"section%1\">%2</div>").arg(hasSection ? " section-spaced" : "").arg(renderInlineMarkdown(trimmedLine.mid(4)));
            hasSection = true;
        } else if (trimmedLine.startsWith("* ")) {
            html += QString("<div class=\"bullet\">&#8226;&nbsp;%1</div>").arg(renderInlineMarkdown(trimmedLine.mid(2)));
        } else {
            html += QString("<div class=\"paragraph\">%1</div>").arg(renderInlineMarkdown(trimmedLine));
        }
    }

    return QString("<div class=\"entry\">%1</div>").arg(html);
}

QString YACReader::WhatsNewDialog::renderLatestChangelogEntries() const
{
    QString html = "<div class=\"latest\">";
    const bool latestGroupHasMultipleVersions = latestEntryCount > 1;

    for (int i = 0; i < latestEntryCount; ++i) {
        html += renderChangelogEntry(changelogEntries.at(i), latestGroupHasMultipleVersions, i == 0);
    }

    html += "</div>";
    return html;
}

QString YACReader::WhatsNewDialog::renderPreviousChangelogEntries() const
{
    QString html = QString("<div class=\"previous-title\">%1</div><div class=\"previous\">").arg(tr("Previous versions").toHtmlEscaped());

    for (int i = latestEntryCount; i < changelogEntries.size(); ++i) {
        const bool isFirstPrevious = (i == latestEntryCount);
        html += renderChangelogEntry(changelogEntries.at(i), true, isFirstPrevious);
    }

    html += "</div>";
    return html;
}

QString YACReader::WhatsNewDialog::renderHtmlDocument(const QString &content) const
{
    return QString("<html>"
                   "<head>"
                   "<style type=\"text/css\">"
                   "body { margin: 0; color: %1; background-color: %2; font-family: Arial; font-size: 14.5pt; font-weight: 300; line-height: 1.3; }"
                   ".intro { margin-bottom: 32px; }"
                   ".version { font-size: 20pt; font-weight: 700; margin-top: 48px; margin-bottom: 20px; }"
                   ".version-flush { margin-top: 0; }"
                   ".section { font-size: 16pt; font-weight: 700; margin-bottom: 8px; }"
                   ".section-spaced { margin-top: 24px; }"
                   ".bullet { margin-bottom: 4px; }"
                   ".paragraph { margin-bottom: 12px; }"
                   ".footer { margin-top: 32px; margin-bottom: 64px; }"
                   ".previous-title { font-size: 24pt; font-weight: 700; margin-bottom: 20px; }"
                   "a { color: %3; }"
                   "</style>"
                   "</head>"
                   "<body>%4</body>"
                   "</html>")
            .arg(contentTextColor, backgroundColor, linkColor, content);
}

QString YACReader::WhatsNewDialog::renderIntro() const
{
    return "YACReader 10.1 is here, with smoother reading, better page saving and exporting, Windows long path support, a refreshed server web UI and more:";
}

QString YACReader::WhatsNewDialog::renderFooter() const
{
    return QString("I hope you enjoy the new update. Please, if you like YACReader consider to become a patron in <a href=\"https://www.patreon.com/yacreader\" style=\"color:%1;\">Patreon</a> "
                   "or donate some money using <a href=\"https://www.paypal.com/donate?business=5TAMNQCDDMVP8&item_name=Support+YACReader\" style=\"color:%1;\">Pay-Pal</a> and help keeping the project alive. "
                   "Remember that there is an iOS version available in the <a href=\"https://apps.apple.com/app/id635717885\" style=\"color:%1;\">Apple App Store</a>, "
                   "and there is a brand new app for Android that you can get on the <a href=\"https://play.google.com/store/apps/details?id=com.yacreader.yacreader\" style=\"color:%1;\">Google Play Store</a>.")
            .arg(linkColor);
}

QString YACReader::WhatsNewDialog::versionSeriesFromEntry(const QString &entry) const
{
    const auto firstLineEnd = entry.indexOf('\n');
    const auto firstLine = (firstLineEnd >= 0 ? entry.left(firstLineEnd) : entry).trimmed();
    if (!firstLine.startsWith("## "))
        return QString();

    const auto version = firstLine.mid(3).section(' ', 0, 0);
    const auto versionParts = version.split('.');

    if (versionParts.size() < 2)
        return version;

    return versionParts.at(0) + "." + versionParts.at(1);
}
