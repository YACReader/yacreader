#include "smart_list_dialog.h"

#include "query_parser.h"
#include "search_filters_menu.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QMenu>
#include <QPushButton>
#include <QVBoxLayout>

namespace {

bool canBeParsed(const QString &query)
{
    if (query.isEmpty())
        return false;
    try {
        QueryParser().parse(query.toStdString());
        return true;
    } catch (const std::exception &) {
        return false;
    }
}

}

SmartListDialog::SmartListDialog(QSettings *settings, const QString &name, const QString &query, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(name.isEmpty() ? tr("New smart list") : tr("Edit smart list"));

    nameEdit = new QLineEdit(name);
    searchEdit = new QLineEdit(query);
    searchEdit->setMinimumWidth(360);
    searchEdit->setClearButtonEnabled(true);
    auto filtersMenu = createSearchFiltersMenu(this, settings, [this](const QString &filter) {
        searchEdit->setText(filter);
        searchEdit->setFocus(Qt::ShortcutFocusReason);
    });
    // a menu button with no text: the style draws the frame and its menu arrow, like the other buttons
    auto filtersButton = new QPushButton;
    filtersButton->setToolTip(tr("Search filters and syntax"));
    filtersButton->setAutoDefault(false);
    filtersButton->setMenu(filtersMenu);
    filtersButton->setFixedWidth(filtersButton->sizeHint().height() + 8);

    auto searchRow = new QHBoxLayout;
    searchRow->setSpacing(2);
    searchRow->addWidget(searchEdit, 1);
    searchRow->addWidget(filtersButton);

    auto form = new QFormLayout;
    form->addRow(tr("Name:"), nameEdit);
    form->addRow(tr("Search:"), searchRow);

    auto buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel);
    saveButton = buttons->button(QDialogButtonBox::Save);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto layout = new QVBoxLayout(this);
    layout->addLayout(form);
    layout->addWidget(buttons);

    connect(searchEdit, &QLineEdit::textChanged, this, [this] {
        updateSaveButton();
        emit searchChanged(this->query());
    });
    connect(nameEdit, &QLineEdit::textChanged, this, &SmartListDialog::updateSaveButton);
    updateSaveButton();
}

QString SmartListDialog::name() const
{
    return nameEdit->text().trimmed();
}

QString SmartListDialog::query() const
{
    return searchEdit->text().trimmed();
}

// a smart list never stores a search that cannot be read
void SmartListDialog::updateSaveButton()
{
    saveButton->setEnabled(!name().isEmpty() && canBeParsed(query()));
}
