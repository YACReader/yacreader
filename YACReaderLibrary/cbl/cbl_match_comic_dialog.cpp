#include "cbl_match_comic_dialog.h"

#include "cbl_entry_detail.h"
#include "cbl_find_comic_page.h"
#include "cbl_list_match.h"
#include "data_base_management.h"
#include "descriptive_comic_metadata.h"
#include "yacreader_global.h"

#include <QLabel>
#include <QSqlDatabase>
#include <QStackedWidget>
#include <QThread>
#include <QVBoxLayout>

#include <algorithm>
#include <iterator>
#include <memory>

namespace {

struct LoadedLibrary {
    bool error = false;
    QList<CblLibraryFolder> folders;
    QString startFolderPath;
};

LoadedLibrary loadLibrary(const CblBook &book, const QString &libraryPath)
{
    LoadedLibrary result;
    QString connectionName;
    {
        auto db = DataBaseManagement::loadDatabase(YACReader::LibraryPaths::libraryDataPath(libraryPath));
        connectionName = db.connectionName();

        const auto comics = db.isOpen() ? CblMatcher::loadLibraryComics(db) : std::nullopt;
        const auto folders = db.isOpen() ? CblFindComicPage::loadFolders(db) : std::nullopt;
        if (!comics || !folders) {
            result.error = true;
        } else {
            result.folders = *folders;
            // only comics with a file can be chosen
            QList<CblLibraryComic> withFile;
            std::copy_if(comics->cbegin(), comics->cend(), std::back_inserter(withFile), [](const CblLibraryComic &comic) { return comic.hasFile; });
            const CblMatcher matcher(withFile);
            CblImportEntry entry;
            entry.book = book;
            entry.match = matcher.match(book);
            result.startFolderPath = CblFindComicPage::startFolderPath(entry, matcher);
        }
    }
    if (!connectionName.isEmpty())
        QSqlDatabase::removeDatabase(connectionName);
    return result;
}

}

CblMatchComicDialog::CblMatchComicDialog(const DescriptiveComicMetadata &metadata, const QString &libraryPath, std::optional<qulonglong> folderId, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Match to library comic"));
    resize(960, 600);

    messageLabel = new QLabel(tr("Loading the library..."));
    messageLabel->setAlignment(Qt::AlignCenter);
    messageLabel->setWordWrap(true);

    findPage = new CblFindComicPage(libraryPath, CblFindComicPage::Mode::OneEntry);
    findPage->setWarningColor(cblWarningColor(palette()));
    connect(findPage, &CblFindComicPage::backRequested, this, &QDialog::reject);
    connect(findPage, &CblFindComicPage::comicChosen, this, [this](const CblLibraryComic &comic) {
        chosen = comic;
        accept();
    });

    pages = new QStackedWidget;
    pages->addWidget(messageLabel);
    pages->addWidget(findPage);

    auto layout = new QVBoxLayout(this);
    layout->addWidget(pages);

    const auto book = cblBookOf(metadata);
    auto result = std::make_shared<LoadedLibrary>();
    auto thread = QThread::create([result, book, libraryPath] { *result = loadLibrary(book, libraryPath); });
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    connect(thread, &QThread::finished, this, [this, result, book, folderId] {
        if (result->error) {
            messageLabel->setText(tr("The library cannot be read."));
            return;
        }
        CblImportEntry entry;
        entry.book = book;
        findPage->setFolders(result->folders);
        if (folderId)
            findPage->limitToFolder(*folderId);
        findPage->showEntry(entry, 0, 1, CblImportStatus::Missing, result->startFolderPath, -1, 0, false, false);
        pages->setCurrentWidget(findPage);
    });
    thread->start();
}

std::optional<CblLibraryComic> CblMatchComicDialog::chosenComic() const
{
    return chosen;
}
