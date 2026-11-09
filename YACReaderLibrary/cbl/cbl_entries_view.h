#ifndef CBL_ENTRIES_VIEW_H
#define CBL_ENTRIES_VIEW_H

#include <QPersistentModelIndex>
#include <QTreeView>

// the entries of a CBL import: the expand arrow and the "Find comic" button show on the row under the mouse; a detail
// row shows its index widget across all columns
class CblEntriesView : public QTreeView
{
    Q_OBJECT

public:
    explicit CblEntriesView(QWidget *parent = nullptr);

    bool isHovered(const QModelIndex &index) const;
    QRect findButtonRect(const QRect &cell) const;
    QString findButtonText() const;

signals:
    void findComicRequested(const QModelIndex &index);

protected:
    void drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    void setHovered(const QModelIndex &index);

    QPersistentModelIndex hovered;
};

#endif
