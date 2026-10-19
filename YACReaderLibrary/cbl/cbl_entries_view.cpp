#include "cbl_entries_view.h"

#include "cbl_import_model.h"

#include <QApplication>
#include <QMouseEvent>
#include <QPainter>
#include <QStyledItemDelegate>

#include <algorithm>

namespace {

class CblEntriesDelegate : public QStyledItemDelegate
{
public:
    explicit CblEntriesDelegate(CblEntriesView *view)
        : QStyledItemDelegate(view), view(view)
    {
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        if (CblImportModel::isDetail(index))
            return;

        QStyledItemDelegate::paint(painter, option, index);
        if (index.column() != CblImportModel::ActionColumn || !view->isHovered(index))
            return;

        QStyleOptionButton button;
        button.initFrom(view);
        button.rect = view->findButtonRect(option.rect);
        button.text = view->findButtonText();
        button.state = QStyle::State_Enabled | QStyle::State_Raised;
        view->style()->drawControl(QStyle::CE_PushButton, &button, painter, view);
    }

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        if (CblImportModel::isDetail(index)) {
            const auto *widget = view->indexWidget(index.siblingAtColumn(0));
            return widget != nullptr ? QSize(0, widget->sizeHint().height()) : QSize(0, 0);
        }

        auto size = QStyledItemDelegate::sizeHint(option, index);
        // every row is as tall as the button, so the button fits on any row it shows on
        if (index.column() == CblImportModel::ActionColumn) {
            const auto button = view->findButtonRect(QRect(0, 0, 1000, 1000));
            size.setWidth(button.width() + 8);
            size.setHeight(std::max(size.height(), button.height() + 2));
        }
        return size;
    }

private:
    CblEntriesView *view;
};

}

CblEntriesView::CblEntriesView(QWidget *parent)
    : QTreeView(parent)
{
    setMouseTracking(true);
    setUniformRowHeights(false);
    setItemDelegate(new CblEntriesDelegate(this));
}

bool CblEntriesView::isHovered(const QModelIndex &index) const
{
    return hovered.isValid() && !CblImportModel::isDetail(index) && index.row() == hovered.row() && index.parent() == hovered.parent();
}

QString CblEntriesView::findButtonText() const
{
    return tr("Find comic ›");
}

QRect CblEntriesView::findButtonRect(const QRect &cell) const
{
    QStyleOptionButton button;
    button.initFrom(this);
    button.text = findButtonText();
    const auto textSize = fontMetrics().size(Qt::TextShowMnemonic, button.text);
    auto size = style()->sizeFromContents(QStyle::CT_PushButton, &button, textSize, this);
    size.setHeight(std::min(size.height(), cell.height() - 2));
    QRect rect(QPoint(0, 0), size);
    rect.moveCenter(cell.center());
    rect.moveRight(cell.right() - 4);
    return rect;
}

void CblEntriesView::drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const
{
    // the indentation of a detail would show the selection of its entry
    if (CblImportModel::isDetail(index)) {
        painter->fillRect(rect, palette().base());
        return;
    }
    if (isExpanded(index) || isHovered(index))
        QTreeView::drawBranches(painter, rect, index);
}

void CblEntriesView::mouseMoveEvent(QMouseEvent *event)
{
    const auto index = indexAt(event->position().toPoint());
    setHovered(CblImportModel::isDetail(index) ? QModelIndex() : index);
    QTreeView::mouseMoveEvent(event);
}

void CblEntriesView::mousePressEvent(QMouseEvent *event)
{
    const auto position = event->position().toPoint();
    const auto index = indexAt(position);
    if (event->button() == Qt::LeftButton && index.isValid() && !CblImportModel::isDetail(index) && index.column() == CblImportModel::ActionColumn &&
        findButtonRect(visualRect(index)).contains(position)) {
        emit findComicRequested(index);
        event->accept();
        return;
    }
    QTreeView::mousePressEvent(event);
}

void CblEntriesView::leaveEvent(QEvent *event)
{
    setHovered(QModelIndex());
    QTreeView::leaveEvent(event);
}

void CblEntriesView::setHovered(const QModelIndex &index)
{
    if (index.isValid() == hovered.isValid() && (!index.isValid() || (index.row() == hovered.row() && index.parent() == hovered.parent())))
        return;

    const auto previous = hovered;
    hovered = index.isValid() ? QPersistentModelIndex(index.siblingAtColumn(0)) : QPersistentModelIndex();
    // the branch area is outside the cells, so the whole rows are repainted
    for (const auto &row : { QModelIndex(previous), QModelIndex(hovered) }) {
        if (row.isValid()) {
            const auto rect = visualRect(row);
            viewport()->update(QRect(0, rect.top(), viewport()->width(), rect.height()));
        }
    }
}
