

#ifndef __QNATURALSORTING_H
#define __QNATURALSORTING_H

#include "library_item.h"

#include <QFileInfo>
#include <QString>
#include <QVariant>

int naturalCompare(const QString &s1, const QString &s2, Qt::CaseSensitivity caseSensitivity);
bool naturalSortLessThanCS(const QString &left, const QString &right);
bool naturalSortLessThanCI(const QString &left, const QString &right);
bool naturalSortLessThanCIFileInfo(const QFileInfo &left, const QFileInfo &right);
bool naturalSortLessThanCILibraryItem(LibraryItem *left, LibraryItem *right);

/* Qt only does numeric ordering and case folding in QCollator when it is built with
 * ICU. A Qt built with -no-icu falls back to the POSIX collator, which drops
 * setNumericMode() and setCaseSensitivity() without reporting anything, so "a10"
 * sorts before "a2" and issue #100 lands between #10 and #11. This is common in
 * minimal Linux builds. Returns true when the collator honours both settings.
 **/
bool naturalCollationIsSupported();

/* The comparison naturalCompare() uses when naturalCollationIsSupported() is false.
 * Digit runs compare by value, numbers sort before text, and the rest goes through
 * the collator so that whatever ordering the platform can still do is kept. Exposed
 * for the tests, which must cover it on machines whose Qt does have ICU.
 **/
int naturalCompareFallback(const QString &s1, const QString &s2, Qt::CaseSensitivity caseSensitivity);

/* The order comics are read in. Issue number wins when both comics have one,
 * numbered comics come before unnumbered ones, and file name breaks the tie
 * otherwise. Every place that lists the comics of a folder must use this, so
 * that what YACReaderLibrary shows and what YACReader walks with next/previous
 * are the same sequence.
 **/
bool comicNumberLessThan(const QVariant &leftNumber, const QString &leftName,
                         const QVariant &rightNumber, const QString &rightName);

/* Name-only ordering for server responses that mix folders and comics in a single
 * list, where there is no issue number to lean on. Reading order is not this: for
 * the comics of a folder use DBHelper::getFolderComicsFromLibraryForReading, which
 * sorts by issue number the way the clients do.
 **/
struct LibraryItemSorter {
    bool operator()(const LibraryItem *a, const LibraryItem *b) const
    {
        return naturalSortLessThanCI(a->name, b->name);
    }
};

#endif
