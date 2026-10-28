#ifndef CBL_ENTRY_DETAIL_H
#define CBL_ENTRY_DETAIL_H

#include "cbl_import_model.h"

#include <QWidget>

class QGridLayout;

// the book of an entry next to what it refers to: its choice, its possible comics, or the closest series
class CblEntryDetail : public QWidget
{
    Q_OBJECT

public:
    CblEntryDetail(const CblImportEntry &entry, int row, int entryCount, const QList<CblLibraryComic> &closestSeries, const QString &libraryDataPath, const QColor &warningColor, QWidget *parent = nullptr);

    // the first library column starts at `x`, so it can sit under a column of the table
    void alignLibraryColumns(int x);

signals:
    void candidateChosen(const CblCandidate &candidate);

private:
    QGridLayout *grid;
    int labelsWidth = 0;
};

// what the UI says about matching
QString cblMatchLevelText(CblMatchLevel level);
QString cblBookDetails(const CblBook &book);
// amber, for problems and values that are different
QColor cblWarningColor(const QPalette &palette);

#endif
