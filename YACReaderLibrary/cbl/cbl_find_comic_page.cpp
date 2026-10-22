#include "cbl_find_comic_page.h"

#include "QsLog.h"
#include "cbl_entry_detail.h"
#include "data_base_management.h"
#include "db_helper.h"
#include "qnaturalsorting.h"
#include "search_query.h"
#include "yacreader_global.h"

#include <QCheckBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSplitter>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QStandardItemModel>
#include <QTimer>
#include <QTreeView>
#include <QVBoxLayout>

#include <algorithm>

namespace {

enum ComicColumn {
    FileColumn,
    SeriesColumn,
    NumberColumn,
    VolumeColumn,
    YearColumn,
};

constexpr int FolderIdRole = Qt::UserRole + 1;
constexpr int ComicPositionRole = Qt::UserRole + 2;

CblLibraryComic libraryComic(const ComicDB &comic)
{
    CblLibraryComic result;
    result.id = comic.info.id;
    result.hasFile = true;
    result.path = comic.file.path;
    result.hash = comic.info.hash;
    result.series = comic.info.series.toString();
    result.number = comic.info.number.toString();
    result.volume = comic.info.volume.toString();
    result.date = comic.info.date.toString();
    result.format = comic.info.format.toString();
    result.comicVineId = comic.info.comicVineID.toString();
    return result;
}

bool different(const QString &a, const QString &b)
{
    return !a.trimmed().isEmpty() && !b.trimmed().isEmpty() && a.simplified().compare(b.simplified(), Qt::CaseInsensitive) != 0;
}

}

CblFindComicPage::CblFindComicPage(const QString &libraryPath, Mode mode, QWidget *parent)
    : QWidget(parent), libraryPath(libraryPath), mode(mode)
{
    auto backButton = new QPushButton(tr("‹ All entries"));
    connect(backButton, &QPushButton::clicked, this, &CblFindComicPage::backRequested);

    entryLabel = new QLabel;
    entryLabel->setTextFormat(Qt::RichText);
    problemLabel = new QLabel;

    auto entryTexts = new QVBoxLayout;
    entryTexts->setSpacing(0);
    entryTexts->addWidget(entryLabel);
    entryTexts->addWidget(problemLabel);

    previousButton = new QPushButton(QStringLiteral("‹"));
    nextButton = new QPushButton(QStringLiteral("›"));
    for (auto *button : { previousButton, nextButton })
        button->setFixedWidth(button->sizeHint().height() * 3 / 2);
    previousButton->setToolTip(tr("Previous problem"));
    nextButton->setToolTip(tr("Next problem"));
    problemCountLabel = new QLabel;
    connect(previousButton, &QPushButton::clicked, this, &CblFindComicPage::previousProblemRequested);
    connect(nextButton, &QPushButton::clicked, this, &CblFindComicPage::nextProblemRequested);

    auto header = new QHBoxLayout;
    header->addWidget(backButton);
    header->addSpacing(12);
    header->addLayout(entryTexts, 1);
    header->addWidget(previousButton);
    header->addWidget(problemCountLabel);
    header->addWidget(nextButton);

    foldersModel = new QStandardItemModel(this);
    foldersModel->setHorizontalHeaderLabels({ tr("Folders") });
    foldersView = new QTreeView;
    foldersView->setModel(foldersModel);
    foldersView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    connect(foldersView->selectionModel(), &QItemSelectionModel::currentChanged, this, [this](const QModelIndex &current) {
        if (current.isValid()) {
            searchEdit->clear();
            showFolder(current.data(FolderIdRole).toULongLong());
        }
    });

    breadcrumbLabel = new QLabel;
    breadcrumbLabel->setTextFormat(Qt::RichText);
    searchEdit = new QLineEdit;
    searchEdit->setPlaceholderText(tr("Search the whole library"));
    searchEdit->setClearButtonEnabled(true);
    searchEdit->setMinimumWidth(260);
    searchTimer = new QTimer(this);
    searchTimer->setSingleShot(true);
    searchTimer->setInterval(300);
    connect(searchTimer, &QTimer::timeout, this, &CblFindComicPage::search);
    connect(searchEdit, &QLineEdit::textEdited, searchTimer, qOverload<>(&QTimer::start));
    connect(searchEdit, &QLineEdit::textChanged, this, [this](const QString &text) {
        if (!text.isEmpty())
            return;
        if (onlyFolderId)
            showFolder(*onlyFolderId);
        else if (foldersView->currentIndex().isValid())
            showFolder(foldersView->currentIndex().data(FolderIdRole).toULongLong());
    });

    auto comicsHeader = new QHBoxLayout;
    comicsHeader->addWidget(breadcrumbLabel, 1);
    comicsHeader->addWidget(searchEdit);

    comicsModel = new QStandardItemModel(this);
    comicsModel->setHorizontalHeaderLabels({ tr("File"), tr("Series"), tr("No."), tr("Volume"), tr("Year") });
    comicsView = new QTreeView;
    comicsView->setModel(comicsModel);
    comicsView->setRootIsDecorated(false);
    comicsView->setUniformRowHeights(true);
    comicsView->setAlternatingRowColors(true);
    comicsView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    comicsView->setSelectionBehavior(QAbstractItemView::SelectRows);
    comicsView->header()->setSectionResizeMode(FileColumn, QHeaderView::Stretch);
    comicsView->header()->setSectionResizeMode(SeriesColumn, QHeaderView::Stretch);
    comicsView->header()->setStretchLastSection(false);
    connect(comicsView, &QTreeView::doubleClicked, this, &CblFindComicPage::chooseSelectedComic);
    connect(comicsView->selectionModel(), &QItemSelectionModel::selectionChanged, this, [this] {
        useButton->setEnabled(comicsView->selectionModel()->hasSelection());
    });

    auto comicsPane = new QWidget;
    auto comicsLayout = new QVBoxLayout(comicsPane);
    comicsLayout->setContentsMargins(0, 0, 0, 0);
    comicsLayout->addLayout(comicsHeader);
    comicsLayout->addWidget(comicsView, 1);

    auto splitter = new QSplitter;
    splitter->addWidget(foldersView);
    splitter->addWidget(comicsPane);
    splitter->setStretchFactor(1, 1);
    splitter->setSizes({ 240, 700 });

    nextProblemCheck = new QCheckBox(tr("Go to the next problem after I choose"));
    nextProblemCheck->setChecked(true);
    useButton = new QPushButton(tr("Use this comic"));
    useButton->setAutoDefault(false);
    useButton->setEnabled(false);
    connect(useButton, &QPushButton::clicked, this, &CblFindComicPage::chooseSelectedComic);

    for (auto *button : { backButton, previousButton, nextButton })
        button->setAutoDefault(false);

    auto bottom = new QHBoxLayout;
    bottom->addWidget(nextProblemCheck);
    bottom->addStretch();
    if (mode == Mode::OneEntry) {
        for (QWidget *widget : std::initializer_list<QWidget *> { backButton, previousButton, problemCountLabel, nextButton, nextProblemCheck })
            widget->hide();
        auto cancelButton = new QPushButton(tr("Cancel"));
        cancelButton->setAutoDefault(false);
        connect(cancelButton, &QPushButton::clicked, this, &CblFindComicPage::backRequested);
        bottom->addWidget(cancelButton);
    }
    bottom->addWidget(useButton);

    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addLayout(header);
    layout->addWidget(splitter, 1);
    layout->addLayout(bottom);
}

std::optional<QList<CblLibraryFolder>> CblFindComicPage::loadFolders(QSqlDatabase &db)
{
    QSqlQuery query(db);
    query.setForwardOnly(true);
    if (!query.exec("SELECT id, parentId, name, path FROM folder WHERE id <> 1")) {
        QLOG_ERROR() << "Unable to load the folders:" << query.lastError().text();
        return std::nullopt;
    }

    QList<CblLibraryFolder> folders;
    while (query.next())
        folders.append({ query.value(0).toULongLong(), query.value(1).toULongLong(), query.value(2).toString(), query.value(3).toString() });
    std::sort(folders.begin(), folders.end(), [](const CblLibraryFolder &a, const CblLibraryFolder &b) { return naturalSortLessThanCI(a.name, b.name); });
    return folders;
}

QString CblFindComicPage::startFolderPath(const CblImportEntry &entry, const CblMatcher &matcher)
{
    QString comicPath;
    const auto pathOf = [](const CblCandidate &candidate) {
        const auto *comic = std::get_if<CblLibraryComic>(&candidate);
        return comic != nullptr ? comic->path : QString();
    };
    if (entry.choice)
        comicPath = pathOf(*entry.choice);
    for (int i = 0; comicPath.isEmpty() && i < entry.match.candidates.size(); ++i)
        comicPath = pathOf(entry.match.candidates.at(i));
    if (comicPath.isEmpty()) {
        const auto closest = matcher.closestSeries(entry.book);
        for (int i = 0; comicPath.isEmpty() && i < closest.size(); ++i)
            comicPath = closest.at(i).path;
    }
    return comicPath.section('/', 0, -2);
}

void CblFindComicPage::setFolders(const QList<CblLibraryFolder> &folders)
{
    foldersModel->removeRows(0, foldersModel->rowCount());
    folderItems.clear();
    folderIdsByPath.clear();

    for (const auto &folder : folders) {
        auto item = new QStandardItem(folder.name);
        item->setData(folder.id, FolderIdRole);
        folderItems.insert(folder.id, item);
        folderIdsByPath.insert(folder.path, folder.id);
    }
    // the folders come sorted by name, so the children keep that order
    for (const auto &folder : folders) {
        auto *item = folderItems.value(folder.id);
        if (auto *parentItem = folderItems.value(folder.parentId))
            parentItem->appendRow(item);
        else
            foldersModel->appendRow(item);
    }
}

void CblFindComicPage::limitToFolder(qulonglong folderId)
{
    onlyFolderId = folderId;
    foldersView->hide();
    problemLabel->hide();
    searchEdit->setPlaceholderText(tr("Search in this folder"));
}

void CblFindComicPage::setWarningColor(const QColor &color)
{
    warningColor = color;
    problemLabel->setStyleSheet(QStringLiteral("color: %1;").arg(color.name()));
}

void CblFindComicPage::showEntry(const CblImportEntry &shownEntry, int row, int entryCount, CblImportStatus status, const QString &folderPath, int problem, int problemCount, bool hasPrevious, bool hasNext)
{
    entry = shownEntry;

    const auto details = cblBookDetails(entry.book);
    auto text = QStringLiteral("<b>") + CblImportModel::bookTitle(entry.book).toHtmlEscaped() + QStringLiteral("</b>");
    if (mode == Mode::ImportEntries)
        text = tr("Entry %1 of %2").arg(row + 1).arg(entryCount) + QStringLiteral(" · ") + text;
    if (!details.isEmpty())
        text += QStringLiteral(" (") + details.toHtmlEscaped() + QStringLiteral(")");
    entryLabel->setText(text);

    if (mode == Mode::OneEntry)
        status = CblImportStatus::Missing;
    switch (status) {
    case CblImportStatus::Missing:
        problemLabel->setText(tr("Missing: no comic found. Browse or search the library."));
        break;
    case CblImportStatus::Ambiguous:
        problemLabel->setText(tr("Ambiguous: %n possible comic(s). Browse or search the library.", "", entry.match.candidates.size()));
        break;
    case CblImportStatus::Matched:
    case CblImportStatus::Changed:
        problemLabel->setText(tr("Browse or search the library to choose another comic."));
        break;
    }

    problemCountLabel->setText(problem >= 0 ? tr("Problem %1 of %2").arg(problem + 1).arg(problemCount) : tr("%n problem(s)", "", problemCount));
    previousButton->setEnabled(hasPrevious);
    nextButton->setEnabled(hasNext);

    searchEdit->clear();
    if (onlyFolderId) {
        showFolder(*onlyFolderId);
        return;
    }
    const auto folderId = folderIdsByPath.value(folderPath);
    if (auto *item = folderItems.value(folderId)) {
        const auto index = item->index();
        foldersView->scrollTo(index);
        if (foldersView->currentIndex() == index)
            showFolder(folderId);
        else
            foldersView->setCurrentIndex(index);
    } else {
        foldersView->setCurrentIndex(QModelIndex());
        breadcrumbLabel->setText(tr("Library"));
        showComics({ });
    }
}

void CblFindComicPage::showFolder(qulonglong folderId)
{
    breadcrumbLabel->setText(breadcrumb(folderItems.value(folderId)));

    QList<ComicDB> folderComics;
    QString connectionName;
    {
        auto db = DataBaseManagement::loadDatabase(YACReader::LibraryPaths::libraryDataPath(libraryPath));
        connectionName = db.connectionName();
        if (db.isOpen())
            folderComics = DBHelper::getComicsFromParent(folderId, db);
    }
    if (!connectionName.isEmpty())
        QSqlDatabase::removeDatabase(connectionName);
    showComics(folderComics);
}

void CblFindComicPage::search()
{
    const auto text = searchEdit->text().trimmed();
    if (text.isEmpty())
        return;

    QList<ComicDB> found;
    bool parsed = true;
    QString connectionName;
    {
        auto db = DataBaseManagement::loadDatabase(YACReader::LibraryPaths::libraryDataPath(libraryPath));
        connectionName = db.connectionName();
        if (db.isOpen()) {
            try {
                auto query = comicsSearchQuery(db, text);
                const auto indexes = ComicDB::columnIndexes(query.record(), "c", "ci");
                while (query.next()) {
                    auto comic = ComicDB::fromQuery(query, indexes);
                    if (!onlyFolderId || comic.file.folderId == FolderId(*onlyFolderId))
                        found.append(std::move(comic));
                }
            } catch (const std::exception &) {
                parsed = false;
            }
        }
    }
    if (!connectionName.isEmpty())
        QSqlDatabase::removeDatabase(connectionName);
    // an incomplete search text cannot be parsed; the previous results stay
    if (!parsed)
        return;

    breadcrumbLabel->setText(tr("Search results"));
    showComics(found);
}

void CblFindComicPage::showComics(const QList<ComicDB> &shownComics)
{
    comics = shownComics;
    comicsModel->removeRows(0, comicsModel->rowCount());
    useButton->setEnabled(false);

    const auto bookYear = cblBookYear(entry.book);
    for (int i = 0; i < comics.size(); ++i) {
        const auto comic = libraryComic(comics.at(i));
        const auto year = cblYearOfDate(comic.date);
        const QList<std::pair<QString, bool>> values = {
            { comics.at(i).file.fileName, false },
            { comic.series, different(entry.book.series, comic.series) },
            { comic.number, !entry.book.number.trimmed().isEmpty() && !comic.number.trimmed().isEmpty() && !cblSameNumber(entry.book.number, comic.number) },
            { comic.volume, different(entry.book.volume, cblEffectiveVolume(comic)) },
            { year > 0 ? QString::number(year) : QString(), bookYear > 0 && year > 0 && bookYear != year },
        };

        QList<QStandardItem *> items;
        for (const auto &[value, isDifferent] : values) {
            auto item = new QStandardItem(value);
            item->setData(i, ComicPositionRole);
            if (isDifferent && warningColor.isValid())
                item->setForeground(warningColor);
            items.append(item);
        }
        comicsModel->appendRow(items);
    }
    comicsView->header()->resizeSections(QHeaderView::ResizeToContents);
    comicsView->header()->setSectionResizeMode(FileColumn, QHeaderView::Stretch);
    comicsView->header()->setSectionResizeMode(SeriesColumn, QHeaderView::Stretch);
}

void CblFindComicPage::chooseSelectedComic()
{
    const auto selected = comicsView->selectionModel()->selectedRows();
    if (selected.isEmpty())
        return;
    const auto position = selected.constFirst().data(ComicPositionRole).toInt();
    if (position < 0 || position >= comics.size())
        return;
    emit comicChosen(libraryComic(comics.at(position)), nextProblemCheck->isChecked());
}

QString CblFindComicPage::breadcrumb(QStandardItem *folder) const
{
    if (folder == nullptr)
        return tr("Library");

    QStringList parts { QStringLiteral("<b>") + folder->text().toHtmlEscaped() + QStringLiteral("</b>") };
    for (auto *item = folder->parent(); item != nullptr; item = item->parent())
        parts.prepend(item->text().toHtmlEscaped());
    parts.prepend(tr("Library").toHtmlEscaped());
    return parts.join(QStringLiteral(" / "));
}
