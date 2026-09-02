#include "reading_list_management_coordinator.h"

#include "add_label_dialog.h"
#include "cbl_reader.h"
#include "comic_model.h"
#include "data_base_management.h"
#include "reading_list_model.h"

#include <QAction>
#include <QFileDialog>
#include <QFileInfo>
#include <QInputDialog>
#include <QKeySequence>
#include <QLineEdit>
#include <QMessageBox>
#include <QRegularExpression>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QWidget>

#include <utility>

namespace {
struct LibraryComicMatchData
{
    qulonglong id = 0;
    QString fileName;
    QString series;
    QString number;
    QString volume;
};

enum class CblMatchState {
    Matched = 0,
    Missing = 1,
    Ambiguous = 2
};

struct CblMatchResult
{
    CblMatchState state = CblMatchState::Missing;
    QList<LibraryComicMatchData> candidates;
};

struct MatchedCblEntry
{
    CblBook book;
    CblMatchResult match;
};

QString normalized(const QString &value)
{
    return value.simplified().toCaseFolded();
}

bool sameValue(const QString &left, const QString &right)
{
    return normalized(left) == normalized(right);
}

CblMatchResult matchBook(const CblBook &book, const QList<LibraryComicMatchData> &libraryComics)
{
    CblMatchResult result;

    QList<LibraryComicMatchData> seriesNumberMatches;
    QList<LibraryComicMatchData> exactMatches;

    for (const auto &comic : libraryComics) {
        if (!sameValue(book.series, comic.series) || !sameValue(book.number, comic.number))
            continue;

        seriesNumberMatches.append(comic);

        if (!book.volume.isEmpty() && sameValue(book.volume, comic.volume))
            exactMatches.append(comic);
    }

    if (!book.volume.isEmpty() && exactMatches.size() == 1) {
        result.state = CblMatchState::Matched;
        result.candidates = exactMatches;
        return result;
    }

    if (!book.volume.isEmpty() && exactMatches.size() > 1) {
        result.state = CblMatchState::Ambiguous;
        result.candidates = exactMatches;
        return result;
    }

    if (seriesNumberMatches.size() == 1) {
        result.state = CblMatchState::Matched;
        result.candidates = seriesNumberMatches;
        return result;
    }

    if (seriesNumberMatches.size() > 1) {
        result.state = CblMatchState::Ambiguous;
        result.candidates = seriesNumberMatches;
        return result;
    }

    // Metadata is preferred, but older libraries can have sparse ComicInfo data.
    // Fall back to the user's filename convention: "Series #Number" with an
    // optional leading zero and optional "(of N)" issue suffix.
    if (!book.series.isEmpty() && !book.number.isEmpty()) {
        const QString issuePattern = QStringLiteral("#0*%1(?:\\s*\\(of\\s+\\d+\\))?(?=\\D|$)")
                                             .arg(QRegularExpression::escape(book.number));
        const QRegularExpression issueExpression(issuePattern, QRegularExpression::CaseInsensitiveOption);
        const QString normalizedSeries = normalized(book.series);

        QList<LibraryComicMatchData> fileMatches;
        for (const auto &comic : libraryComics) {
            if (!normalized(comic.fileName).contains(normalizedSeries))
                continue;
            if (issueExpression.match(comic.fileName).hasMatch())
                fileMatches.append(comic);
        }

        if (fileMatches.size() == 1) {
            result.state = CblMatchState::Matched;
            result.candidates = fileMatches;
            return result;
        }

        if (fileMatches.size() > 1) {
            result.state = CblMatchState::Ambiguous;
            result.candidates = fileMatches;
            return result;
        }
    }

    return result;
}

bool execSql(QSqlQuery &query, QString *error)
{
    if (query.exec())
        return true;
    if (error)
        *error = query.lastError().text();
    return false;
}

bool ensureCblImportTables(QSqlDatabase &db, QString *error)
{
    QSqlQuery meta(db);
    meta.prepare(QStringLiteral(
            "CREATE TABLE IF NOT EXISTS cbl_reading_list_meta ("
            "reading_list_id INTEGER PRIMARY KEY, "
            "source_name TEXT, "
            "imported_at INTEGER NOT NULL, "
            "FOREIGN KEY(reading_list_id) REFERENCES reading_list(id) ON DELETE CASCADE)"));
    if (!execSql(meta, error))
        return false;

    QSqlQuery entries(db);
    entries.prepare(QStringLiteral(
            "CREATE TABLE IF NOT EXISTS cbl_reading_list_entry ("
            "id INTEGER PRIMARY KEY, "
            "reading_list_id INTEGER NOT NULL, "
            "ordering INTEGER NOT NULL, "
            "comic_id INTEGER, "
            "series TEXT, "
            "number TEXT, "
            "volume TEXT, "
            "year TEXT, "
            "source_id TEXT, "
            "match_state INTEGER NOT NULL, "
            "candidate_count INTEGER NOT NULL DEFAULT 0, "
            "FOREIGN KEY(reading_list_id) REFERENCES reading_list(id) ON DELETE CASCADE, "
            "FOREIGN KEY(comic_id) REFERENCES comic(id) ON DELETE SET NULL, "
            "UNIQUE(reading_list_id, ordering))"));
    if (!execSql(entries, error))
        return false;

    QSqlQuery index(db);
    index.prepare(QStringLiteral(
            "CREATE INDEX IF NOT EXISTS cbl_reading_list_entry_ordering_index "
            "ON cbl_reading_list_entry(reading_list_id, ordering)"));
    return execSql(index, error);
}

bool persistCblReadingList(QSqlDatabase &db,
                           const CblReadingList &readingList,
                           const QList<MatchedCblEntry> &entries,
                           const QString &sourceName,
                           qulonglong *readingListId,
                           QString *error)
{
    if (!db.transaction()) {
        if (error)
            *error = db.lastError().text();
        return false;
    }

    auto rollback = [&db, error](const QString &message) {
        db.rollback();
        if (error)
            *error = message;
        return false;
    };

    if (!ensureCblImportTables(db, error)) {
        const auto message = error ? *error : QStringLiteral("Unable to create CBL import tables.");
        return rollback(message);
    }

    QSqlQuery createList(db);
    createList.prepare(QStringLiteral("INSERT INTO reading_list (name) VALUES (:name)"));
    createList.bindValue(QStringLiteral(":name"), readingList.name);
    if (!createList.exec())
        return rollback(createList.lastError().text());

    const qulonglong newReadingListId = createList.lastInsertId().toULongLong();
    if (newReadingListId == 0)
        return rollback(QStringLiteral("YACReader did not return an id for the new reading list."));

    QSqlQuery meta(db);
    meta.prepare(QStringLiteral(
            "INSERT INTO cbl_reading_list_meta (reading_list_id, source_name, imported_at) "
            "VALUES (:reading_list_id, :source_name, :imported_at)"));
    meta.bindValue(QStringLiteral(":reading_list_id"), newReadingListId);
    meta.bindValue(QStringLiteral(":source_name"), sourceName);
    meta.bindValue(QStringLiteral(":imported_at"), QDateTime::currentSecsSinceEpoch());
    if (!meta.exec())
        return rollback(meta.lastError().text());

    QSet<qulonglong> linkedComicIds;

    for (const auto &entry : entries) {
        qulonglong comicId = 0;
        if (entry.match.state == CblMatchState::Matched && !entry.match.candidates.isEmpty())
            comicId = entry.match.candidates.constFirst().id;

        QSqlQuery insertEntry(db);
        insertEntry.prepare(QStringLiteral(
                "INSERT INTO cbl_reading_list_entry "
                "(reading_list_id, ordering, comic_id, series, number, volume, year, source_id, match_state, candidate_count) "
                "VALUES (:reading_list_id, :ordering, :comic_id, :series, :number, :volume, :year, :source_id, :match_state, :candidate_count)"));
        insertEntry.bindValue(QStringLiteral(":reading_list_id"), newReadingListId);
        insertEntry.bindValue(QStringLiteral(":ordering"), entry.book.ordering);
        if (comicId != 0)
            insertEntry.bindValue(QStringLiteral(":comic_id"), comicId);
        else
            insertEntry.bindValue(QStringLiteral(":comic_id"), QVariant());
        insertEntry.bindValue(QStringLiteral(":series"), entry.book.series);
        insertEntry.bindValue(QStringLiteral(":number"), entry.book.number);
        insertEntry.bindValue(QStringLiteral(":volume"), entry.book.volume);
        insertEntry.bindValue(QStringLiteral(":year"), entry.book.year);
        insertEntry.bindValue(QStringLiteral(":source_id"), entry.book.id);
        insertEntry.bindValue(QStringLiteral(":match_state"), static_cast<int>(entry.match.state));
        insertEntry.bindValue(QStringLiteral(":candidate_count"), entry.match.candidates.size());
        if (!insertEntry.exec())
            return rollback(insertEntry.lastError().text());

        // Keep matched entries visible to existing YACReader builds. Imported CBL
        // metadata remains authoritative for ordering and missing placeholders.
        // The legacy relation cannot contain the same comic twice in one list,
        // so a duplicate CBL occurrence is retained in cbl_reading_list_entry but
        // linked only once here.
        if (comicId != 0 && !linkedComicIds.contains(comicId)) {
            QSqlQuery link(db);
            link.prepare(QStringLiteral(
                    "INSERT INTO comic_reading_list (reading_list_id, comic_id, ordering) "
                    "VALUES (:reading_list_id, :comic_id, :ordering)"));
            link.bindValue(QStringLiteral(":reading_list_id"), newReadingListId);
            link.bindValue(QStringLiteral(":comic_id"), comicId);
            link.bindValue(QStringLiteral(":ordering"), entry.book.ordering);
            if (!link.exec())
                return rollback(link.lastError().text());
            linkedComicIds.insert(comicId);
        }
    }

    if (!db.commit())
        return rollback(db.lastError().text());

    if (readingListId)
        *readingListId = newReadingListId;
    return true;
}
}

ReadingListManagementCoordinator::ReadingListManagementCoordinator(QWidget *dialogParent,
                                                                   ReadingListModel *listsModel,
                                                                   ComicModel *comicsModel,
                                                                   CurrentListProvider currentListProvider)
    : QObject(dialogParent), dialogParent(dialogParent), listsModel(listsModel), currentListProvider(std::move(currentListProvider))
{
    connect(listsModel, &ReadingListModel::addComicsToFavorites, comicsModel, QOverload<const QList<qulonglong> &>::of(&ComicModel::addComicsToFavorites));
    connect(listsModel, &ReadingListModel::addComicsToLabel, comicsModel, QOverload<const QList<qulonglong> &, qulonglong>::of(&ComicModel::addComicsToLabel));
    connect(listsModel, &ReadingListModel::addComicsToReadingList, comicsModel, QOverload<const QList<qulonglong> &, qulonglong>::of(&ComicModel::addComicsToReadingList));

    auto *importCblAction = new QAction(tr("Import CBL Reading List..."), dialogParent);
    importCblAction->setShortcut(QKeySequence(QStringLiteral("Ctrl+Shift+I")));
    importCblAction->setShortcutContext(Qt::ApplicationShortcut);
    dialogParent->addAction(importCblAction);
    connect(importCblAction, &QAction::triggered, this, &ReadingListManagementCoordinator::importCblReadingList);
}

void ReadingListManagementCoordinator::addReadingList()
{
    const auto currentList = currentListProvider();
    if (currentList.isValid() && listsModel->isReadingSubList(currentList))
        return;

    bool accepted = false;
    const auto name = QInputDialog::getText(dialogParent,
                                            tr("Add new reading lists"),
                                            tr("List name:"),
                                            QLineEdit::Normal,
                                            { },
                                            &accepted);
    if (!accepted)
        return;

    if (currentList.isValid() && listsModel->isReadingList(currentList))
        listsModel->addReadingListAt(name, currentList);
    else
        listsModel->addReadingList(name);
}

void ReadingListManagementCoordinator::importCblReadingList()
{
    const auto filePath = QFileDialog::getOpenFileName(dialogParent,
                                                       tr("Import CBL reading list"),
                                                       { },
                                                       tr("Comic Book Reading Lists (*.cbl);;XML files (*.xml);;All files (*)"));
    if (filePath.isEmpty())
        return;

    const auto result = CblReader::read(filePath);
    if (!result.success) {
        QString details = result.errorMessage;
        if (result.errorLine > 0)
            details += tr("\n\nLine %1, column %2").arg(result.errorLine).arg(result.errorColumn);

        QMessageBox::critical(dialogParent, tr("Unable to import CBL"), details);
        return;
    }

    if (listsModel->databasePath().isEmpty()) {
        QMessageBox::warning(dialogParent,
                             tr("CBL import unavailable"),
                             tr("Open a library before importing a CBL reading list."));
        return;
    }

    QList<LibraryComicMatchData> libraryComics;
    QString databaseError;
    QString connectionName;
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(listsModel->databasePath());
        connectionName = db.connectionName();

        QSqlQuery query(db);
        if (!query.exec(QStringLiteral("SELECT c.id, c.fileName, ci.series, ci.number, ci.volume "
                                       "FROM comic c "
                                       "INNER JOIN comic_info ci ON c.comicInfoId = ci.id"))) {
            databaseError = query.lastError().text();
        } else {
            while (query.next()) {
                LibraryComicMatchData comic;
                comic.id = query.value(0).toULongLong();
                comic.fileName = query.value(1).toString();
                comic.series = query.value(2).toString();
                comic.number = query.value(3).toString();
                comic.volume = query.value(4).toString();
                libraryComics.append(comic);
            }
        }
    }
    QSqlDatabase::removeDatabase(connectionName);

    if (!databaseError.isEmpty()) {
        QMessageBox::critical(dialogParent,
                              tr("Unable to inspect library"),
                              tr("YACReader could not read comic metadata for CBL matching.\n\n%1").arg(databaseError));
        return;
    }

    QString preview;
    const int previewLimit = 30;
    const int count = result.readingList.books.size();
    int matchedCount = 0;
    int missingCount = 0;
    int ambiguousCount = 0;
    QList<MatchedCblEntry> matchedEntries;
    matchedEntries.reserve(count);

    for (int i = 0; i < count; ++i) {
        const auto &book = result.readingList.books.at(i);
        const auto match = matchBook(book, libraryComics);
        matchedEntries.append({ book, match });

        if (match.state == CblMatchState::Matched)
            ++matchedCount;
        else if (match.state == CblMatchState::Ambiguous)
            ++ambiguousCount;
        else
            ++missingCount;

        if (i >= previewLimit)
            continue;

        const QString bookName = QStringLiteral("%1 #%2")
                                         .arg(book.series.isEmpty() ? tr("Unknown series") : book.series)
                                         .arg(book.number.isEmpty() ? QStringLiteral("?") : book.number);

        if (match.state == CblMatchState::Matched) {
            preview += tr("%1. [MATCHED] %2  ->  %3\n")
                               .arg(i + 1)
                               .arg(bookName)
                               .arg(match.candidates.constFirst().fileName);
        } else if (match.state == CblMatchState::Ambiguous) {
            preview += tr("%1. [AMBIGUOUS] %2  ->  %3 possible matches\n")
                               .arg(i + 1)
                               .arg(bookName)
                               .arg(match.candidates.size());
        } else {
            preview += tr("%1. [MISSING] %2\n").arg(i + 1).arg(bookName);
        }
    }

    if (count > previewLimit)
        preview += tr("\n...and %1 more entries.").arg(count - previewLimit);

    const auto answer = QMessageBox::question(
            dialogParent,
            tr("Import CBL reading list"),
            tr("%1\n\n%2 entries\n%3 matched\n%4 missing\n%5 ambiguous\n\n%6\n\n"
               "Import this reading list? Matched comics will be added immediately. "
               "Missing and ambiguous entries will be preserved for placeholder/manual matching support.")
                    .arg(result.readingList.name)
                    .arg(count)
                    .arg(matchedCount)
                    .arg(missingCount)
                    .arg(ambiguousCount)
                    .arg(preview),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No);

    if (answer != QMessageBox::Yes)
        return;

    QString importError;
    qulonglong readingListId = 0;
    connectionName.clear();
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(listsModel->databasePath());
        connectionName = db.connectionName();
        if (!db.isOpen()) {
            importError = tr("Unable to open the library database for writing.");
        } else {
            persistCblReadingList(db,
                                  result.readingList,
                                  matchedEntries,
                                  QFileInfo(filePath).fileName(),
                                  &readingListId,
                                  &importError);
        }
    }
    if (!connectionName.isEmpty())
        QSqlDatabase::removeDatabase(connectionName);

    if (!importError.isEmpty() || readingListId == 0) {
        QMessageBox::critical(dialogParent,
                              tr("CBL import failed"),
                              tr("No partial import was kept. The database transaction was rolled back.\n\n%1")
                                      .arg(importError.isEmpty() ? tr("Unknown database error.") : importError));
        return;
    }

    listsModel->setupReadingListsData(listsModel->databasePath());

    QMessageBox::information(dialogParent,
                             tr("CBL import complete"),
                             tr("%1 was imported.\n\n%2 matched comics are available in the reading list now. "
                                "%3 unresolved entries were preserved in CBL import storage for the next placeholder/manual-match step.")
                                     .arg(result.readingList.name)
                                     .arg(matchedCount)
                                     .arg(missingCount + ambiguousCount));
}

void ReadingListManagementCoordinator::deleteCurrentList()
{
    const auto currentList = currentListProvider();
    if (!currentList.isValid() || !listsModel->isEditable(currentList))
        return;

    const auto answer = QMessageBox::question(dialogParent,
                                              tr("Delete list/label"),
                                              tr("The selected item will be deleted, your comics or folders will NOT be deleted from your disk. Are you sure?"),
                                              QMessageBox::Yes,
                                              QMessageBox::No);
    if (answer != QMessageBox::Yes)
        return;

    listsModel->deleteItem(currentList);
    emit currentListReselectionRequested();
}

void ReadingListManagementCoordinator::addLabel()
{
    AddLabelDialog dialog(dialogParent);
    if (dialog.exec() == QDialog::Accepted)
        listsModel->addNewLabel(dialog.name(), dialog.selectedColor());
}

void ReadingListManagementCoordinator::renameCurrentList()
{
    const auto currentList = currentListProvider();
    if (!currentList.isValid() || !listsModel->isEditable(currentList))
        return;

    bool accepted = false;
    const auto name = QInputDialog::getText(dialogParent,
                                            tr("Rename list name"),
                                            tr("List name:"),
                                            QLineEdit::Normal,
                                            listsModel->name(currentList),
                                            &accepted);
    if (accepted)
        listsModel->rename(currentList, name);
}
