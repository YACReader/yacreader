#include "search_syntax_dialog.h"

#include "search_field_registry.h"

#include <QApplication>
#include <QClipboard>
#include <QDialogButtonBox>
#include <QFontDatabase>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QTabWidget>
#include <QTreeView>
#include <QTreeWidget>
#include <QVBoxLayout>

namespace {

QLabel *codeLabel(const QString &text, QWidget *parent = nullptr)
{
    auto *label = new QLabel(text, parent);
    label->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    label->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    label->setMargin(6);
    label->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
    return label;
}

QWidget *guideStep(
        const QString &title,
        const QString &description,
        const QString &example,
        QWidget *parent)
{
    auto *group = new QGroupBox(title, parent);
    group->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
    auto *layout = new QVBoxLayout(group);
    auto *descriptionLabel = new QLabel(description, group);
    descriptionLabel->setWordWrap(true);
    layout->addWidget(descriptionLabel);
    layout->addWidget(codeLabel(example, group));
    return group;
}

void configureReferenceLayout(QFormLayout *layout)
{
    layout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    layout->setRowWrapPolicy(QFormLayout::WrapLongRows);
    layout->setFormAlignment(Qt::AlignLeft | Qt::AlignTop);
    layout->setLabelAlignment(Qt::AlignLeft | Qt::AlignTop);
}

void addFormRow(QFormLayout *layout, const QString &syntax, const QString &description)
{
    auto *syntaxLabel = new QLabel(syntax);
    syntaxLabel->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    syntaxLabel->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);

    auto *descriptionLabel = new QLabel(description);
    descriptionLabel->setWordWrap(true);
    layout->addRow(syntaxLabel, descriptionLabel);
}

QString categoryName(SearchFieldCategory category)
{
    switch (category) {
    case SearchFieldCategory::Common:
        return SearchSyntaxDialog::tr("Common");
    case SearchFieldCategory::Credits:
        return SearchSyntaxDialog::tr("Credits");
    case SearchFieldCategory::Story:
        return SearchSyntaxDialog::tr("Story");
    case SearchFieldCategory::Publication:
        return SearchSyntaxDialog::tr("Publication");
    case SearchFieldCategory::ReadingAndFiles:
        return SearchSyntaxDialog::tr("Reading & files");
    case SearchFieldCategory::Folders:
        return SearchSyntaxDialog::tr("Folders");
    }

    return { };
}

QList<SearchFieldCategory> fieldCategories()
{
    return {
        SearchFieldCategory::Common,
        SearchFieldCategory::Credits,
        SearchFieldCategory::Story,
        SearchFieldCategory::Publication,
        SearchFieldCategory::ReadingAndFiles,
        SearchFieldCategory::Folders
    };
}

QList<QStandardItem *> fieldRow(const SearchFieldDefinition &field)
{
    auto item = [](const QString &text) {
        auto *standardItem = new QStandardItem(text);
        standardItem->setEditable(false);
        standardItem->setToolTip(text);
        return standardItem;
    };

    auto *key = item(field.key);
    key->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

    auto *example = item(field.example);
    example->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

    return {
        key,
        item(field.description),
        item(field.input),
        example
    };
}

void addExample(
        QTreeWidgetItem *category,
        const QString &query,
        const QString &description)
{
    auto *item = new QTreeWidgetItem(category, { query, description });
    item->setFont(0, QFontDatabase::systemFont(QFontDatabase::FixedFont));
    item->setData(0, Qt::UserRole, query);
}

}

SearchSyntaxDialog::SearchSyntaxDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Search syntax"));
    setWindowModality(Qt::WindowModal);
    setMinimumSize(760, 480);
    resize(900, 560);

    auto *subtitle = new QLabel(
            tr("Search every comic and folder field, or build precise queries."),
            this);
    subtitle->setWordWrap(true);

    auto *tabs = new QTabWidget(this);
    tabs->addTab(createQuickGuideTab(), tr("Quick guide"));
    tabs->addTab(
            createFieldsTab(),
            tr("Fields (%1)").arg(searchFieldDefinitions().size()));
    tabs->addTab(createExamplesTab(), tr("Examples"));

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::close);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(subtitle);
    layout->addWidget(tabs, 1);
    layout->addWidget(buttons);
}

QWidget *SearchSyntaxDialog::createQuickGuideTab()
{
    auto *tab = new QWidget(this);
    auto *layout = new QVBoxLayout(tab);

    auto *plainSearch = new QGroupBox(tr("Start with a simple search"), tab);
    plainSearch->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
    auto *plainSearchLayout = new QVBoxLayout(plainSearch);
    auto *plainSearchDescription = new QLabel(
            tr("Just start typing. Plain text search across all metadata."),
            plainSearch);
    plainSearchDescription->setWordWrap(true);
    plainSearchLayout->addWidget(plainSearchDescription);
    plainSearchLayout->addWidget(codeLabel(QStringLiteral("san -> searchs `san` in any field of the database"), plainSearch));
    layout->addWidget(plainSearch);

    auto *steps = new QHBoxLayout();
    steps->addWidget(guideStep(
                             tr("1. Search everywhere"),
                             tr("Type any text or quoted text."),
                             QStringLiteral("\"hidden kingdom\""),
                             tab),
                     1,
                     Qt::AlignTop);
    steps->addWidget(guideStep(
                             tr("2. Target a field"),
                             tr("Use a field name followed by : or ="),
                             QStringLiteral("writer:Smith"),
                             tab),
                     1,
                     Qt::AlignTop);
    steps->addWidget(guideStep(
                             tr("3. Combine conditions"),
                             tr("Use AND, OR, NOT and parentheses."),
                             QStringLiteral("read:false AND rating>=4"),
                             tab),
                     1,
                     Qt::AlignTop);
    layout->addLayout(steps);

    auto *reference = new QHBoxLayout();

    auto *operators = new QGroupBox(tr("Operators"), tab);
    operators->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    auto *operatorsLayout = new QFormLayout(operators);
    configureReferenceLayout(operatorsLayout);
    addFormRow(operatorsLayout, tr(": or ="), tr("contains the text"));
    addFormRow(operatorsLayout, QStringLiteral("=="), tr("matches the complete value"));
    addFormRow(operatorsLayout, QStringLiteral(">  >="), tr("greater than / at least"));
    addFormRow(operatorsLayout, QStringLiteral("<  <="), tr("less than / at most"));
    addFormRow(operatorsLayout, tr("\"quoted text\""), tr("keeps spaces inside one value"));
    reference->addWidget(operators, 1);

    auto *dates = new QGroupBox(tr("Dates and grouping"), tab);
    dates->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    auto *datesLayout = new QFormLayout(dates);
    configureReferenceLayout(datesLayout);
    addFormRow(datesLayout, QStringLiteral("added>7"), tr("added in the last 7 days"));
    addFormRow(datesLayout, QStringLiteral("added<30"), tr("added more than 30 days ago"));
    addFormRow(
            datesLayout,
            QStringLiteral("(writer:Smith OR writer:Jones)"),
            tr("group alternatives"));
    reference->addWidget(dates, 1);

    layout->addLayout(reference);

    auto *tip = new QLabel(
            tr("Tips:\nSpaces act like AND, and searches are not case-sensitive.\nUse quotes to include spaces in a value."),
            tab);
    tip->setWordWrap(true);
    layout->addWidget(tip);
    layout->addStretch();

    return tab;
}

QWidget *SearchSyntaxDialog::createFieldsTab()
{
    auto *tab = new QWidget(this);
    auto *layout = new QVBoxLayout(tab);

    auto *filterEdit = new QLineEdit(tab);
    filterEdit->setClearButtonEnabled(true);
    filterEdit->setPlaceholderText(tr("Find a field…"));
    layout->addWidget(filterEdit);

    auto *help = new QLabel(
            tr("Text can be entered plainly or in quotes. Integer fields support <, <=, > and >=. "
               "For date fields, the integer is a number of days (added>7 means added within the last 7 days)."),
            tab);
    help->setTextFormat(Qt::PlainText);
    help->setWordWrap(true);
    layout->addWidget(help);

    auto *sourceModel = new QStandardItemModel(tab);
    sourceModel->setHorizontalHeaderLabels({ tr("Field"),
                                             tr("Description"),
                                             tr("Input"),
                                             tr("Example") });

    const auto categories = fieldCategories();
    for (const auto category : categories) {
        QList<QStandardItem *> categoryRow {
            new QStandardItem(categoryName(category)),
            new QStandardItem(),
            new QStandardItem(),
            new QStandardItem()
        };
        for (auto *item : categoryRow)
            item->setEditable(false);

        auto *categoryItem = categoryRow.first();
        QFont categoryFont = categoryItem->font();
        categoryFont.setBold(true);
        categoryItem->setFont(categoryFont);
        sourceModel->appendRow(categoryRow);

        for (const auto &field : searchFieldDefinitions()) {
            if (field.category == category)
                categoryItem->appendRow(fieldRow(field));
        }
    }

    auto *proxyModel = new QSortFilterProxyModel(tab);
    proxyModel->setSourceModel(sourceModel);
    proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxyModel->setFilterKeyColumn(-1);
    proxyModel->setRecursiveFilteringEnabled(true);

    auto *fieldsView = new QTreeView(tab);
    fieldsView->setModel(proxyModel);
    fieldsView->setAlternatingRowColors(true);
    fieldsView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    fieldsView->setSelectionBehavior(QAbstractItemView::SelectRows);
    fieldsView->setUniformRowHeights(true);
    fieldsView->setTextElideMode(Qt::ElideRight);
    auto *header = fieldsView->header();
    header->setStretchLastSection(false);
    header->setMinimumSectionSize(80);
    header->setSectionResizeMode(QHeaderView::Interactive);
    header->resizeSection(0, 165);
    header->resizeSection(1, 205);
    header->resizeSection(2, 250);
    header->resizeSection(3, 225);
    fieldsView->expandAll();
    layout->addWidget(fieldsView, 1);

    connect(filterEdit, &QLineEdit::textChanged, proxyModel, &QSortFilterProxyModel::setFilterFixedString);
    connect(filterEdit, &QLineEdit::textChanged, fieldsView, [fieldsView] {
        fieldsView->expandAll();
    });

    return tab;
}

QWidget *SearchSyntaxDialog::createExamplesTab()
{
    auto *tab = new QWidget(this);
    auto *layout = new QVBoxLayout(tab);

    auto *intro = new QLabel(
            tr("Examples show the pattern—replace the values with your own."),
            tab);
    layout->addWidget(intro);

    auto *examples = new QTreeWidget(tab);
    examples->setColumnCount(2);
    examples->setHeaderLabels({ tr("Query"), tr("What it finds") });
    examples->setAlternatingRowColors(true);
    examples->setSelectionBehavior(QAbstractItemView::SelectRows);
    examples->setContextMenuPolicy(Qt::CustomContextMenu);

    auto *common = new QTreeWidgetItem(examples, { tr("Common filters") });
    addExample(common, QStringLiteral("read:false"), tr("Unread comics"));
    addExample(common, QStringLiteral("hasBeenOpened:true AND read:false"), tr("Comics in progress"));
    addExample(common, QStringLiteral("rating>=4"), tr("Highly rated comics"));
    addExample(common, QStringLiteral("added>7"), tr("Comics added in the last 7 days"));

    auto *metadata = new QTreeWidgetItem(examples, { tr("Metadata") });
    addExample(metadata, QStringLiteral("series:\"Starfall Chronicles\""), tr("Search by series"));
    addExample(metadata, QStringLiteral("writer:Smith"), tr("Search by writer"));
    addExample(metadata, QStringLiteral("type:manga"), tr("Manga comics"));
    addExample(metadata, QStringLiteral("tags:\"to review\""), tr("Search textual tags"));

    auto *advanced = new QTreeWidgetItem(examples, { tr("Advanced combinations") });
    addExample(
            advanced,
            QStringLiteral("writer:Smith OR writer:Jones"),
            tr("Match either writer"));
    addExample(
            advanced,
            QStringLiteral("(publisher:ExamplePress OR publisher:StoryHouse) read:false"),
            tr("Group alternatives"));
    addExample(advanced, QStringLiteral("NOT format:annual"), tr("Exclude a value"));
    addExample(
            advanced,
            QStringLiteral("added<30 AND rating>=4"),
            tr("Older, highly rated comics"));

    examples->expandAll();
    examples->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    examples->header()->setSectionResizeMode(1, QHeaderView::Stretch);
    layout->addWidget(examples, 1);

    connect(examples, &QTreeWidget::customContextMenuRequested, examples, [examples, this](const QPoint &position) {
        auto *item = examples->itemAt(position);
        if (!item)
            return;

        const QString query = item->data(0, Qt::UserRole).toString();
        if (query.isEmpty())
            return;

        QMenu menu(examples);
        auto *copyAction = menu.addAction(tr("Copy query"));
        if (menu.exec(examples->viewport()->mapToGlobal(position)) == copyAction)
            QApplication::clipboard()->setText(query);
    });

    auto *note = new QLabel(
            tr("Spaces behave like AND. Use quotes for phrases and parentheses to control grouping."),
            tab);
    note->setWordWrap(true);
    layout->addWidget(note);

    return tab;
}
