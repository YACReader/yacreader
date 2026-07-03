#ifndef WHATSNEWDIALOG_H
#define WHATSNEWDIALOG_H

#include "rounded_corners_dialog.h"
#include "themable.h"

#include <QString>
#include <QStringList>

class QLabel;
class QPushButton;

namespace YACReader {

class WhatsNewDialog : public RoundedCornersDialog, protected Themable
{
    Q_OBJECT
public:
    explicit WhatsNewDialog(QWidget *parent = nullptr);

protected:
    void applyTheme(const Theme &theme) override;

private:
    void loadChangelog();
    void renderChangelog();
    QString renderChangelogEntry(const QString &entry, bool includeVersionHeader, bool flushVersionTopMargin = false) const;
    QString renderLatestChangelogEntries() const;
    QString renderPreviousChangelogEntries() const;
    QString renderHtmlDocument(const QString &content) const;
    QString renderBody() const;
    QString renderIntro() const;
    QString renderFooter() const;
    QString versionSeriesFromEntry(const QString &entry) const;

    QLabel *headerImageLabel;
    QLabel *headerLabel;
    QLabel *versionLabel;
    QLabel *textLabel;
    QPushButton *closeButton;
    QStringList changelogEntries;
    int latestEntryCount = 0;
    QString linkColor;
    QString contentTextColor;
    QString backgroundColor;
};
}

#endif // WHATSNEWDIALOG_H
