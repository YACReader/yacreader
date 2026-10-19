#include "cbl_import_dialog.h"

#include "QsLog.h"
#include "cbl_entries_view.h"
#include "cbl_entry_detail.h"
#include "cbl_find_comic_page.h"
#include "cbl_importer.h"
#include "data_base_management.h"
#include "qnaturalsorting.h"
#include "yacreader_global.h"

#include <QButtonGroup>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStackedWidget>
#include <QThread>
#include <QTreeView>
#include <QVBoxLayout>

#include <algorithm>

struct CblImportFile {
    QString filePath;
    std::optional<CblReadError> readError;
    bool libraryError = false;
    CblReadingList list;
    QList<CblImportEntry> entries;
    QStringList readingListNames;
    std::shared_ptr<const CblMatcher> matcher;
    QList<CblLibraryFolder> folders;
};

namespace {

// the other filter button ids are the statuses; QButtonGroup reads -1 as "choose an id", so it cannot mean all
constexpr int AllEntriesFilter = int(CblImportStatus::Changed) + 1;

std::optional<QStringList> loadTopLevelReadingListNames(QSqlDatabase &db)
{
    QSqlQuery query(db);
    query.setForwardOnly(true);
    if (!query.exec("SELECT name FROM reading_list WHERE parentId IS NULL")) {
        QLOG_ERROR() << "Unable to load the reading list names:" << query.lastError().text();
        return std::nullopt;
    }

    QStringList names;
    while (query.next())
        names.append(query.value(0).toString());
    return names;
}

// every folder but the root, sorted by name
std::optional<QList<CblLibraryFolder>> loadFolders(QSqlDatabase &db)
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

CblImportFile loadCblImportFile(const QString &filePath, const QString &libraryPath)
{
    CblImportFile result;
    result.filePath = filePath;

    auto read = readCbl(filePath);
    if (const auto *error = std::get_if<CblReadError>(&read)) {
        QLOG_WARN() << "Unable to read the reading list" << filePath << error->detail;
        result.readError = *error;
        return result;
    }
    result.list = std::get<CblReadingList>(std::move(read));

    QString connectionName;
    {
        auto db = DataBaseManagement::loadDatabase(YACReader::LibraryPaths::libraryDataPath(libraryPath));
        connectionName = db.connectionName();

        const auto comics = db.isOpen() ? CblMatcher::loadLibraryComics(db) : std::nullopt;
        const auto placeholders = db.isOpen() ? CblMatcher::loadLibraryPlaceholders(db) : std::nullopt;
        const auto names = db.isOpen() ? loadTopLevelReadingListNames(db) : std::nullopt;
        const auto folders = db.isOpen() ? loadFolders(db) : std::nullopt;
        if (!comics || !placeholders || !names || !folders) {
            result.libraryError = true;
        } else {
            result.readingListNames = *names;
            result.folders = *folders;
            result.matcher = std::make_shared<const CblMatcher>(*comics, *placeholders);
            result.entries.reserve(result.list.books.size());
            for (const auto &book : std::as_const(result.list.books)) {
                CblImportEntry entry;
                entry.book = book;
                entry.match = result.matcher->match(book);
                entry.choice = entry.match.accepted;
                result.entries.append(entry);
            }
        }
    }
    if (!connectionName.isEmpty())
        QSqlDatabase::removeDatabase(connectionName);
    return result;
}

// checkable buttons that look like the segments of a filter; colours come from the palette, which the theme drives
QString filterButtonsStyleSheet()
{
    return QStringLiteral(
            "QPushButton {"
            "  border: 1px solid palette(mid);"
            "  border-radius: 4px;"
            "  padding: 4px 14px;"
            "  background-color: palette(button);"
            "  color: palette(button-text);"
            "}"
            "QPushButton:hover { background-color: palette(midlight); }"
            "QPushButton:checked {"
            "  background-color: palette(highlight);"
            "  color: palette(highlighted-text);"
            "  border-color: palette(highlight);"
            "}");
}

}

CblImportDialog::CblImportDialog(const QStringList &filePaths, const QString &libraryPath, QWidget *parent)
    : QDialog(parent), filePaths(filePaths), libraryPath(libraryPath)
{
    model = new CblImportModel(this);
    filterModel = new CblImportFilterModel(this);
    filterModel->setSourceModel(model);

    pages = new QStackedWidget;
    messagePage = createMessagePage();
    entriesPage = createEntriesPage();
    findPage = new CblFindComicPage(libraryPath);
    pages->addWidget(messagePage);
    pages->addWidget(entriesPage);
    pages->addWidget(findPage);
    connect(findPage, &CblFindComicPage::backRequested, this, &CblImportDialog::showEntries);
    connect(findPage, &CblFindComicPage::previousProblemRequested, this, [this] { showProblem(false); });
    connect(findPage, &CblFindComicPage::nextProblemRequested, this, [this] { showProblem(true); });
    connect(findPage, &CblFindComicPage::comicChosen, this, [this](const CblLibraryComic &comic, bool goToNextProblem) {
        model->choose(findRow, comic);
        if (!goToNextProblem || !showProblem(true))
            showEntries();
    });

    summaryLabel = new QLabel;

    closeButton = new QPushButton(tr("Close"));
    skipButton = new QPushButton(tr("Skip"));
    importButton = new QPushButton(tr("Import"));
    // Return in the name or filter field must not import
    for (auto *button : { closeButton, skipButton, importButton })
        button->setAutoDefault(false);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(skipButton, &QPushButton::clicked, this, &CblImportDialog::nextFile);
    connect(importButton, &QPushButton::clicked, this, &CblImportDialog::importCurrentFile);

    bottomBar = new QWidget;
    auto buttons = new QHBoxLayout(bottomBar);
    buttons->setContentsMargins(0, 0, 0, 0);
    buttons->addWidget(summaryLabel);
    buttons->addStretch();
    buttons->addWidget(closeButton);
    buttons->addWidget(skipButton);
    buttons->addWidget(importButton);

    auto layout = new QVBoxLayout(this);
    layout->addWidget(pages, 1);
    layout->addWidget(bottomBar);

    resize(1000, 700);
    loadCurrentFile();
}

CblImportDialog::~CblImportDialog() = default;

QWidget *CblImportDialog::createMessagePage()
{
    auto page = new QWidget;
    auto layout = new QVBoxLayout(page);

    messageLabel = new QLabel;
    messageLabel->setAlignment(Qt::AlignCenter);
    messageLabel->setWordWrap(true);
    layout->addWidget(messageLabel);

    return page;
}

QWidget *CblImportDialog::createEntriesPage()
{
    auto page = new QWidget;
    auto layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);

    nameEdit = new QLineEdit;
    connect(nameEdit, &QLineEdit::textChanged, this, &CblImportDialog::updateNameConflict);
    auto nameLabel = new QLabel(tr("&Name:"));
    nameLabel->setBuddy(nameEdit);
    nameConflictLabel = new QLabel(tr("A reading list with this name already exists. Import creates another one."));

    auto nameRow = new QHBoxLayout;
    nameRow->addWidget(nameLabel);
    nameRow->addWidget(nameEdit, 1);

    filterButtons = new QButtonGroup(this);
    auto filterButtonsWidget = new QWidget;
    auto filterButtonsLayout = new QHBoxLayout(filterButtonsWidget);
    filterButtonsLayout->setContentsMargins(0, 0, 0, 0);
    for (const auto id : { AllEntriesFilter, int(CblImportStatus::Matched), int(CblImportStatus::Ambiguous), int(CblImportStatus::Missing), int(CblImportStatus::Changed) }) {
        auto button = new QPushButton;
        button->setCheckable(true);
        button->setAutoDefault(false);
        filterButtons->addButton(button, id);
        filterButtonList.append(button);
        filterButtonsLayout->addWidget(button);
    }
    filterButtons->button(AllEntriesFilter)->setChecked(true);
    filterButtonsWidget->setStyleSheet(filterButtonsStyleSheet());
    connect(filterButtons, &QButtonGroup::idClicked, this, [this](int id) {
        filterModel->setStatusFilter(id == AllEntriesFilter ? std::nullopt : std::optional(CblImportStatus(id)));
    });

    filterEdit = new QLineEdit;
    filterEdit->setPlaceholderText(tr("Filter entries"));
    filterEdit->setClearButtonEnabled(true);
    connect(filterEdit, &QLineEdit::textChanged, filterModel, &QSortFilterProxyModel::setFilterFixedString);

    auto filterRow = new QHBoxLayout;
    filterRow->addWidget(filterButtonsWidget);
    filterRow->addStretch();
    filterRow->addWidget(filterEdit);

    entriesView = new CblEntriesView;
    entriesView->setModel(filterModel);
    entriesView->setAlternatingRowColors(true);
    entriesView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    entriesView->setSelectionBehavior(QAbstractItemView::SelectRows);
    entriesView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    entriesView->header()->setStretchLastSection(false);
    entriesView->header()->setSectionResizeMode(CblImportModel::NumberColumn, QHeaderView::ResizeToContents);
    entriesView->header()->setSectionResizeMode(CblImportModel::BookColumn, QHeaderView::Stretch);
    entriesView->header()->setSectionResizeMode(CblImportModel::ComicColumn, QHeaderView::Stretch);
    entriesView->header()->setSectionResizeMode(CblImportModel::StatusColumn, QHeaderView::ResizeToContents);
    entriesView->header()->setSectionResizeMode(CblImportModel::ActionColumn, QHeaderView::ResizeToContents);
    connect(entriesView, &QTreeView::expanded, this, &CblImportDialog::createDetail);
    connect(entriesView->header(), &QHeaderView::sectionResized, this, [this] {
        for (auto *detail : entriesView->viewport()->findChildren<CblEntryDetail *>())
            alignDetail(detail);
    });
    connect(entriesView, &CblEntriesView::findComicRequested, this, [this](const QModelIndex &index) {
        showFind(filterModel->mapToSource(index).row());
    });
    entriesView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(entriesView, &QTreeView::customContextMenuRequested, this, &CblImportDialog::showEntriesMenu);
    connect(model, &QAbstractItemModel::dataChanged, this, &CblImportDialog::updateCounts);
    // a detail can change its own entry, so it is replaced after its signal returns
    connect(model, &QAbstractItemModel::dataChanged, this, [this](const QModelIndex &topLeft, const QModelIndex &bottomRight) {
        QList<QPersistentModelIndex> changed;
        for (int row = topLeft.row(); row <= bottomRight.row(); ++row) {
            const auto index = filterModel->mapFromSource(model->index(row, 0));
            if (index.isValid() && entriesView->isExpanded(index))
                changed.append(index);
        }
        if (!changed.isEmpty())
            QMetaObject::invokeMethod(this, [this, changed] {
                for (const auto &index : changed) {
                    if (index.isValid() && entriesView->isExpanded(index))
                        createDetail(index);
                } }, Qt::QueuedConnection);
    });

    layout->addLayout(nameRow);
    layout->addWidget(nameConflictLabel);
    layout->addLayout(filterRow);
    layout->addWidget(entriesView, 1);

    return page;
}

void CblImportDialog::loadCurrentFile()
{
    file.reset();
    findRow = -1;
    updateTitle();
    showMessage(tr("Matching the books of %1 with your library...").arg(QFileInfo(filePaths.at(currentFile)).fileName()));
    skipButton->setEnabled(false);

    auto result = std::make_shared<CblImportFile>();
    auto thread = QThread::create([result, filePath = filePaths.at(currentFile), libraryPath = libraryPath] {
        *result = loadCblImportFile(filePath, libraryPath);
    });
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    connect(thread, &QThread::finished, this, [this, result] { showFile(*result); });
    thread->start();
}

void CblImportDialog::showFile(const CblImportFile &loaded)
{
    skipButton->setEnabled(true);

    if (loaded.readError) {
        switch (loaded.readError->kind) {
        case CblReadError::CannotOpen:
            showMessage(tr("The file cannot be opened."));
            break;
        case CblReadError::Malformed:
            showMessage(tr("The file is damaged (line %1, column %2).").arg(loaded.readError->line).arg(loaded.readError->column));
            break;
        case CblReadError::NotAReadingList:
            showMessage(tr("The file is not a reading list."));
            break;
        }
        return;
    }
    if (loaded.libraryError) {
        showMessage(tr("The library cannot be read."));
        return;
    }

    file = std::make_unique<CblImportFile>(loaded);
    model->setWarningColor(warningColor());
    findPage->setWarningColor(warningColor());
    findPage->setFolders(file->folders);
    model->setEntries(file->entries);
    filterEdit->clear();
    filterButtons->button(AllEntriesFilter)->click();
    nameEdit->setText(cblListName(file->list, file->filePath));
    updateCounts();
    updateNameConflict();

    showEntries();
    importButton->setEnabled(true);
    summaryLabel->setVisible(true);
}

void CblImportDialog::showEntries()
{
    pages->setCurrentWidget(entriesPage);
    bottomBar->setVisible(true);

    if (findRow >= 0 && findRow < model->rowCount()) {
        const auto index = filterModel->mapFromSource(model->index(findRow, 0));
        if (index.isValid()) {
            entriesView->scrollTo(index);
            entriesView->setCurrentIndex(index);
        }
    }
}

void CblImportDialog::showFind(int row)
{
    if (!file || row < 0 || row >= model->rowCount())
        return;

    findRow = row;
    const auto &entry = model->entries().at(row);
    const auto problems = model->problemRows();
    const auto problem = problems.indexOf(row);
    const bool hasPrevious = !problems.isEmpty() && problems.constFirst() < row;
    const bool hasNext = !problems.isEmpty() && problems.constLast() > row;

    // the folder of the comic it refers to, else of its first possible comic, else of the closest series
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
        const auto closest = file->matcher->closestSeries(entry.book);
        for (int i = 0; comicPath.isEmpty() && i < closest.size(); ++i)
            comicPath = closest.at(i).path;
    }

    findPage->showEntry(entry, row, model->rowCount(), model->status(row), comicPath.section('/', 0, -2), problem, problems.size(), hasPrevious, hasNext);
    pages->setCurrentWidget(findPage);
    bottomBar->setVisible(false);
}

bool CblImportDialog::showProblem(bool forward)
{
    const auto problems = model->problemRows();
    if (forward) {
        const auto next = std::find_if(problems.cbegin(), problems.cend(), [this](int row) { return row > findRow; });
        if (next == problems.cend())
            return false;
        showFind(*next);
    } else {
        const auto previous = std::find_if(problems.crbegin(), problems.crend(), [this](int row) { return row < findRow; });
        if (previous == problems.crend())
            return false;
        showFind(*previous);
    }
    return true;
}

void CblImportDialog::alignDetail(CblEntryDetail *detail) const
{
    // a detail is a child row, so its widget starts two indentations in: the root decoration and its own level
    const auto *header = entriesView->header();
    const auto detailLeft = header->sectionViewportPosition(CblImportModel::NumberColumn) + 2 * entriesView->indentation();
    detail->alignLibraryColumns(header->sectionViewportPosition(CblImportModel::ComicColumn) - detailLeft);
}

void CblImportDialog::createDetail(const QModelIndex &index)
{
    if (!file || !index.isValid() || CblImportModel::isDetail(index))
        return;

    const auto row = filterModel->mapToSource(index).row();
    const auto &entry = model->entries().at(row);
    const auto closest = entry.choice || !entry.match.candidates.isEmpty() ? QList<CblLibraryComic>() : file->matcher->closestSeries(entry.book);
    auto detail = new CblEntryDetail(entry, row, model->rowCount(), closest, YACReader::LibraryPaths::libraryDataPath(libraryPath), warningColor());
    connect(detail, &CblEntryDetail::candidateChosen, this, [this, row](const CblCandidate &candidate) { model->choose(row, candidate); });

    alignDetail(detail);
    entriesView->setFirstColumnSpanned(0, index, true);
    entriesView->setIndexWidget(filterModel->index(0, 0, index), detail);
    entriesView->doItemsLayout();
}

void CblImportDialog::showMessage(const QString &message)
{
    messageLabel->setText(message);
    pages->setCurrentWidget(messagePage);
    bottomBar->setVisible(true);
    importButton->setEnabled(false);
    summaryLabel->setVisible(false);
}

void CblImportDialog::importCurrentFile()
{
    if (!file)
        return;

    auto name = nameEdit->text().trimmed();
    if (name.isEmpty())
        name = cblListName(file->list, file->filePath);

    std::optional<ReadingListId> id;
    QString connectionName;
    {
        auto db = DataBaseManagement::loadDatabase(YACReader::LibraryPaths::libraryDataPath(libraryPath));
        connectionName = db.connectionName();
        if (db.isOpen())
            id = importCblReadingList(name, file->list, model->choices(), db);
    }
    if (!connectionName.isEmpty())
        QSqlDatabase::removeDatabase(connectionName);

    if (!id) {
        QMessageBox::critical(this, tr("Import reading lists"), tr("The reading list could not be imported. Nothing was written."));
        return;
    }

    emit readingListImported(*id, name);
    nextFile();
}

void CblImportDialog::nextFile()
{
    if (currentFile + 1 >= filePaths.size()) {
        accept();
        return;
    }

    ++currentFile;
    loadCurrentFile();
}

void CblImportDialog::updateTitle()
{
    const auto fileName = QFileInfo(filePaths.at(currentFile)).fileName();
    if (filePaths.size() > 1)
        setWindowTitle(tr("Import reading lists — %1 (%2 of %3)").arg(fileName).arg(currentFile + 1).arg(filePaths.size()));
    else
        setWindowTitle(tr("Import reading lists — %1").arg(fileName));

    skipButton->setVisible(currentFile + 1 < filePaths.size());
}

void CblImportDialog::updateCounts()
{
    const auto total = model->rowCount();
    const QStringList texts = {
        tr("All (%1)").arg(total),
        tr("Matched (%1)").arg(model->count(CblImportStatus::Matched)),
        tr("Ambiguous (%1)").arg(model->count(CblImportStatus::Ambiguous)),
        tr("Missing (%1)").arg(model->count(CblImportStatus::Missing)),
        tr("Changed by you (%1)").arg(model->count(CblImportStatus::Changed)),
    };
    for (int i = 0; i < filterButtonList.size(); ++i)
        filterButtonList.at(i)->setText(texts.at(i));

    summaryLabel->setText(tr("%1 of %n have a comic", "", total).arg(model->withComicCount()));
}

void CblImportDialog::updateNameConflict()
{
    const bool exists = file && file->readingListNames.contains(nameEdit->text().trimmed(), Qt::CaseInsensitive);
    nameConflictLabel->setStyleSheet(QStringLiteral("color: %1;").arg(warningColor().name()));
    nameConflictLabel->setVisible(exists);
}

void CblImportDialog::showEntriesMenu(const QPoint &position)
{
    const auto rows = selectedRows();
    if (rows.isEmpty())
        return;

    const bool anyChanged = std::any_of(rows.cbegin(), rows.cend(), [this](int row) { return model->status(row) == CblImportStatus::Changed; });
    const bool anyCanBeMissing = std::any_of(rows.cbegin(), rows.cend(), [this](int row) {
        return model->entries().at(row).choice || model->status(row) == CblImportStatus::Ambiguous;
    });

    QMenu menu(this);
    if (rows.size() == 1) {
        auto find = menu.addAction(tr("Find comic..."));
        connect(find, &QAction::triggered, this, [this, row = rows.constFirst()] { showFind(row); });
        menu.addSeparator();
    }
    auto original = menu.addAction(tr("Use the original match"));
    original->setEnabled(anyChanged);
    connect(original, &QAction::triggered, this, [this, rows] { model->useOriginalMatch(rows); });
    auto missing = menu.addAction(tr("Import as missing"));
    missing->setEnabled(anyCanBeMissing);
    connect(missing, &QAction::triggered, this, [this, rows] { model->importAsMissing(rows); });
    menu.exec(entriesView->viewport()->mapToGlobal(position));
}

QList<int> CblImportDialog::selectedRows() const
{
    QList<int> rows;
    for (const auto &index : entriesView->selectionModel()->selectedRows())
        rows.append(filterModel->mapToSource(index).row());
    return rows;
}

QColor CblImportDialog::warningColor() const
{
    const bool dark = palette().color(QPalette::Base).lightness() < 128;
    return dark ? QColor(0xE0, 0xA0, 0x30) : QColor(0xB2, 0x6B, 0x00);
}
