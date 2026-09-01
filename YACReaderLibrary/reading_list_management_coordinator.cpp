#include "reading_list_management_coordinator.h"

#include "add_label_dialog.h"
#include "cbl_reader.h"
#include "comic_model.h"
#include "data_base_management.h"
#include "reading_list_model.h"

#include <QAction>
#include <QFileDialog>
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
    Matched,
    Missing,
    Ambiguous
};

struct CblMatchResult
{
    CblMatchState state = CblMatchState::Missing;
    QList<LibraryComicMatchData> candidates;
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

    // Phase 1: expose the importer immediately without touching the database.
    // The permanent toolbar action will replace this temporary shortcut once the
    // import preview and matching flow are settled.
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

    for (int i = 0; i < count; ++i) {
        const auto &book = result.readingList.books.at(i);
        const auto match = matchBook(book, libraryComics);

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

    QMessageBox::information(dialogParent,
                             tr("CBL import preview"),
                             tr("%1\n\n%2 entries\n%3 matched\n%4 missing\n%5 ambiguous\n\n%6\n\nPreview only. No changes have been made to your library yet.")
                                     .arg(result.readingList.name)
                                     .arg(count)
                                     .arg(matchedCount)
                                     .arg(missingCount)
                                     .arg(ambiguousCount)
                                     .arg(preview));
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
