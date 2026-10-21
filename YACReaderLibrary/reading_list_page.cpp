#include "reading_list_page.h"

#include <QBuffer>
#include <QCoreApplication>
#include <QImage>
#include <QLocale>

#include <algorithm>

namespace {

const char *style = R"(
:root {
  --background: #f5f4f1;
  --card: #ffffff;
  --text: #1d1d1f;
  --muted: #6e6e73;
  --line: #e8e6e1;
  --cover: #dddbd6;
  --missing: #9a5800;
  --missing-background: #fdf1dc;
}
@media (prefers-color-scheme: dark) {
  :root {
    --background: #151517;
    --card: #1f1f22;
    --text: #f2f2f4;
    --muted: #9b9ba2;
    --line: #2d2d31;
    --cover: #2d2d31;
    --missing: #f2b65e;
    --missing-background: #3a2b12;
  }
}
* { box-sizing: border-box; }
body {
  margin: 0;
  background: var(--background);
  color: var(--text);
  font: 15px/1.4 -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, "Helvetica Neue", Arial, sans-serif;
  -webkit-font-smoothing: antialiased;
}
main { max-width: 760px; margin: 0 auto; padding: 56px 24px 64px; }
header { margin: 0 4px 28px; }
h1 { margin: 0 0 6px; font-size: 34px; line-height: 1.15; font-weight: 700; letter-spacing: -0.02em; overflow-wrap: anywhere; }
.summary { margin: 0; color: var(--muted); font-size: 14px; }
ol {
  list-style: none;
  margin: 0;
  padding: 0;
  background: var(--card);
  border-radius: 14px;
  box-shadow: 0 1px 2px rgba(0, 0, 0, 0.06), 0 10px 30px rgba(0, 0, 0, 0.07);
  overflow: hidden;
}
li {
  display: grid;
  grid-template-columns: 2.5em 56px minmax(0, 1fr) auto;
  align-items: center;
  gap: 16px;
  padding: 12px 20px 12px 12px;
  border-top: 1px solid var(--line);
  break-inside: avoid;
}
li:first-child { border-top: 0; }
.position { text-align: right; color: var(--muted); font-size: 13px; font-variant-numeric: tabular-nums; }
.cover { display: block; width: 56px; height: 84px; border-radius: 4px; object-fit: cover; background: var(--cover); box-shadow: 0 1px 3px rgba(0, 0, 0, 0.22); }
.title { font-weight: 600; overflow-wrap: anywhere; }
.details { margin-top: 2px; color: var(--muted); font-size: 13px; overflow-wrap: anywhere; }
.badge {
  padding: 3px 9px;
  border-radius: 999px;
  background: var(--missing-background);
  color: var(--missing);
  font-size: 11px;
  font-weight: 600;
  letter-spacing: 0.06em;
  text-transform: uppercase;
  white-space: nowrap;
}
li.missing .cover { filter: grayscale(1); opacity: 0.55; }
li.missing .title { color: var(--muted); }
footer { margin-top: 24px; color: var(--muted); font-size: 12px; text-align: center; }
@media (max-width: 520px) {
  main { padding: 28px 12px 40px; }
  h1 { font-size: 26px; }
  li { grid-template-columns: 2em 44px minmax(0, 1fr); gap: 12px; }
  .cover { width: 44px; height: 66px; }
  .badge { grid-column: 3; justify-self: start; }
}
@media print {
  :root { --background: #ffffff; --card: #ffffff; --text: #000000; --muted: #555555; --line: #dddddd; --missing: #8a4f00; --missing-background: #ffffff; }
  main { max-width: none; padding: 0; }
  ol { border-radius: 0; box-shadow: none; overflow: visible; }
  .badge { border: 1px solid currentColor; }
}
)";

QString escaped(const QString &text)
{
    return text.toHtmlEscaped();
}

}

QByteArray writeReadingListPage(const QString &name, const QList<ReadingListPageEntry> &entries, const QDate &exported)
{
    const auto missing = std::count_if(entries.cbegin(), entries.cend(), [](const ReadingListPageEntry &entry) { return entry.missing; });
    auto summary = QCoreApplication::translate("ReadingListPage", "%n comic(s)", nullptr, int(entries.size()));
    if (missing > 0)
        summary += QStringLiteral(" · ") + QCoreApplication::translate("ReadingListPage", "%n missing", nullptr, int(missing));

    QString page;
    page += QStringLiteral("<!DOCTYPE html>\n<html>\n<head>\n<meta charset=\"utf-8\">\n"
                           "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n"
                           "<meta name=\"generator\" content=\"YACReader\">\n");
    page += QStringLiteral("<title>%1</title>\n<style>%2</style>\n</head>\n<body>\n<main>\n").arg(escaped(name), QString::fromUtf8(style));
    page += QStringLiteral("<header>\n<h1>%1</h1>\n<p class=\"summary\">%2</p>\n</header>\n<ol>\n").arg(escaped(name), escaped(summary));

    int position = 0;
    for (const auto &entry : entries) {
        const auto cover = entry.cover.isEmpty()
                ? QStringLiteral("<div class=\"cover\"></div>")
                : QStringLiteral("<img class=\"cover\" src=\"data:image/jpeg;base64,%1\" alt=\"\">").arg(QString::fromLatin1(entry.cover.toBase64()));
        page += QStringLiteral("<li%1><span class=\"position\">%2</span>%3<div><div class=\"title\">%4</div>")
                        .arg(entry.missing ? QStringLiteral(" class=\"missing\"") : QString(), QString::number(++position), cover, escaped(entry.title));
        if (!entry.details.isEmpty())
            page += QStringLiteral("<div class=\"details\">%1</div>").arg(escaped(entry.details));
        page += QStringLiteral("</div>");
        if (entry.missing)
            page += QStringLiteral("<span class=\"badge\">%1</span>").arg(escaped(QCoreApplication::translate("ReadingListPage", "Missing")));
        page += QStringLiteral("</li>\n");
    }

    page += QStringLiteral("</ol>\n<footer>%1</footer>\n</main>\n</body>\n</html>\n")
                    .arg(escaped(QCoreApplication::translate("ReadingListPage", "Exported from YACReader on %1").arg(QLocale().toString(exported, QLocale::LongFormat))));
    return page.toUtf8();
}

QByteArray readingListPageCover(const QString &path)
{
    QImage image(path);
    if (image.isNull())
        return { };

    // twice the size on the page, for high density screens
    if (image.height() > 168)
        image = image.scaledToHeight(168, Qt::SmoothTransformation);
    QByteArray jpeg;
    QBuffer buffer(&jpeg);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "JPG", 82);
    return jpeg;
}

QString readingListPageDetails(const QString &series, const QString &number, const QString &volume, const QString &date)
{
    QString issue = series;
    if (!number.isEmpty())
        issue += (issue.isEmpty() ? QStringLiteral("#") : QStringLiteral(" #")) + number;
    if (!volume.isEmpty())
        issue += (issue.isEmpty() ? QString() : QStringLiteral(" ")) + QStringLiteral("(%1)").arg(volume);

    QString when;
    if (const auto parts = date.split('/'); parts.size() == 3) {
        if (const QDate day(parts[2].toInt(), parts[1].toInt(), parts[0].toInt()); day.isValid())
            when = QLocale().toString(day, QStringLiteral("MMM yyyy"));
        else if (parts[2].toInt() > 0)
            when = parts[2];
    }

    if (issue.isEmpty() || when.isEmpty())
        return issue + when;
    return issue + QStringLiteral(" · ") + when;
}
