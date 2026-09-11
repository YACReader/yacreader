#include "reading_list_management_coordinator.h"

#include "add_label_dialog.h"
#include "cbl_reader.h"
#include "comic_model.h"
#include "data_base_management.h"
#include "db_helper.h"
#include "reading_list_model.h"

#include <QAction>
#include <QApplication>
#include <QComboBox>
#include <QCryptographicHash>
#include <QDateTime>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QKeySequence>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPageLayout>
#include <QPageSize>
#include <QPdfWriter>
#include <QRegularExpression>
#include <QPushButton>
#include <QProgressDialog>
#include <QSaveFile>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSpinBox>
#include <QTableWidget>
#include <QTextDocument>
#include <QTreeWidget>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>
#include <QXmlStreamWriter>

#include <algorithm>
#include <utility>

namespace {
struct LibraryComicMatchData
{
    qulonglong id = 0;
    QString fileName;
    QString series;
    QString number;
    QString volume;
    QString year;
    QString format;
    QString alternateSeries;
    QString comicVineIssueId;
};

enum class CblMatchTier {
    None = 0,
    SavedRemap,
    ComicVineIssue,
    ExactMetadata,
    NormalizedMetadata,
    AlternateSeries,
    FileName,
    Manual
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
    CblMatchTier tier = CblMatchTier::None;
    bool saveRemap = false;
};

struct MatchedCblEntry
{
    CblBook book;
    CblMatchResult match;
};

struct MissingComicEntry
{
    QString series;
    QString number;
    QString volume;
    QString year;
    QString format;
    QString fileName;
    QString sourceId;
    QString comicVineSeriesId;
    QString comicVineIssueId;
};

QString safeFileName(QString name)
{
    name.replace(QRegularExpression(QStringLiteral("[<>:\"/\\|?*]")), QStringLiteral("_"));
    return name;
}

bool writeCblFile(const QString &filePath, const QString &name, const QList<CblBook> &books, QString *error)
{
    QSaveFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        if (error)
            *error = file.errorString();
        return false;
    }
    QXmlStreamWriter xml(&file);
    xml.setAutoFormatting(true);
    xml.writeStartDocument();
    xml.writeStartElement(QStringLiteral("ReadingList"));
    xml.writeNamespace(QStringLiteral("http://www.w3.org/2001/XMLSchema"), QStringLiteral("xsd"));
    xml.writeNamespace(QStringLiteral("http://www.w3.org/2001/XMLSchema-instance"), QStringLiteral("xsi"));
    xml.writeTextElement(QStringLiteral("Name"), name);
    xml.writeTextElement(QStringLiteral("NumIssues"), QString::number(books.size()));
    xml.writeStartElement(QStringLiteral("Books"));
    for (const auto &book : books) {
        xml.writeStartElement(QStringLiteral("Book"));
        const auto attribute = [&xml](const QString &key, const QString &value) {
            if (!value.isEmpty())
                xml.writeAttribute(key, value);
        };
        attribute(QStringLiteral("Series"), book.series);
        attribute(QStringLiteral("Number"), book.number);
        attribute(QStringLiteral("Volume"), book.volume);
        attribute(QStringLiteral("Year"), book.year);
        attribute(QStringLiteral("Format"), book.format);
        if (!book.id.isEmpty())
            xml.writeTextElement(QStringLiteral("Id"), book.id);
        if (!book.fileName.isEmpty())
            xml.writeTextElement(QStringLiteral("FileName"), book.fileName);
        if (!book.comicVineSeriesId.isEmpty() || !book.comicVineIssueId.isEmpty()) {
            xml.writeStartElement(QStringLiteral("Database"));
            xml.writeAttribute(QStringLiteral("Name"), QStringLiteral("cv"));
            attribute(QStringLiteral("Series"), book.comicVineSeriesId);
            attribute(QStringLiteral("Issue"), book.comicVineIssueId);
            xml.writeEndElement();
        }
        xml.writeEndElement();
    }
    xml.writeEndElement();
    xml.writeEndElement();
    xml.writeEndDocument();
    if (xml.hasError() || !file.commit()) {
        if (error)
            *error = file.errorString().isEmpty() ? QObject::tr("The CBL file could not be written.") : file.errorString();
        return false;
    }
    return true;
}

QString normalized(const QString &value)
{
    return value.simplified().toCaseFolded();
}

QString normalizedIssueNumber(const QString &value)
{
    QString result = normalized(value);
    static const QRegularExpression simpleNumber(QStringLiteral("^0+(\\d+(?:\\.\\d+)?)$"));
    const auto match = simpleNumber.match(result);
    if (match.hasMatch())
        result = match.captured(1);
    return result;
}

QString normalizedSeries(const QString &value, bool relaxed = false)
{
    QString result = normalized(value);
    result.remove(QRegularExpression(QStringLiteral("\\s*\\((?:(?:vol(?:ume)?\\.?\\s*)?\\d{1,4})\\)\\s*$"),
                                     QRegularExpression::CaseInsensitiveOption));
    if (!relaxed)
        return result;

    static const QRegularExpression article(QStringLiteral("^(?:the|a|an|le|la|les|el|los|las|der|die|das|l['’])\\s+"),
                                            QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression edition(
            QStringLiteral("\\s+(?:director['’]s cut|deluxe(?: edition)?|omnibus|trade paperback|tpb|hardcover|hc|complete(?: collection)?|absolute|new edition|revised edition|anniversary edition|collected edition|compendium|gallery edition|artist['’]s edition)$"),
            QRegularExpression::CaseInsensitiveOption);
    result.remove(article);
    result.remove(edition);
    result.remove(QRegularExpression(QStringLiteral("[^\\p{L}\\p{N}]+")));
    return result;
}

bool sameValue(const QString &left, const QString &right)
{
    return normalized(left) == normalized(right);
}

bool matchesRequiredDates(const CblBook &book, const LibraryComicMatchData &comic)
{
    // CBL Volume commonly contains the series start year (for example, 2016).
    // Treat supplied dates as constraints: missing or different target metadata
    // is not safe enough for an automatic match, but remains available manually.
    if (!book.volume.isEmpty()
        && (comic.volume.isEmpty() || !sameValue(book.volume, comic.volume)))
        return false;

    if (!book.year.isEmpty()
        && (comic.year.isEmpty() || !sameValue(book.year, comic.year)))
        return false;

    return true;
}

QString remapKey(const CblBook &book)
{
    return normalizedSeries(book.series, true) + QLatin1Char('|')
            + normalized(book.volume) + QLatin1Char('|') + normalizedIssueNumber(book.number);
}

QString normalizedComicVineIssueId(const QString &value)
{
    const auto trimmed = value.trimmed();
    if (trimmed.isEmpty())
        return { };

    static const QRegularExpression issueIdExpression(
            QStringLiteral("(?:^|/issue/|4000-)(\\d+)/?(?:[?#].*)?$"),
            QRegularExpression::CaseInsensitiveOption);
    const auto match = issueIdExpression.match(trimmed);
    return match.hasMatch() ? match.captured(1) : QString();
}

CblMatchResult resolvedMatch(const QList<LibraryComicMatchData> &matches, CblMatchTier tier)
{
    CblMatchResult result;
    result.candidates = matches;
    result.tier = tier;
    result.state = matches.size() == 1 ? CblMatchState::Matched
                                      : (matches.isEmpty() ? CblMatchState::Missing : CblMatchState::Ambiguous);
    return result;
}

CblMatchResult matchBook(const CblBook &book,
                         const QList<LibraryComicMatchData> &libraryComics,
                         const QHash<QString, qulonglong> &remaps,
                         QProgressDialog *progress = nullptr)
{
    int inspectedComics = 0;
    const auto continueMatching = [&] {
        if (++inspectedComics % 100 == 0)
            QApplication::processEvents();
        return progress == nullptr || !progress->wasCanceled();
    };

    const auto remappedId = remaps.value(remapKey(book));
    if (remappedId != 0) {
        for (const auto &comic : libraryComics) {
            if (!continueMatching())
                return { };
            if (comic.id == remappedId)
                return resolvedMatch({ comic }, CblMatchTier::SavedRemap);
        }
    }

    const auto cblComicVineIssueId = normalizedComicVineIssueId(book.comicVineIssueId);
    if (!cblComicVineIssueId.isEmpty()) {
        QList<LibraryComicMatchData> comicVineMatches;
        for (const auto &comic : libraryComics) {
            if (!continueMatching())
                return { };
            if (normalizedComicVineIssueId(comic.comicVineIssueId) == cblComicVineIssueId)
                comicVineMatches.append(comic);
        }

        if (comicVineMatches.size() == 1) {
            return resolvedMatch(comicVineMatches, CblMatchTier::ComicVineIssue);
        }

        if (comicVineMatches.size() > 1) {
            return resolvedMatch(comicVineMatches, CblMatchTier::ComicVineIssue);
        }
    }

    QList<LibraryComicMatchData> seriesNumberMatches;
    QList<LibraryComicMatchData> exactMatches;

    for (const auto &comic : libraryComics) {
        if (!continueMatching())
            return { };
        if (!sameValue(book.series, comic.series)
            || normalizedIssueNumber(book.number) != normalizedIssueNumber(comic.number)
            || !matchesRequiredDates(book, comic))
            continue;

        seriesNumberMatches.append(comic);

        if (!book.volume.isEmpty() && sameValue(book.volume, comic.volume))
            exactMatches.append(comic);
    }

    if (!book.volume.isEmpty() && exactMatches.size() == 1) {
        return resolvedMatch(exactMatches, CblMatchTier::ExactMetadata);
    }

    if (!book.volume.isEmpty() && exactMatches.size() > 1) {
        return resolvedMatch(exactMatches, CblMatchTier::ExactMetadata);
    }

    if (seriesNumberMatches.size() == 1) {
        return resolvedMatch(seriesNumberMatches, CblMatchTier::ExactMetadata);
    }

    if (seriesNumberMatches.size() > 1) {
        QList<LibraryComicMatchData> narrowed = seriesNumberMatches;
        if (!book.year.isEmpty()) {
            QList<LibraryComicMatchData> byYear;
            for (const auto &comic : std::as_const(narrowed)) {
                if (!comic.year.isEmpty() && qAbs(comic.year.toInt() - book.year.toInt()) <= 1)
                    byYear.append(comic);
            }
            if (!byYear.isEmpty())
                narrowed = byYear;
        }
        if (!book.format.isEmpty()) {
            QList<LibraryComicMatchData> byFormat;
            for (const auto &comic : std::as_const(narrowed)) {
                if (sameValue(comic.format, book.format))
                    byFormat.append(comic);
            }
            if (!byFormat.isEmpty())
                narrowed = byFormat;
        }
        return resolvedMatch(narrowed, CblMatchTier::ExactMetadata);
    }

    QList<LibraryComicMatchData> normalizedMatches;
    const auto relaxedSeries = normalizedSeries(book.series, true);
    for (const auto &comic : libraryComics) {
        if (!continueMatching())
            return { };
        if (normalizedIssueNumber(book.number) == normalizedIssueNumber(comic.number)
            && relaxedSeries == normalizedSeries(comic.series, true)
            && matchesRequiredDates(book, comic))
            normalizedMatches.append(comic);
    }
    if (!normalizedMatches.isEmpty())
        return resolvedMatch(normalizedMatches, CblMatchTier::NormalizedMetadata);

    QList<LibraryComicMatchData> alternateMatches;
    for (const auto &comic : libraryComics) {
        if (!continueMatching())
            return { };
        if (normalizedIssueNumber(book.number) == normalizedIssueNumber(comic.number)
            && relaxedSeries == normalizedSeries(comic.alternateSeries, true)
            && matchesRequiredDates(book, comic))
            alternateMatches.append(comic);
    }
    if (!alternateMatches.isEmpty())
        return resolvedMatch(alternateMatches, CblMatchTier::AlternateSeries);

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
            if (!continueMatching())
                return { };
            if (!matchesRequiredDates(book, comic))
                continue;
            if (!normalized(comic.fileName).contains(normalizedSeries))
                continue;
            if (issueExpression.match(comic.fileName).hasMatch())
                fileMatches.append(comic);
        }

        if (fileMatches.size() == 1) {
            return resolvedMatch(fileMatches, CblMatchTier::FileName);
        }

        if (fileMatches.size() > 1) {
            return resolvedMatch(fileMatches, CblMatchTier::FileName);
        }
    }

    return { };
}

QString matchTierName(CblMatchTier tier)
{
    switch (tier) {
    case CblMatchTier::SavedRemap: return QObject::tr("saved choice");
    case CblMatchTier::ComicVineIssue: return QObject::tr("ComicVine ID");
    case CblMatchTier::ExactMetadata: return QObject::tr("exact metadata");
    case CblMatchTier::NormalizedMetadata: return QObject::tr("normalized metadata");
    case CblMatchTier::AlternateSeries: return QObject::tr("alternate series");
    case CblMatchTier::FileName: return QObject::tr("filename");
    case CblMatchTier::Manual: return QObject::tr("manual choice");
    case CblMatchTier::None: return QObject::tr("unresolved");
    }
    return QObject::tr("unresolved");
}

QString comicLabel(const LibraryComicMatchData &comic)
{
    QString label = QStringLiteral("%1 #%2").arg(comic.series, comic.number);
    if (!comic.volume.isEmpty())
        label += QObject::tr(" (volume %1)").arg(comic.volume);
    if (!comic.year.isEmpty())
        label += QStringLiteral(" — %1").arg(comic.year);
    if (!comic.fileName.isEmpty())
        label += QStringLiteral(" — %1").arg(comic.fileName);
    return label;
}

bool reviewMatches(QWidget *parent,
                   const QString &name,
                   QList<MatchedCblEntry> *entries,
                   const QList<LibraryComicMatchData> &libraryComics,
                   const QString &acceptText)
{
    QDialog dialog(parent);
    dialog.setWindowTitle(QObject::tr("Review CBL reading list"));
    dialog.resize(1050, 650);

    auto *layout = new QVBoxLayout(&dialog);
    auto *summary = new QLabel(QObject::tr("Review %1. Ambiguous and missing entries remain as placeholders unless you choose a comic.").arg(name), &dialog);
    summary->setWordWrap(true);
    layout->addWidget(summary);

    auto *filter = new QComboBox(&dialog);
    filter->addItem(QObject::tr("All entries"), -1);
    filter->addItem(QObject::tr("Matched"), static_cast<int>(CblMatchState::Matched));
    filter->addItem(QObject::tr("Missing"), static_cast<int>(CblMatchState::Missing));
    filter->addItem(QObject::tr("Ambiguous"), static_cast<int>(CblMatchState::Ambiguous));
    layout->addWidget(filter);

    auto *moveUp = new QPushButton(QObject::tr("Move up"), &dialog);
    auto *moveDown = new QPushButton(QObject::tr("Move down"), &dialog);
    auto *undo = new QPushButton(QObject::tr("Undo"), &dialog);
    undo->setEnabled(false);
    auto *saveState = new QLabel(QObject::tr("No unsaved changes"), &dialog);
    auto *orderLayout = new QHBoxLayout;
    orderLayout->addWidget(moveUp);
    orderLayout->addWidget(moveDown);
    orderLayout->addWidget(undo);
    orderLayout->addWidget(saveState);
    orderLayout->addStretch(1);
    layout->addLayout(orderLayout);

    auto *table = new QTableWidget(entries->size(), 5, &dialog);
    table->setHorizontalHeaderLabels({ QObject::tr("Order"), QObject::tr("CBL entry"), QObject::tr("Status"), QObject::tr("Match method"), QObject::tr("Library comic") });
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->verticalHeader()->hide();
    table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    table->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);
    for (int row = 0; row < entries->size(); ++row) {
        const auto &entry = entries->at(row);
        const QString status = entry.match.state == CblMatchState::Matched ? QObject::tr("Matched")
                : entry.match.state == CblMatchState::Ambiguous ? QObject::tr("Ambiguous") : QObject::tr("Missing");
        table->setItem(row, 0, new QTableWidgetItem(QString::number(row + 1)));
        table->setItem(row, 1, new QTableWidgetItem(QStringLiteral("%1 #%2").arg(entry.book.series, entry.book.number)));
        table->setItem(row, 2, new QTableWidgetItem(status));
        table->setItem(row, 3, new QTableWidgetItem(matchTierName(entry.match.tier)));
        table->setItem(row, 4, new QTableWidgetItem(entry.match.state == CblMatchState::Matched && !entry.match.candidates.isEmpty()
                                                            ? comicLabel(entry.match.candidates.constFirst()) : QString()));
    }
    layout->addWidget(table, 1);

    auto *comicSearch = new QLineEdit(&dialog);
    comicSearch->setPlaceholderText(QObject::tr("Search series, issue, year, or filename..."));
    layout->addWidget(comicSearch);

    auto *comicTree = new QTreeWidget(&dialog);
    comicTree->setHeaderLabels({ QObject::tr("Library series and comics") });
    comicTree->setUniformRowHeights(true);
    comicTree->setRootIsDecorated(true);
    comicTree->setMinimumHeight(190);
    QHash<QString, QTreeWidgetItem *> seriesItems;
    QHash<qulonglong, QTreeWidgetItem *> comicItemsById;
    for (const auto &comic : libraryComics) {
        const QString seriesName = comic.series.isEmpty() ? QObject::tr("Unknown series") : comic.series;
        const QString key = normalized(seriesName);
        auto *seriesItem = seriesItems.value(key, nullptr);
        if (!seriesItem) {
            seriesItem = new QTreeWidgetItem(comicTree, { seriesName });
            seriesItem->setFlags(seriesItem->flags() & ~Qt::ItemIsSelectable);
            seriesItems.insert(key, seriesItem);
        }
        auto *comicItem = new QTreeWidgetItem(seriesItem, { comicLabel(comic) });
        comicItem->setData(0, Qt::UserRole, QVariant::fromValue(comic.id));
        comicItemsById.insert(comic.id, comicItem);
    }
    comicTree->sortItems(0, Qt::AscendingOrder);
    layout->addWidget(comicTree, 1);

    auto *applyChoice = new QPushButton(QObject::tr("Use selected comic"), &dialog);
    auto *clearChoice = new QPushButton(QObject::tr("Leave as placeholder"), &dialog);
    auto *choiceLayout = new QHBoxLayout;
    choiceLayout->addStretch(1);
    choiceLayout->addWidget(applyChoice);
    choiceLayout->addWidget(clearChoice);
    layout->addLayout(choiceLayout);

    auto refreshRow = [table, entries](int row) {
        const auto &entry = entries->at(row);
        const QString status = entry.match.state == CblMatchState::Matched ? QObject::tr("Matched")
                : entry.match.state == CblMatchState::Ambiguous ? QObject::tr("Ambiguous") : QObject::tr("Missing");
        table->item(row, 2)->setText(status);
        table->item(row, 3)->setText(matchTierName(entry.match.tier));
        table->item(row, 4)->setText(entry.match.state == CblMatchState::Matched && !entry.match.candidates.isEmpty()
                                             ? comicLabel(entry.match.candidates.constFirst()) : QString());
    };
    auto refreshAllRows = [table, entries, refreshRow, filter] {
        const int wanted = filter->currentData().toInt();
        for (int row = 0; row < entries->size(); ++row) {
            auto &entry = (*entries)[row];
            entry.book.ordering = row;
            table->item(row, 0)->setText(QString::number(row + 1));
            table->item(row, 1)->setText(QStringLiteral("%1 #%2").arg(entry.book.series, entry.book.number));
            refreshRow(row);
            table->setRowHidden(row, wanted >= 0 && static_cast<int>(entry.match.state) != wanted);
        }
    };
    QList<QList<MatchedCblEntry>> undoStack;
    const auto rememberChange = [&] {
        undoStack.append(*entries);
        if (undoStack.size() > 50)
            undoStack.removeFirst();
        undo->setEnabled(true);
        saveState->setText(QObject::tr("Unsaved changes"));
    };
    QObject::connect(applyChoice, &QPushButton::clicked, &dialog, [=] {
        const int row = table->currentRow();
        const auto selected = comicTree->selectedItems();
        const auto comicId = selected.isEmpty() ? 0 : selected.constFirst()->data(0, Qt::UserRole).toULongLong();
        if (row < 0 || comicId == 0)
            return;
        rememberChange();
        for (const auto &comic : libraryComics) {
            if (comic.id == comicId) {
                auto &entry = (*entries)[row];
                entry.match = resolvedMatch({ comic }, CblMatchTier::Manual);
                entry.match.saveRemap = true;
                refreshRow(row);
                break;
            }
        }
    });
    QObject::connect(clearChoice, &QPushButton::clicked, &dialog, [=] {
        const int row = table->currentRow();
        if (row < 0)
            return;
        rememberChange();
        (*entries)[row].match = { };
        refreshRow(row);
    });
    QObject::connect(moveUp, &QPushButton::clicked, &dialog, [=] {
        const int row = table->currentRow();
        if (row <= 0)
            return;
        rememberChange();
        entries->swapItemsAt(row, row - 1);
        refreshAllRows();
        table->selectRow(row - 1);
    });
    QObject::connect(moveDown, &QPushButton::clicked, &dialog, [=] {
        const int row = table->currentRow();
        if (row < 0 || row >= entries->size() - 1)
            return;
        rememberChange();
        entries->swapItemsAt(row, row + 1);
        refreshAllRows();
        table->selectRow(row + 1);
    });
    QObject::connect(undo, &QPushButton::clicked, &dialog, [&] {
        if (undoStack.isEmpty())
            return;
        *entries = undoStack.takeLast();
        refreshAllRows();
        undo->setEnabled(!undoStack.isEmpty());
        saveState->setText(undoStack.isEmpty() ? QObject::tr("No unsaved changes") : QObject::tr("Unsaved changes"));
    });
    QObject::connect(filter, QOverload<int>::of(&QComboBox::currentIndexChanged), &dialog, [=](int) {
        const int wanted = filter->currentData().toInt();
        for (int row = 0; row < entries->size(); ++row)
            table->setRowHidden(row, wanted >= 0 && static_cast<int>(entries->at(row).match.state) != wanted);
    });
    QObject::connect(comicSearch, &QLineEdit::textChanged, &dialog, [=](const QString &text) {
        const QString needle = normalized(text);
        for (int seriesRow = 0; seriesRow < comicTree->topLevelItemCount(); ++seriesRow) {
            auto *seriesItem = comicTree->topLevelItem(seriesRow);
            const bool seriesMatches = needle.isEmpty() || normalized(seriesItem->text(0)).contains(needle);
            bool anyVisible = seriesMatches;
            for (int comicRow = 0; comicRow < seriesItem->childCount(); ++comicRow) {
                auto *comicItem = seriesItem->child(comicRow);
                const bool visible = seriesMatches || normalized(comicItem->text(0)).contains(needle);
                comicItem->setHidden(!visible);
                anyVisible = anyVisible || visible;
            }
            seriesItem->setHidden(!anyVisible);
            seriesItem->setExpanded(!needle.isEmpty() && anyVisible);
        }
    });
    QObject::connect(comicTree, &QTreeWidget::itemDoubleClicked, applyChoice, [=](QTreeWidgetItem *item, int) {
        if (item && item->data(0, Qt::UserRole).toULongLong() != 0)
            applyChoice->click();
    });
    QObject::connect(table, &QTableWidget::currentCellChanged, &dialog, [=](int currentRow, int, int, int) {
        if (currentRow < 0)
            return;
        const auto &entry = entries->at(currentRow);
        if (entry.match.state == CblMatchState::Matched && !entry.match.candidates.isEmpty()) {
            const auto wantedId = entry.match.candidates.constFirst().id;
            if (auto *item = comicItemsById.value(wantedId, nullptr)) {
                comicTree->setCurrentItem(item);
                comicTree->scrollToItem(item);
            }
        }
    });

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    buttons->button(QDialogButtonBox::Ok)->setText(acceptText);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(buttons);
    if (!entries->isEmpty())
        table->selectRow(0);
    return dialog.exec() == QDialog::Accepted;
}

bool execSql(QSqlQuery &query, QString *error)
{
    if (query.exec())
        return true;
    if (error)
        *error = query.lastError().text();
    return false;
}

bool ensureColumn(QSqlDatabase &db, const QString &table, const QString &name, const QString &definition, QString *error)
{
    QSqlQuery columns(db);
    if (!columns.exec(QStringLiteral("PRAGMA table_info(%1)").arg(table))) {
        if (error)
            *error = columns.lastError().text();
        return false;
    }

    while (columns.next()) {
        if (columns.value(1).toString() == name)
            return true;
    }

    QSqlQuery alter(db);
    alter.prepare(QStringLiteral("ALTER TABLE %1 ADD COLUMN %2 %3").arg(table, name, definition));
    return execSql(alter, error);
}

bool ensureCblImportTables(QSqlDatabase &db, QString *error)
{
    QSqlQuery meta(db);
    meta.prepare(QStringLiteral(
            "CREATE TABLE IF NOT EXISTS cbl_reading_list_meta ("
            "reading_list_id INTEGER PRIMARY KEY, "
            "source_name TEXT, "
            "source_path TEXT, "
            "source_hash TEXT, "
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
            "format TEXT, "
            "file_name TEXT, "
            "source_id TEXT, "
            "comicvine_series_id TEXT, "
            "comicvine_issue_id TEXT, "
            "match_state INTEGER NOT NULL, "
            "match_tier INTEGER NOT NULL DEFAULT 0, "
            "candidate_count INTEGER NOT NULL DEFAULT 0, "
            "FOREIGN KEY(reading_list_id) REFERENCES reading_list(id) ON DELETE CASCADE, "
            "FOREIGN KEY(comic_id) REFERENCES comic(id) ON DELETE SET NULL, "
            "UNIQUE(reading_list_id, ordering))"));
    if (!execSql(entries, error))
        return false;

    QSqlQuery remaps(db);
    remaps.prepare(QStringLiteral(
            "CREATE TABLE IF NOT EXISTS cbl_remap_rule ("
            "id INTEGER PRIMARY KEY, "
            "series_key TEXT NOT NULL, volume_key TEXT NOT NULL DEFAULT '', number_key TEXT NOT NULL DEFAULT '', "
            "comic_id INTEGER NOT NULL, created_at INTEGER NOT NULL, "
            "FOREIGN KEY(comic_id) REFERENCES comic(id) ON DELETE CASCADE, "
            "UNIQUE(series_key, volume_key, number_key))"));
    if (!execSql(remaps, error))
        return false;

    if (!ensureColumn(db, QStringLiteral("cbl_reading_list_meta"), QStringLiteral("source_path"), QStringLiteral("TEXT"), error)
        || !ensureColumn(db, QStringLiteral("cbl_reading_list_meta"), QStringLiteral("source_hash"), QStringLiteral("TEXT"), error)
        || !ensureColumn(db, QStringLiteral("cbl_reading_list_entry"), QStringLiteral("comicvine_series_id"), QStringLiteral("TEXT"), error)
        || !ensureColumn(db, QStringLiteral("cbl_reading_list_entry"), QStringLiteral("comicvine_issue_id"), QStringLiteral("TEXT"), error)
        || !ensureColumn(db, QStringLiteral("cbl_reading_list_entry"), QStringLiteral("format"), QStringLiteral("TEXT"), error)
        || !ensureColumn(db, QStringLiteral("cbl_reading_list_entry"), QStringLiteral("file_name"), QStringLiteral("TEXT"), error)
        || !ensureColumn(db, QStringLiteral("cbl_reading_list_entry"), QStringLiteral("match_tier"), QStringLiteral("INTEGER NOT NULL DEFAULT 0"), error))
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
                           const QString &sourcePath,
                           const QString &sourceHash,
                           qulonglong parentReadingListId,
                           qulonglong forcedExistingId,
                           bool createCopy,
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

    qulonglong newReadingListId = forcedExistingId;
    if (newReadingListId == 0 && !createCopy) {
        QSqlQuery existing(db);
        existing.prepare(QStringLiteral("SELECT reading_list_id FROM cbl_reading_list_meta "
                                        "WHERE source_path = :source_path OR (source_path IS NULL AND source_name = :source_name) LIMIT 1"));
        existing.bindValue(QStringLiteral(":source_path"), sourcePath);
        existing.bindValue(QStringLiteral(":source_name"), sourceName);
        if (!existing.exec())
            return rollback(existing.lastError().text());
        if (existing.next())
            newReadingListId = existing.value(0).toULongLong();
    }

    if (newReadingListId != 0) {
        const bool hasMatch = std::any_of(entries.cbegin(), entries.cend(), [](const MatchedCblEntry &entry) {
            return entry.match.state == CblMatchState::Matched;
        });
        QSqlQuery previousMatches(db);
        previousMatches.prepare(QStringLiteral("SELECT COUNT(*) FROM cbl_reading_list_entry WHERE reading_list_id = :id AND comic_id IS NOT NULL"));
        previousMatches.bindValue(QStringLiteral(":id"), newReadingListId);
        if (!previousMatches.exec() || !previousMatches.next())
            return rollback(previousMatches.lastError().text());
        if (!hasMatch && previousMatches.value(0).toInt() > 0)
            return rollback(QStringLiteral("No comics matched during re-import, so the existing reading list was left unchanged."));

        QSqlQuery updateList(db);
        updateList.prepare(QStringLiteral("UPDATE reading_list SET name = :name WHERE id = :id"));
        updateList.bindValue(QStringLiteral(":name"), readingList.name);
        updateList.bindValue(QStringLiteral(":id"), newReadingListId);
        if (!updateList.exec())
            return rollback(updateList.lastError().text());
        for (const auto &table : { QStringLiteral("comic_reading_list"), QStringLiteral("cbl_reading_list_entry") }) {
            QSqlQuery clear(db);
            clear.prepare(QStringLiteral("DELETE FROM %1 WHERE reading_list_id = :id").arg(table));
            clear.bindValue(QStringLiteral(":id"), newReadingListId);
            if (!clear.exec())
                return rollback(clear.lastError().text());
        }
    } else {
        QSqlQuery createList(db);
        createList.prepare(parentReadingListId == 0
                                   ? QStringLiteral("INSERT INTO reading_list (name) VALUES (:name)")
                                   : QStringLiteral("INSERT INTO reading_list (name, parentId, ordering) "
                                                    "VALUES (:name, :parent_id, (SELECT COUNT(*) FROM reading_list WHERE parentId = :parent_id))"));
        createList.bindValue(QStringLiteral(":name"), readingList.name);
        if (parentReadingListId != 0)
            createList.bindValue(QStringLiteral(":parent_id"), parentReadingListId);
        if (!createList.exec())
            return rollback(createList.lastError().text());
        newReadingListId = createList.lastInsertId().toULongLong();
    }
    if (newReadingListId == 0)
        return rollback(QStringLiteral("YACReader did not return an id for the new reading list."));

    QSqlQuery meta(db);
    meta.prepare(QStringLiteral(
            "INSERT OR REPLACE INTO cbl_reading_list_meta (reading_list_id, source_name, source_path, source_hash, imported_at) "
            "VALUES (:reading_list_id, :source_name, :source_path, :source_hash, :imported_at)"));
    meta.bindValue(QStringLiteral(":reading_list_id"), newReadingListId);
    meta.bindValue(QStringLiteral(":source_name"), sourceName);
    meta.bindValue(QStringLiteral(":source_path"), sourcePath);
    meta.bindValue(QStringLiteral(":source_hash"), sourceHash);
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
                "(reading_list_id, ordering, comic_id, series, number, volume, year, format, file_name, source_id, comicvine_series_id, comicvine_issue_id, match_state, match_tier, candidate_count) "
                "VALUES (:reading_list_id, :ordering, :comic_id, :series, :number, :volume, :year, :format, :file_name, :source_id, :comicvine_series_id, :comicvine_issue_id, :match_state, :match_tier, :candidate_count)"));
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
        insertEntry.bindValue(QStringLiteral(":format"), entry.book.format);
        insertEntry.bindValue(QStringLiteral(":file_name"), entry.book.fileName);
        insertEntry.bindValue(QStringLiteral(":source_id"), entry.book.id);
        insertEntry.bindValue(QStringLiteral(":comicvine_series_id"), entry.book.comicVineSeriesId);
        insertEntry.bindValue(QStringLiteral(":comicvine_issue_id"), entry.book.comicVineIssueId);
        insertEntry.bindValue(QStringLiteral(":match_state"), static_cast<int>(entry.match.state));
        insertEntry.bindValue(QStringLiteral(":match_tier"), static_cast<int>(entry.match.tier));
        insertEntry.bindValue(QStringLiteral(":candidate_count"), entry.match.candidates.size());
        if (!insertEntry.exec())
            return rollback(insertEntry.lastError().text());

        if (comicId != 0 && entry.match.saveRemap) {
            QSqlQuery remap(db);
            remap.prepare(QStringLiteral(
                    "INSERT OR REPLACE INTO cbl_remap_rule (series_key, volume_key, number_key, comic_id, created_at) "
                    "VALUES (:series, :volume, :number, :comic_id, :created_at)"));
            remap.bindValue(QStringLiteral(":series"), normalizedSeries(entry.book.series, true));
            remap.bindValue(QStringLiteral(":volume"), normalized(entry.book.volume));
            remap.bindValue(QStringLiteral(":number"), normalizedIssueNumber(entry.book.number));
            remap.bindValue(QStringLiteral(":comic_id"), comicId);
            remap.bindValue(QStringLiteral(":created_at"), QDateTime::currentSecsSinceEpoch());
            if (!remap.exec())
                return rollback(remap.lastError().text());
        }

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

void ensureSmartListTables(QSqlDatabase &db)
{
    db.exec(QStringLiteral("CREATE TABLE IF NOT EXISTS reading_list_smart (reading_list_id INTEGER PRIMARY KEY, rules_json TEXT NOT NULL, "
                           "FOREIGN KEY(reading_list_id) REFERENCES reading_list(id) ON DELETE CASCADE)"));
}

QJsonObject smartListRulesFromDialog(QLineEdit *series,
                                     QLineEdit *title,
                                     QLineEdit *creator,
                                     QLineEdit *publisher,
                                     QLineEdit *storyArc,
                                     QLineEdit *tag,
                                     QLineEdit *path,
                                     QComboBox *readState,
                                     QComboBox *openedState,
                                     QComboBox *type,
                                     QSpinBox *ratingMin,
                                     QSpinBox *yearFrom,
                                     QSpinBox *yearTo,
                                     QSpinBox *addedWithinDays,
                                     QSpinBox *openedWithinDays,
                                     QComboBox *sortField,
                                     QComboBox *sortDirection)
{
    QJsonObject rules;
    const auto addText = [&rules](const QString &key, QLineEdit *edit) {
        const QString value = edit->text().trimmed();
        if (!value.isEmpty())
            rules.insert(key, value);
    };
    addText(QStringLiteral("series"), series);
    addText(QStringLiteral("title"), title);
    addText(QStringLiteral("creator"), creator);
    addText(QStringLiteral("publisher"), publisher);
    addText(QStringLiteral("storyArc"), storyArc);
    addText(QStringLiteral("tag"), tag);
    addText(QStringLiteral("path"), path);
    if (readState->currentData().toInt() != 0)
        rules.insert(QStringLiteral("readState"), readState->currentData().toInt());
    if (openedState->currentData().toInt() != 0)
        rules.insert(QStringLiteral("openedState"), openedState->currentData().toInt());
    if (type->currentData().toInt() >= 0)
        rules.insert(QStringLiteral("type"), type->currentData().toInt());
    if (ratingMin->value() > 0)
        rules.insert(QStringLiteral("ratingMin"), ratingMin->value());
    if (yearFrom->value() > 0)
        rules.insert(QStringLiteral("yearFrom"), yearFrom->value());
    if (yearTo->value() > 0)
        rules.insert(QStringLiteral("yearTo"), yearTo->value());
    if (addedWithinDays->value() > 0)
        rules.insert(QStringLiteral("addedWithinDays"), addedWithinDays->value());
    if (openedWithinDays->value() > 0)
        rules.insert(QStringLiteral("openedWithinDays"), openedWithinDays->value());
    rules.insert(QStringLiteral("sortField"), sortField->currentData().toString());
    rules.insert(QStringLiteral("sortDirection"), sortDirection->currentData().toString());
    return rules;
}

bool editSmartListDefinition(QWidget *parent, QString *name, QJsonObject *rules)
{
    QDialog dialog(parent);
    dialog.setWindowTitle(QObject::tr("Smart list"));
    dialog.resize(560, 520);

    auto *layout = new QVBoxLayout(&dialog);
    auto *form = new QFormLayout;
    layout->addLayout(form);

    auto *nameEdit = new QLineEdit(*name, &dialog);
    form->addRow(QObject::tr("Name:"), nameEdit);

    auto makeLineEdit = [&dialog, rules](const QString &key) {
        return new QLineEdit(rules->value(key).toString(), &dialog);
    };
    auto *series = makeLineEdit(QStringLiteral("series"));
    auto *title = makeLineEdit(QStringLiteral("title"));
    auto *creator = makeLineEdit(QStringLiteral("creator"));
    auto *publisher = makeLineEdit(QStringLiteral("publisher"));
    auto *storyArc = makeLineEdit(QStringLiteral("storyArc"));
    auto *tag = makeLineEdit(QStringLiteral("tag"));
    auto *path = makeLineEdit(QStringLiteral("path"));
    form->addRow(QObject::tr("Series contains:"), series);
    form->addRow(QObject::tr("Title contains:"), title);
    form->addRow(QObject::tr("Creator contains:"), creator);
    form->addRow(QObject::tr("Publisher contains:"), publisher);
    form->addRow(QObject::tr("Story arc contains:"), storyArc);
    form->addRow(QObject::tr("Tags/characters contain:"), tag);
    form->addRow(QObject::tr("Path or filename contains:"), path);

    auto *readState = new QComboBox(&dialog);
    readState->addItem(QObject::tr("Any"), 0);
    readState->addItem(QObject::tr("Unread"), 1);
    readState->addItem(QObject::tr("Read"), 2);
    readState->setCurrentIndex(readState->findData(rules->value(QStringLiteral("readState")).toInt(0)));
    form->addRow(QObject::tr("Read state:"), readState);

    auto *openedState = new QComboBox(&dialog);
    openedState->addItem(QObject::tr("Any"), 0);
    openedState->addItem(QObject::tr("Never opened"), 1);
    openedState->addItem(QObject::tr("Opened"), 2);
    openedState->setCurrentIndex(openedState->findData(rules->value(QStringLiteral("openedState")).toInt(0)));
    form->addRow(QObject::tr("Opened state:"), openedState);

    auto *type = new QComboBox(&dialog);
    type->addItem(QObject::tr("Any"), -1);
    type->addItem(QObject::tr("Comic"), 0);
    type->addItem(QObject::tr("Manga"), 1);
    type->addItem(QObject::tr("Manga left to right"), 2);
    type->addItem(QObject::tr("Web comic"), 3);
    type->addItem(QObject::tr("4-koma"), 4);
    type->setCurrentIndex(type->findData(rules->value(QStringLiteral("type")).toInt(-1)));
    form->addRow(QObject::tr("Type:"), type);

    auto makeSpin = [&dialog](int maximum) {
        auto *spin = new QSpinBox(&dialog);
        spin->setRange(0, maximum);
        spin->setSpecialValueText(QObject::tr("Any"));
        return spin;
    };
    auto *ratingMin = makeSpin(5);
    auto *yearFrom = makeSpin(9999);
    auto *yearTo = makeSpin(9999);
    auto *addedWithinDays = makeSpin(36500);
    auto *openedWithinDays = makeSpin(36500);
    ratingMin->setValue(rules->value(QStringLiteral("ratingMin")).toInt(0));
    yearFrom->setValue(rules->value(QStringLiteral("yearFrom")).toInt(0));
    yearTo->setValue(rules->value(QStringLiteral("yearTo")).toInt(0));
    addedWithinDays->setValue(rules->value(QStringLiteral("addedWithinDays")).toInt(0));
    openedWithinDays->setValue(rules->value(QStringLiteral("openedWithinDays")).toInt(0));
    form->addRow(QObject::tr("Minimum rating:"), ratingMin);
    form->addRow(QObject::tr("Year from:"), yearFrom);
    form->addRow(QObject::tr("Year to:"), yearTo);
    form->addRow(QObject::tr("Added in last days:"), addedWithinDays);
    form->addRow(QObject::tr("Opened in last days:"), openedWithinDays);

    auto *sortField = new QComboBox(&dialog);
    sortField->addItem(QObject::tr("Series and issue"), QStringLiteral("series"));
    sortField->addItem(QObject::tr("Title"), QStringLiteral("title"));
    sortField->addItem(QObject::tr("File name"), QStringLiteral("fileName"));
    sortField->addItem(QObject::tr("Publication year"), QStringLiteral("year"));
    sortField->addItem(QObject::tr("Added date"), QStringLiteral("added"));
    sortField->addItem(QObject::tr("Last opened"), QStringLiteral("lastOpened"));
    sortField->addItem(QObject::tr("Rating"), QStringLiteral("rating"));
    sortField->addItem(QObject::tr("Publisher"), QStringLiteral("publisher"));
    const int sortIndex = sortField->findData(rules->value(QStringLiteral("sortField")).toString(QStringLiteral("series")));
    sortField->setCurrentIndex(sortIndex < 0 ? 0 : sortIndex);
    form->addRow(QObject::tr("Sort by:"), sortField);

    auto *sortDirection = new QComboBox(&dialog);
    sortDirection->addItem(QObject::tr("Ascending"), QStringLiteral("asc"));
    sortDirection->addItem(QObject::tr("Descending"), QStringLiteral("desc"));
    const int directionIndex = sortDirection->findData(rules->value(QStringLiteral("sortDirection")).toString(QStringLiteral("asc")));
    sortDirection->setCurrentIndex(directionIndex < 0 ? 0 : directionIndex);
    form->addRow(QObject::tr("Sort direction:"), sortDirection);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, [&] {
        const auto trimmedName = nameEdit->text().trimmed();
        if (trimmedName.isEmpty()) {
            QMessageBox::information(&dialog, QObject::tr("Smart list"), QObject::tr("Enter a name for the smart list."));
            return;
        }
        const auto updatedRules = smartListRulesFromDialog(series, title, creator, publisher, storyArc, tag, path,
                                                           readState, openedState, type, ratingMin, yearFrom, yearTo,
                                                           addedWithinDays, openedWithinDays, sortField, sortDirection);
        if (updatedRules.size() <= 2) {
            QMessageBox::information(&dialog, QObject::tr("Smart list"), QObject::tr("Add at least one rule."));
            return;
        }
        *name = trimmedName;
        *rules = updatedRules;
        dialog.accept();
    });
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(buttons);

    return dialog.exec() == QDialog::Accepted;
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

    if (currentList.isValid() && listsModel->isReadingListFolder(currentList))
        listsModel->addReadingListAt(name, currentList);
    else
        listsModel->addReadingList(name);
}

void ReadingListManagementCoordinator::addSmartList()
{
    const auto currentList = currentListProvider();
    if (currentList.isValid() && listsModel->isReadingSubList(currentList))
        return;

    QString name = tr("New smart list");
    QJsonObject rules;
    if (!editSmartListDefinition(dialogParent, &name, &rules))
        return;

    QString error;
    QString connectionName;
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(listsModel->databasePath());
        connectionName = db.connectionName();
        ensureSmartListTables(db);

        qulonglong id = 0;
        if (currentList.isValid() && listsModel->isReadingListFolder(currentList)) {
            QSqlQuery childCount(db);
            childCount.prepare(QStringLiteral("SELECT COUNT(*) FROM reading_list WHERE parentId = :parentId"));
            childCount.bindValue(QStringLiteral(":parentId"), currentList.data(ReadingListModel::IDRole).toULongLong());
            const int ordering = childCount.exec() && childCount.next() ? childCount.value(0).toInt() : 0;
            id = DBHelper::insertReadingSubList(name, currentList.data(ReadingListModel::IDRole).toULongLong(), ordering, db);
        } else {
            id = DBHelper::insertReadingList(name, db);
        }

        QSqlQuery smart(db);
        smart.prepare(QStringLiteral("INSERT INTO reading_list_smart (reading_list_id, rules_json) VALUES (:id, :rules)"));
        smart.bindValue(QStringLiteral(":id"), id);
        smart.bindValue(QStringLiteral(":rules"), QString::fromUtf8(QJsonDocument(rules).toJson(QJsonDocument::Compact)));
        if (!smart.exec())
            error = smart.lastError().text();
    }
    QSqlDatabase::removeDatabase(connectionName);

    if (!error.isEmpty()) {
        QMessageBox::critical(dialogParent, tr("Unable to save smart list"), error);
        return;
    }

    listsModel->setupReadingListsData(listsModel->databasePath());
    emit currentListReselectionRequested();
}

void ReadingListManagementCoordinator::editSmartList()
{
    const auto currentList = currentListProvider();
    if (!currentList.isValid() || !listsModel->isSmartList(currentList)) {
        QMessageBox::information(dialogParent, tr("Edit smart list"), tr("Select a smart list first."));
        return;
    }

    const auto readingListId = currentList.data(ReadingListModel::IDRole).toULongLong();
    QString name = listsModel->name(currentList);
    QJsonObject rules;
    QString error;
    QString connectionName;
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(listsModel->databasePath());
        connectionName = db.connectionName();
        ensureSmartListTables(db);
        QSqlQuery smart(db);
        smart.prepare(QStringLiteral("SELECT rules_json FROM reading_list_smart WHERE reading_list_id = :id"));
        smart.bindValue(QStringLiteral(":id"), readingListId);
        if (!smart.exec()) {
            error = smart.lastError().text();
        } else if (!smart.next()) {
            error = tr("This reading list is not a smart list.");
        } else {
            rules = QJsonDocument::fromJson(smart.value(0).toString().toUtf8()).object();
        }
    }
    QSqlDatabase::removeDatabase(connectionName);

    if (!error.isEmpty()) {
        QMessageBox::information(dialogParent, tr("Edit smart list"), error);
        return;
    }
    if (!editSmartListDefinition(dialogParent, &name, &rules))
        return;

    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(listsModel->databasePath());
        connectionName = db.connectionName();
        DBHelper::renameList(readingListId, name, db);
        QSqlQuery smart(db);
        smart.prepare(QStringLiteral("UPDATE reading_list_smart SET rules_json = :rules WHERE reading_list_id = :id"));
        smart.bindValue(QStringLiteral(":rules"), QString::fromUtf8(QJsonDocument(rules).toJson(QJsonDocument::Compact)));
        smart.bindValue(QStringLiteral(":id"), readingListId);
        if (!smart.exec())
            error = smart.lastError().text();
    }
    QSqlDatabase::removeDatabase(connectionName);

    if (!error.isEmpty()) {
        QMessageBox::critical(dialogParent, tr("Unable to save smart list"), error);
        return;
    }

    listsModel->setupReadingListsData(listsModel->databasePath());
    emit currentListReselectionRequested();
}

void ReadingListManagementCoordinator::addReadingListFolder()
{
    bool accepted = false;
    const auto name = QInputDialog::getText(dialogParent,
                                            tr("Add reading-list folder"),
                                            tr("Folder name:"),
                                            QLineEdit::Normal,
                                            { },
                                            &accepted);
    if (accepted && !name.trimmed().isEmpty())
        listsModel->addReadingListFolder(name.trimmed());
}

void ReadingListManagementCoordinator::moveReadingListToFolder()
{
    const auto currentList = currentListProvider();
    if (!currentList.isValid() || !listsModel->isReadingList(currentList)) {
        QMessageBox::information(dialogParent,
                                 tr("Move reading list"),
                                 tr("Select a reading list to move."));
        return;
    }

    QSqlDatabase db = DataBaseManagement::loadDatabase(listsModel->databasePath());
    const auto connectionName = db.connectionName();
    QSqlQuery folders(db);
    if (!folders.exec(QStringLiteral("SELECT rl.id, rl.name FROM reading_list rl "
                                     "INNER JOIN reading_list_folder rlf ON rlf.reading_list_id = rl.id "
                                     "ORDER BY rl.name COLLATE NOCASE"))) {
        QMessageBox::critical(dialogParent, tr("Unable to move reading list"), folders.lastError().text());
        db.close();
        db = QSqlDatabase();
        QSqlDatabase::removeDatabase(connectionName);
        return;
    }

    QStringList choices { tr("Top level") };
    QList<qulonglong> folderIds { 0 };
    while (folders.next()) {
        folderIds.append(folders.value(0).toULongLong());
        choices.append(folders.value(1).toString());
    }

    bool accepted = false;
    const auto choice = QInputDialog::getItem(dialogParent,
                                               tr("Move reading list"),
                                               tr("Destination:"),
                                               choices,
                                               0,
                                               false,
                                               &accepted);
    if (!accepted) {
        db.close();
        db = QSqlDatabase();
        QSqlDatabase::removeDatabase(connectionName);
        return;
    }

    const auto destinationId = folderIds.at(choices.indexOf(choice));
    QSqlQuery count(db);
    if (destinationId == 0)
        count.prepare(QStringLiteral("SELECT COUNT(*) FROM reading_list WHERE parentId IS NULL"));
    else {
        count.prepare(QStringLiteral("SELECT COUNT(*) FROM reading_list WHERE parentId = :parentId"));
        count.bindValue(QStringLiteral(":parentId"), destinationId);
    }

    int ordering = 0;
    if (count.exec() && count.next())
        ordering = count.value(0).toInt();

    QSqlQuery move(db);
    move.prepare(QStringLiteral("UPDATE reading_list SET parentId = :parentId, ordering = :ordering WHERE id = :id"));
    move.bindValue(QStringLiteral(":parentId"), destinationId == 0 ? QVariant() : QVariant::fromValue(destinationId));
    move.bindValue(QStringLiteral(":ordering"), ordering);
    move.bindValue(QStringLiteral(":id"), currentList.data(ReadingListModel::IDRole));
    const auto moved = move.exec();
    const auto error = move.lastError().text();

    db.close();
    db = QSqlDatabase();
    QSqlDatabase::removeDatabase(connectionName);

    if (!moved) {
        QMessageBox::critical(dialogParent, tr("Unable to move reading list"), error);
        return;
    }

    listsModel->setupReadingListsData(listsModel->databasePath());
    emit currentListReselectionRequested();
}

void ReadingListManagementCoordinator::importCblReadingList()
{
    const auto filePath = QFileDialog::getOpenFileName(dialogParent,
                                                       tr("Import CBL reading list"),
                                                       { },
                                                       tr("Comic Book Reading Lists (*.cbl);;XML files (*.xml);;All files (*)"));
    if (filePath.isEmpty())
        return;

    importCblReadingListFromPath(filePath);
}

void ReadingListManagementCoordinator::updateCblReadingListFromSource()
{
    const auto currentList = currentListProvider();
    if (!currentList.isValid() || !listsModel->isReadingList(currentList)) {
        QMessageBox::information(dialogParent, tr("Update reading list"), tr("Select an imported CBL reading list first."));
        return;
    }

    const auto readingListId = currentList.data(ReadingListModel::IDRole).toULongLong();
    QString sourcePath;
    QString connectionName;
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(listsModel->databasePath());
        connectionName = db.connectionName();
        QSqlQuery source(db);
        source.prepare(QStringLiteral("SELECT source_path FROM cbl_reading_list_meta WHERE reading_list_id = :id"));
        source.bindValue(QStringLiteral(":id"), readingListId);
        if (source.exec() && source.next())
            sourcePath = source.value(0).toString();
    }
    QSqlDatabase::removeDatabase(connectionName);

    if (sourcePath.isEmpty() || !QFileInfo::exists(sourcePath)) {
        QMessageBox::warning(dialogParent,
                             tr("Source CBL unavailable"),
                             tr("The original CBL file could not be found. Use Import CBL to choose it again."));
        return;
    }
    importCblReadingListFromPath(sourcePath, readingListId);
}

void ReadingListManagementCoordinator::importCblReadingListFromPath(const QString &filePath, qulonglong updateReadingListId)
{

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
    QHash<QString, qulonglong> remaps;
    QString databaseError;
    QString connectionName;
    bool matchingCanceled = false;
    QProgressDialog matchingProgress(tr("Loading library comics for matching..."),
                                     tr("Cancel"),
                                     0,
                                     0,
                                     dialogParent);
    matchingProgress.setWindowTitle(tr("Matching comics"));
    matchingProgress.setWindowModality(Qt::WindowModal);
    matchingProgress.setMinimumDuration(0);
    matchingProgress.setAutoClose(false);
    matchingProgress.setAutoReset(false);
    matchingProgress.show();
    QApplication::processEvents();
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(listsModel->databasePath());
        connectionName = db.connectionName();

        if (!ensureCblImportTables(db, &databaseError)) {
            // databaseError is reported below
        }

        QSqlQuery query(db);
        if (databaseError.isEmpty() && !query.exec(QStringLiteral("SELECT c.id, c.fileName, ci.series, ci.number, ci.volume, ci.comicVineID, ci.date, ci.format, ci.alternateSeries "
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
                comic.comicVineIssueId = query.value(5).toString();
                comic.year = query.value(6).toString().right(4);
                comic.format = query.value(7).toString();
                comic.alternateSeries = query.value(8).toString();
                libraryComics.append(comic);

                if (libraryComics.size() % 100 == 0) {
                    matchingProgress.setLabelText(tr("Loading library comics for matching... %1")
                                                          .arg(libraryComics.size()));
                    QApplication::processEvents();
                    if (matchingProgress.wasCanceled()) {
                        matchingCanceled = true;
                        break;
                    }
                }
            }

            QSqlQuery remapQuery(db);
            if (!matchingCanceled && !remapQuery.exec(QStringLiteral("SELECT series_key, volume_key, number_key, comic_id FROM cbl_remap_rule"))) {
                databaseError = remapQuery.lastError().text();
            } else if (!matchingCanceled) {
                while (remapQuery.next()) {
                    const QString key = remapQuery.value(0).toString() + QLatin1Char('|')
                            + remapQuery.value(1).toString() + QLatin1Char('|') + remapQuery.value(2).toString();
                    remaps.insert(key, remapQuery.value(3).toULongLong());
                }
            }
        }
    }
    QSqlDatabase::removeDatabase(connectionName);

    if (matchingCanceled || matchingProgress.wasCanceled()) {
        matchingProgress.close();
        return;
    }

    matchingProgress.hide();
    bool createCopy = false;
    if (updateReadingListId == 0) {
        QString duplicateConnection;
        qulonglong duplicateId = 0;
        QString duplicateName;
        {
            QSqlDatabase db = DataBaseManagement::loadDatabase(listsModel->databasePath());
            duplicateConnection = db.connectionName();
            ensureCblImportTables(db, nullptr);
            QSqlQuery duplicate(db);
            duplicate.prepare(QStringLiteral(
                    "SELECT rl.id, rl.name FROM reading_list rl "
                    "LEFT JOIN cbl_reading_list_meta m ON m.reading_list_id = rl.id "
                    "WHERE m.source_path = :path OR LOWER(rl.name) = LOWER(:name) LIMIT 1"));
            duplicate.bindValue(QStringLiteral(":path"), QFileInfo(filePath).canonicalFilePath());
            duplicate.bindValue(QStringLiteral(":name"), result.readingList.name);
            if (duplicate.exec() && duplicate.next()) {
                duplicateId = duplicate.value(0).toULongLong();
                duplicateName = duplicate.value(1).toString();
            }
        }
        QSqlDatabase::removeDatabase(duplicateConnection);

        if (duplicateId != 0) {
            QMessageBox duplicateDialog(dialogParent);
            duplicateDialog.setWindowTitle(tr("Reading list already exists"));
            duplicateDialog.setText(tr("A reading list named “%1” or imported from this source already exists.").arg(duplicateName));
            duplicateDialog.setInformativeText(tr("Update replaces its entries after review. Create copy keeps both lists."));
            auto *updateButton = duplicateDialog.addButton(tr("Update existing"), QMessageBox::AcceptRole);
            auto *copyButton = duplicateDialog.addButton(tr("Create copy"), QMessageBox::ActionRole);
            duplicateDialog.addButton(QMessageBox::Cancel);
            duplicateDialog.exec();
            if (duplicateDialog.clickedButton() == updateButton)
                updateReadingListId = duplicateId;
            else if (duplicateDialog.clickedButton() == copyButton)
                createCopy = true;
            else
                return;
        }
    }

    if (!databaseError.isEmpty()) {
        matchingProgress.close();
        QMessageBox::critical(dialogParent,
                              tr("Unable to inspect library"),
                              tr("YACReader could not read comic metadata for CBL matching.\n\n%1").arg(databaseError));
        return;
    }

    if (updateReadingListId != 0) {
        QStringList oldOrder;
        QString comparisonConnection;
        {
            QSqlDatabase db = DataBaseManagement::loadDatabase(listsModel->databasePath());
            comparisonConnection = db.connectionName();
            QSqlQuery oldEntries(db);
            oldEntries.prepare(QStringLiteral("SELECT series, volume, number FROM cbl_reading_list_entry "
                                              "WHERE reading_list_id = :id ORDER BY ordering"));
            oldEntries.bindValue(QStringLiteral(":id"), updateReadingListId);
            if (oldEntries.exec()) {
                while (oldEntries.next()) {
                    CblBook oldBook;
                    oldBook.series = oldEntries.value(0).toString();
                    oldBook.volume = oldEntries.value(1).toString();
                    oldBook.number = oldEntries.value(2).toString();
                    oldOrder.append(remapKey(oldBook));
                }
            }
        }
        QSqlDatabase::removeDatabase(comparisonConnection);

        QStringList newOrder;
        QHash<QString, int> oldCounts;
        QHash<QString, int> newCounts;
        for (const auto &key : oldOrder)
            ++oldCounts[key];
        for (const auto &book : result.readingList.books) {
            const auto key = remapKey(book);
            newOrder.append(key);
            ++newCounts[key];
        }
        int added = 0;
        int removed = 0;
        for (auto it = newCounts.cbegin(); it != newCounts.cend(); ++it)
            added += qMax(0, it.value() - oldCounts.value(it.key()));
        for (auto it = oldCounts.cbegin(); it != oldCounts.cend(); ++it)
            removed += qMax(0, it.value() - newCounts.value(it.key()));
        const bool reordered = added == 0 && removed == 0 && oldOrder != newOrder;
        const auto answer = QMessageBox::question(
                dialogParent,
                tr("Update from source CBL"),
                tr("Source comparison:\n\n%1 added\n%2 removed\n%3\n\nContinue to matching review?")
                        .arg(added)
                        .arg(removed)
                        .arg(reordered ? tr("Order changed") : tr("No order-only changes detected")),
                QMessageBox::Yes | QMessageBox::No,
                QMessageBox::Yes);
        if (answer != QMessageBox::Yes)
            return;
    }

    matchingProgress.show();

    const int count = result.readingList.books.size();
    int matchedCount = 0;
    int missingCount = 0;
    int ambiguousCount = 0;
    QList<MatchedCblEntry> matchedEntries;
    matchedEntries.reserve(count);

    matchingProgress.setLabelText(tr("Matching comics... 0 of %1").arg(count));
    matchingProgress.setRange(0, qMax(count, 1));
    matchingProgress.setValue(0);

    for (int i = 0; i < count; ++i) {
        const auto &book = result.readingList.books.at(i);
        const auto match = matchBook(book, libraryComics, remaps, &matchingProgress);
        matchedEntries.append({ book, match });

        if (match.state == CblMatchState::Matched)
            ++matchedCount;
        else if (match.state == CblMatchState::Ambiguous)
            ++ambiguousCount;
        else
            ++missingCount;

        matchingProgress.setValue(i + 1);
        matchingProgress.setLabelText(tr("Matching comics... %1 of %2").arg(i + 1).arg(count));
        if ((i + 1) % 25 == 0 || i + 1 == count)
            QApplication::processEvents();
        if (matchingProgress.wasCanceled()) {
            matchingProgress.close();
            return;
        }
    }

    matchingProgress.close();

    if (!reviewMatches(dialogParent, result.readingList.name, &matchedEntries, libraryComics, tr("Import")))
        return;

    matchedCount = missingCount = ambiguousCount = 0;
    for (const auto &entry : std::as_const(matchedEntries)) {
        if (entry.match.state == CblMatchState::Matched)
            ++matchedCount;
        else if (entry.match.state == CblMatchState::Ambiguous)
            ++ambiguousCount;
        else
            ++missingCount;
    }

    QString importError;
    qulonglong readingListId = 0;
    const auto selectedList = currentListProvider();
    const qulonglong parentReadingListId = listsModel->isReadingListFolder(selectedList)
            ? selectedList.data(ReadingListModel::IDRole).toULongLong() : 0;
    QString sourceHash;
    QFile sourceFile(filePath);
    if (sourceFile.open(QIODevice::ReadOnly))
        sourceHash = QString::fromLatin1(QCryptographicHash::hash(sourceFile.readAll(), QCryptographicHash::Sha256).toHex());
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
                                  QFileInfo(filePath).canonicalFilePath(),
                                  sourceHash,
                                  parentReadingListId,
                                  updateReadingListId,
                                  createCopy,
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

void ReadingListManagementCoordinator::exportReadingList()
{
    const auto currentList = currentListProvider();
    if (!currentList.isValid() || !listsModel->isReadingList(currentList)) {
        QMessageBox::information(dialogParent,
                                 tr("Export reading list"),
                                 tr("Select a reading list to export."));
        return;
    }

    const auto readingListId = currentList.data(ReadingListModel::IDRole).toULongLong();
    const auto readingListName = listsModel->name(currentList);
    const bool exportingFolder = listsModel->isReadingListFolder(currentList);
    bool separateFiles = false;
    if (exportingFolder) {
        bool accepted = false;
        const auto mode = QInputDialog::getItem(dialogParent,
                                                 tr("Export reading-list folder"),
                                                 tr("Export format:"),
                                                 { tr("One combined CBL"), tr("Separate CBL files") },
                                                 0,
                                                 false,
                                                 &accepted);
        if (!accepted)
            return;
        separateFiles = mode == tr("Separate CBL files");
    }
    QList<CblBook> books;
    QMap<QString, QList<CblBook>> booksByList;
    QString exportError;
    QString connectionName;
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(listsModel->databasePath());
        connectionName = db.connectionName();
        DBHelper::ensureReadingListEntries(db);
        ensureCblImportTables(db, &exportError);

        bool importedCbl = false;
        QSqlQuery tableCheck(db);
        if (tableCheck.exec(QStringLiteral("SELECT 1 FROM sqlite_master WHERE type = 'table' AND name = 'cbl_reading_list_meta'"))
            && tableCheck.next()) {
            QSqlQuery importedCheck(db);
            importedCheck.prepare(QStringLiteral("SELECT 1 FROM cbl_reading_list_meta WHERE reading_list_id = :id"));
            importedCheck.bindValue(QStringLiteral(":id"), readingListId);
            importedCbl = importedCheck.exec() && importedCheck.next();
        }

        QSqlQuery entries(db);
        if (exportingFolder) {
            entries.prepare(QStringLiteral(
                    "SELECT series, number, volume, year, format, file_name, source_id, comicvine_series_id, comicvine_issue_id, list_name FROM ("
                    "SELECT rl.ordering list_order, e.ordering entry_order, e.series, e.number, e.volume, e.year, e.format, e.file_name, "
                    "e.source_id, e.comicvine_series_id, e.comicvine_issue_id, rl.name list_name FROM reading_list rl "
                    "INNER JOIN cbl_reading_list_meta m ON m.reading_list_id = rl.id "
                    "INNER JOIN cbl_reading_list_entry e ON e.reading_list_id = rl.id WHERE rl.parentId = :imported_folder_id "
                    "UNION ALL "
                    "SELECT rl.ordering, e.ordering, COALESCE(ci.series,e.series), COALESCE(ci.number,e.number), COALESCE(ci.volume,e.volume), "
                    "substr(COALESCE(ci.date,e.year),-4), COALESCE(ci.format,e.format), COALESCE(c.fileName,e.file_name), '', '', "
                    "COALESCE(ci.comicVineID,e.comicvine_issue_id), rl.name FROM reading_list rl "
                    "INNER JOIN reading_list_entry e ON e.reading_list_id = rl.id "
                    "LEFT JOIN comic c ON c.id = e.comic_id LEFT JOIN comic_info ci ON ci.id = c.comicInfoId "
                    "WHERE rl.parentId = :regular_folder_id AND NOT EXISTS (SELECT 1 FROM cbl_reading_list_meta m WHERE m.reading_list_id = rl.id)) "
                    "ORDER BY list_order, entry_order"));
        } else if (importedCbl) {
            entries.prepare(QStringLiteral(
                    "SELECT series, number, volume, year, format, file_name, source_id, "
                    "comicvine_series_id, comicvine_issue_id "
                    "FROM cbl_reading_list_entry WHERE reading_list_id = :id ORDER BY ordering"));
        } else {
            DBHelper::ensureReadingListEntries(db);
            entries.prepare(QStringLiteral(
                    "SELECT COALESCE(ci.series,e.series), COALESCE(ci.number,e.number), COALESCE(ci.volume,e.volume), "
                    "substr(COALESCE(ci.date,e.year),-4), COALESCE(ci.format,e.format), COALESCE(c.fileName,e.file_name), "
                    "'', '', COALESCE(ci.comicVineID,e.comicvine_issue_id) "
                    "FROM reading_list_entry e "
                    "LEFT JOIN comic c ON c.id = e.comic_id "
                    "LEFT JOIN comic_info ci ON ci.id = c.comicInfoId "
                    "WHERE e.reading_list_id = :id ORDER BY e.ordering"));
        }
        if (exportingFolder) {
            entries.bindValue(QStringLiteral(":imported_folder_id"), readingListId);
            entries.bindValue(QStringLiteral(":regular_folder_id"), readingListId);
        } else {
            entries.bindValue(QStringLiteral(":id"), readingListId);
        }

        if (!entries.exec()) {
            exportError = entries.lastError().text();
        } else {
            while (entries.next()) {
                CblBook book;
                book.series = entries.value(0).toString();
                book.number = entries.value(1).toString();
                book.volume = entries.value(2).toString();
                const auto storedDate = entries.value(3).toString();
                book.year = storedDate;
                book.format = entries.value(4).toString();
                book.fileName = entries.value(5).toString();
                book.id = entries.value(6).toString();
                book.comicVineSeriesId = entries.value(7).toString();
                book.comicVineIssueId = entries.value(8).toString();
                books.append(book);
                if (exportingFolder)
                    booksByList[entries.value(9).toString()].append(book);
            }
        }
    }
    QSqlDatabase::removeDatabase(connectionName);

    if (!exportError.isEmpty()) {
        QMessageBox::critical(dialogParent, tr("Unable to export reading list"), exportError);
        return;
    }

    if (separateFiles) {
        const auto directory = QFileDialog::getExistingDirectory(dialogParent, tr("Export reading lists"));
        if (directory.isEmpty())
            return;
        for (auto it = booksByList.cbegin(); it != booksByList.cend(); ++it) {
            QString error;
            const auto path = QDir(directory).filePath(safeFileName(it.key()) + QStringLiteral(".cbl"));
            if (!writeCblFile(path, it.key(), it.value(), &error)) {
                QMessageBox::critical(dialogParent, tr("Unable to export reading lists"), tr("%1\n\n%2").arg(path, error));
                return;
            }
        }
        QMessageBox::information(dialogParent,
                                 tr("Reading lists exported"),
                                 tr("%1 reading lists were exported to:\n%2").arg(booksByList.size()).arg(directory));
        return;
    }

    const QString safeName = safeFileName(readingListName);
    QString filePath = QFileDialog::getSaveFileName(dialogParent,
                                                     tr("Export reading list"),
                                                     safeName + QStringLiteral(".cbl"),
                                                     tr("Comic Book Reading Lists (*.cbl)"));
    if (filePath.isEmpty())
        return;
    if (!filePath.endsWith(QStringLiteral(".cbl"), Qt::CaseInsensitive))
        filePath += QStringLiteral(".cbl");

    QString writeError;
    if (!writeCblFile(filePath, readingListName, books, &writeError)) {
        QMessageBox::critical(dialogParent,
                              tr("Unable to export reading list"),
                              writeError);
        return;
    }

    QMessageBox::information(dialogParent,
                             tr("Reading list exported"),
                             tr("%1 comics were exported to:\n%2").arg(books.size()).arg(filePath));
}

void ReadingListManagementCoordinator::showMissingComics()
{
    const auto currentList = currentListProvider();
    if (!currentList.isValid() || !listsModel->isReadingList(currentList)) {
        QMessageBox::information(dialogParent,
                                 tr("Missing comics"),
                                 tr("Select a reading list first."));
        return;
    }

    const auto readingListId = currentList.data(ReadingListModel::IDRole).toULongLong();
    const auto readingListName = listsModel->name(currentList);
    const bool folderReport = listsModel->isReadingListFolder(currentList);
    QList<MissingComicEntry> missingComics;
    QString loadError;
    QString connectionName;
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(listsModel->databasePath());
        connectionName = db.connectionName();
        DBHelper::ensureReadingListEntries(db);
        ensureCblImportTables(db, &loadError);

        bool importedCbl = false;
        QSqlQuery tableCheck(db);
        if (tableCheck.exec(QStringLiteral("SELECT 1 FROM sqlite_master WHERE type = 'table' AND name = 'cbl_reading_list_meta'"))
            && tableCheck.next()) {
            QSqlQuery importedCheck(db);
            importedCheck.prepare(QStringLiteral("SELECT 1 FROM cbl_reading_list_meta WHERE reading_list_id = :id"));
            importedCheck.bindValue(QStringLiteral(":id"), readingListId);
            importedCbl = importedCheck.exec() && importedCheck.next();
        }

        QSqlQuery entries(db);
        if (folderReport) {
            entries.prepare(QStringLiteral(
                    "SELECT series, number, volume, year, format, file_name, source_id, comicvine_series_id, comicvine_issue_id FROM ("
                    "SELECT rl.ordering list_order, e.ordering entry_order, e.series, e.number, e.volume, e.year, e.format, e.file_name, "
                    "e.source_id, e.comicvine_series_id, e.comicvine_issue_id FROM reading_list rl "
                    "INNER JOIN cbl_reading_list_meta m ON m.reading_list_id = rl.id "
                    "INNER JOIN cbl_reading_list_entry e ON e.reading_list_id = rl.id "
                    "WHERE rl.parentId = :imported_folder_id AND e.comic_id IS NULL "
                    "UNION ALL "
                    "SELECT rl.ordering, e.ordering, e.series, e.number, e.volume, e.year, e.format, e.file_name, '', '', e.comicvine_issue_id "
                    "FROM reading_list rl INNER JOIN reading_list_entry e ON e.reading_list_id = rl.id "
                    "WHERE rl.parentId = :regular_folder_id AND e.comic_id IS NULL "
                    "AND NOT EXISTS (SELECT 1 FROM cbl_reading_list_meta m WHERE m.reading_list_id = rl.id) "
                    "ORDER BY list_order, entry_order"));
        } else if (importedCbl) {
            entries.prepare(QStringLiteral(
                    "SELECT series, number, volume, year, format, file_name, source_id, "
                    "comicvine_series_id, comicvine_issue_id "
                    "FROM cbl_reading_list_entry WHERE reading_list_id = :id AND comic_id IS NULL ORDER BY ordering"));
        } else {
            DBHelper::ensureReadingListEntries(db);
            entries.prepare(QStringLiteral(
                    "SELECT series, number, volume, year, format, file_name, '', '', comicvine_issue_id "
                    "FROM reading_list_entry WHERE reading_list_id = :id AND comic_id IS NULL ORDER BY ordering"));
        }
        if (folderReport) {
            entries.bindValue(QStringLiteral(":imported_folder_id"), readingListId);
            entries.bindValue(QStringLiteral(":regular_folder_id"), readingListId);
        } else {
            entries.bindValue(QStringLiteral(":id"), readingListId);
        }

        if (!entries.exec()) {
            loadError = entries.lastError().text();
        } else {
            while (entries.next()) {
                MissingComicEntry comic;
                comic.series = entries.value(0).toString();
                comic.number = entries.value(1).toString();
                comic.volume = entries.value(2).toString();
                comic.year = entries.value(3).toString();
                comic.format = entries.value(4).toString();
                comic.fileName = entries.value(5).toString();
                comic.sourceId = entries.value(6).toString();
                comic.comicVineSeriesId = entries.value(7).toString();
                comic.comicVineIssueId = entries.value(8).toString();
                missingComics.append(comic);
            }
        }
    }
    QSqlDatabase::removeDatabase(connectionName);

    if (!loadError.isEmpty()) {
        QMessageBox::critical(dialogParent, tr("Unable to load missing comics"), loadError);
        return;
    }

    QDialog dialog(dialogParent);
    dialog.setWindowTitle(tr("Missing comics — %1").arg(readingListName));
    dialog.resize(1150, 650);
    auto *layout = new QVBoxLayout(&dialog);
    layout->addWidget(new QLabel(tr("%1 missing comics in %2")
                                         .arg(missingComics.size())
                                         .arg(readingListName),
                                 &dialog));

    const QStringList headers { tr("Series"), tr("Issue"), tr("Volume"), tr("Year"), tr("Format"),
                                tr("File name"), tr("Source ID"), tr("ComicVine series"), tr("ComicVine issue") };
    auto *table = new QTableWidget(missingComics.size(), headers.size(), &dialog);
    table->setHorizontalHeaderLabels(headers);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSortingEnabled(false);
    for (int row = 0; row < missingComics.size(); ++row) {
        const auto &comic = missingComics.at(row);
        const QStringList values { comic.series, comic.number, comic.volume, comic.year, comic.format,
                                   comic.fileName, comic.sourceId, comic.comicVineSeriesId, comic.comicVineIssueId };
        for (int column = 0; column < values.size(); ++column)
            table->setItem(row, column, new QTableWidgetItem(values.at(column)));
    }
    table->setSortingEnabled(true);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    table->horizontalHeader()->setStretchLastSection(true);
    layout->addWidget(table);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close, &dialog);
    auto *matchAll = buttons->addButton(tr("Match all again"), QDialogButtonBox::ActionRole);
    auto *exportPdf = buttons->addButton(tr("Export PDF..."), QDialogButtonBox::ActionRole);
    matchAll->setEnabled(!missingComics.isEmpty());
    exportPdf->setEnabled(!missingComics.isEmpty());
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    connect(matchAll, &QPushButton::clicked, &dialog, [&] {
        QString relinkConnection;
        int relinked = 0;
        {
            QSqlDatabase db = DataBaseManagement::loadDatabase(listsModel->databasePath());
            relinkConnection = db.connectionName();
            if (folderReport) {
                QSqlQuery children(db);
                children.prepare(QStringLiteral("SELECT id FROM reading_list WHERE parentId = :id"));
                children.bindValue(QStringLiteral(":id"), readingListId);
                if (children.exec()) {
                    while (children.next())
                        relinked += DBHelper::relinkMissingReadingListEntries(db, children.value(0).toULongLong());
                }
            } else {
                relinked = DBHelper::relinkMissingReadingListEntries(db, readingListId);
            }
        }
        QSqlDatabase::removeDatabase(relinkConnection);
        listsModel->setupReadingListsData(listsModel->databasePath());
        emit currentListReselectionRequested();
        QMessageBox::information(&dialog,
                                 tr("Matching complete"),
                                 tr("%1 missing comics were safely relinked.").arg(relinked));
        if (relinked > 0) {
            dialog.accept();
            QTimer::singleShot(0, this, &ReadingListManagementCoordinator::showMissingComics);
        }
    });
    connect(exportPdf, &QPushButton::clicked, &dialog, [&] {
        QString safeName = readingListName;
        safeName.replace(QRegularExpression(QStringLiteral("[<>:\"/\\|?*]")), QStringLiteral("_"));
        QString filePath = QFileDialog::getSaveFileName(&dialog,
                                                        tr("Export missing comics report"),
                                                        safeName + tr(" - Missing Comics.pdf"),
                                                        tr("PDF documents (*.pdf)"));
        if (filePath.isEmpty())
            return;
        if (!filePath.endsWith(QStringLiteral(".pdf"), Qt::CaseInsensitive))
            filePath += QStringLiteral(".pdf");

        QString html = QStringLiteral(
                "<html><head><style>body{font-family:sans-serif;font-size:9pt}h1{font-size:18pt}"
                "table{border-collapse:collapse;width:100%}th{background:#333;color:white}"
                "th,td{border:1px solid #888;padding:4px;text-align:left}tr:nth-child(even){background:#eee}"
                "</style></head><body><h1>%1</h1><p>%2</p><table><thead><tr>")
                               .arg(tr("Missing comics — %1").arg(readingListName).toHtmlEscaped(),
                                    tr("%1 missing comics • Generated %2")
                                            .arg(missingComics.size())
                                            .arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd HH:mm")))
                                            .toHtmlEscaped());
        for (const auto &header : headers)
            html += QStringLiteral("<th>%1</th>").arg(header.toHtmlEscaped());
        html += QStringLiteral("</tr></thead><tbody>");
        for (const auto &comic : std::as_const(missingComics)) {
            const QStringList values { comic.series, comic.number, comic.volume, comic.year, comic.format,
                                       comic.fileName, comic.sourceId, comic.comicVineSeriesId, comic.comicVineIssueId };
            html += QStringLiteral("<tr>");
            for (const auto &value : values)
                html += QStringLiteral("<td>%1</td>").arg(value.toHtmlEscaped());
            html += QStringLiteral("</tr>");
        }
        html += QStringLiteral("</tbody></table></body></html>");

        QPdfWriter writer(filePath);
        writer.setTitle(tr("Missing comics — %1").arg(readingListName));
        writer.setCreator(QStringLiteral("YACReaderLibrary"));
        writer.setPageSize(QPageSize(QPageSize::Letter));
        writer.setPageOrientation(QPageLayout::Landscape);
        writer.setPageMargins(QMarginsF(10, 10, 10, 10), QPageLayout::Millimeter);
        writer.setResolution(144);

        QTextDocument document;
        document.setHtml(html);
        document.setPageSize(writer.pageLayout().paintRectPixels(writer.resolution()).size());
        document.print(&writer);

        QMessageBox::information(&dialog,
                                 tr("Missing comics report exported"),
                                 tr("The PDF report was saved to:\n%1").arg(filePath));
    });
    layout->addWidget(buttons);
    dialog.exec();
}

void ReadingListManagementCoordinator::editCblReadingList()
{
    const auto currentList = currentListProvider();
    if (!currentList.isValid()
        || currentList.data(ReadingListModel::TypeListsRole).toInt() != ReadingListModel::ReadingList) {
        QMessageBox::information(dialogParent, tr("Edit CBL reading list"), tr("Select an imported CBL reading list first."));
        return;
    }

    const qulonglong readingListId = currentList.data(ReadingListModel::IDRole).toULongLong();
    QList<LibraryComicMatchData> libraryComics;
    QHash<qulonglong, LibraryComicMatchData> comicsById;
    QList<MatchedCblEntry> entries;
    CblReadingList readingList;
    readingList.name = listsModel->name(currentList);
    QString sourceName;
    QString sourcePath;
    QString sourceHash;
    QString loadError;
    QString connectionName;

    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(listsModel->databasePath());
        connectionName = db.connectionName();
        if (!ensureCblImportTables(db, &loadError)) {
            // Reported below.
        } else {
            QSqlQuery meta(db);
            meta.prepare(QStringLiteral("SELECT source_name, source_path, source_hash FROM cbl_reading_list_meta WHERE reading_list_id = :id"));
            meta.bindValue(QStringLiteral(":id"), readingListId);
            if (!meta.exec()) {
                loadError = meta.lastError().text();
            } else if (!meta.next()) {
                loadError = tr("This is a regular YACReader reading list, not an imported CBL reading list.");
            } else {
                sourceName = meta.value(0).toString();
                sourcePath = meta.value(1).toString();
                sourceHash = meta.value(2).toString();
            }
        }

        if (loadError.isEmpty()) {
            QSqlQuery comics(db);
            if (!comics.exec(QStringLiteral("SELECT c.id, c.fileName, ci.series, ci.number, ci.volume, ci.comicVineID, ci.date, ci.format, ci.alternateSeries "
                                             "FROM comic c INNER JOIN comic_info ci ON c.comicInfoId = ci.id"))) {
                loadError = comics.lastError().text();
            } else {
                while (comics.next()) {
                    LibraryComicMatchData comic;
                    comic.id = comics.value(0).toULongLong();
                    comic.fileName = comics.value(1).toString();
                    comic.series = comics.value(2).toString();
                    comic.number = comics.value(3).toString();
                    comic.volume = comics.value(4).toString();
                    comic.comicVineIssueId = comics.value(5).toString();
                    comic.year = comics.value(6).toString().right(4);
                    comic.format = comics.value(7).toString();
                    comic.alternateSeries = comics.value(8).toString();
                    libraryComics.append(comic);
                    comicsById.insert(comic.id, comic);
                }
            }
        }

        if (loadError.isEmpty()) {
            QSqlQuery storedEntries(db);
            storedEntries.prepare(QStringLiteral(
                    "SELECT ordering, comic_id, series, number, volume, year, format, file_name, source_id, "
                    "comicvine_series_id, comicvine_issue_id, match_state, match_tier "
                    "FROM cbl_reading_list_entry WHERE reading_list_id = :id ORDER BY ordering"));
            storedEntries.bindValue(QStringLiteral(":id"), readingListId);
            if (!storedEntries.exec()) {
                loadError = storedEntries.lastError().text();
            } else {
                while (storedEntries.next()) {
                    MatchedCblEntry entry;
                    entry.book.ordering = storedEntries.value(0).toInt();
                    const auto comicId = storedEntries.value(1).toULongLong();
                    entry.book.series = storedEntries.value(2).toString();
                    entry.book.number = storedEntries.value(3).toString();
                    entry.book.volume = storedEntries.value(4).toString();
                    entry.book.year = storedEntries.value(5).toString();
                    entry.book.format = storedEntries.value(6).toString();
                    entry.book.fileName = storedEntries.value(7).toString();
                    entry.book.id = storedEntries.value(8).toString();
                    entry.book.comicVineSeriesId = storedEntries.value(9).toString();
                    entry.book.comicVineIssueId = storedEntries.value(10).toString();
                    entry.match.state = static_cast<CblMatchState>(storedEntries.value(11).toInt());
                    entry.match.tier = static_cast<CblMatchTier>(storedEntries.value(12).toInt());
                    if (comicId != 0 && comicsById.contains(comicId)) {
                        entry.match.state = CblMatchState::Matched;
                        entry.match.candidates = { comicsById.value(comicId) };
                    } else if (entry.match.state == CblMatchState::Matched) {
                        entry.match.state = CblMatchState::Missing;
                        entry.match.tier = CblMatchTier::None;
                    }
                    entries.append(entry);
                }
            }
        }
    }
    QSqlDatabase::removeDatabase(connectionName);

    if (!loadError.isEmpty()) {
        QMessageBox::information(dialogParent, tr("Edit CBL reading list"), loadError);
        return;
    }
    if (!reviewMatches(dialogParent, readingList.name, &entries, libraryComics, tr("Save changes")))
        return;

    QString saveError;
    qulonglong savedId = 0;
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(listsModel->databasePath());
        connectionName = db.connectionName();
        persistCblReadingList(db, readingList, entries, sourceName, sourcePath, sourceHash, 0, readingListId, false, &savedId, &saveError);
    }
    QSqlDatabase::removeDatabase(connectionName);

    if (!saveError.isEmpty() || savedId == 0) {
        QMessageBox::critical(dialogParent, tr("Unable to save reading list"), saveError);
        return;
    }

    listsModel->setupReadingListsData(listsModel->databasePath());
    emit currentListReselectionRequested();
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
