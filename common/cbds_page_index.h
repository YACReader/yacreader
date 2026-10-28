#ifndef CBDS_PAGE_INDEX_H
#define CBDS_PAGE_INDEX_H

#include <QString>
#include <QStringList>

// ComicBookDS (.cbds) books are zip archives made for a Nintendo DS reader.
// Besides the pages in IMAGE/ they store several resized copies of every page
// (SMALL_N/, SMALL_R/, THMB_N/, THMB_R/) that must not be shown as pages.
// See docs/CBDS_FORMAT.md for the layout.
namespace YACReaderCbds {

bool isCbds(const QString &path, const QStringList &fileNames);

// Archive entries that may contain pages: the IMAGE/ entries of a CBDS book, or
// fileNames unchanged for any other archive. The names are returned exactly as
// listed by the archive so they can be looked up in fileNames afterwards.
QStringList pageCandidates(const QString &path, const QStringList &fileNames);

// Natural order of the pages of a CBDS book. Page names are plain sequence
// numbers, so there is no spread detection, and the comparison ignores how the
// path is written (separator, case, leading separator) like pageCandidates() does.
void sortPages(QStringList &pages);

}

#endif // CBDS_PAGE_INDEX_H
