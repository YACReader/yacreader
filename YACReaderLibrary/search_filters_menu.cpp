#include "search_filters_menu.h"

#include "search_syntax_dialog.h"
#include "yacreader_global_gui.h"

#include <QCoreApplication>
#include <QMenu>
#include <QSettings>

QMenu *createSearchFiltersMenu(QWidget *parent, QSettings *settings, const std::function<void(const QString &)> &apply)
{
    auto *menu = new QMenu(QCoreApplication::translate("LibraryWindow", "Search filters"), parent);
    menu->setMinimumWidth(190);

    auto addFilter = [menu, apply](const QString &label, const QString &query) {
        auto *action = menu->addAction(label);
        QObject::connect(action, &QAction::triggered, menu, [apply, query] { apply(query); });
    };

    addFilter(QCoreApplication::translate("LibraryWindow", "Unread"), QStringLiteral("read:false"));
    addFilter(QCoreApplication::translate("LibraryWindow", "In progress"), QStringLiteral("hasBeenOpened:true AND read:false"));
    addFilter(QCoreApplication::translate("LibraryWindow", "Highly rated"), QStringLiteral("rating>=4"));

    auto *recentlyAdded = menu->addAction(QCoreApplication::translate("LibraryWindow", "Recently added"));
    QObject::connect(recentlyAdded, &QAction::triggered, menu, [settings, apply] {
        const int days = settings->value(NUM_DAYS_TO_CONSIDER_RECENT, 1).toInt();
        apply(QStringLiteral("added>%1").arg(days));
    });

    menu->addSeparator();
    auto *syntaxAction = menu->addAction(QCoreApplication::translate("LibraryWindow", "Search syntax…"));
    QObject::connect(syntaxAction, &QAction::triggered, menu, [parent] {
        auto *dialog = new SearchSyntaxDialog(parent);
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        dialog->open();
    });

    return menu;
}
