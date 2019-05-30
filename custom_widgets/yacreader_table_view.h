#ifndef YACREADER_TABLE_VIEW_H
#define YACREADER_TABLE_VIEW_H

#include <QTableView>
#include <QStyledItemDelegate>

class YACReaderDeletingProgress;
class QResizeEvent;
class QPropertyAnimation;

class YACReaderTableView : public QTableView
{
    Q_OBJECT
public:
    explicit YACReaderTableView(QWidget *parent = 0);

signals:
    void comicRated(int, QModelIndex);
public slots:
    void showDeleteProgress();
    void hideDeleteProgress();
    void closeRatingEditor();
protected slots:

    virtual void closeEditor(QWidget *editor, QAbstractItemDelegate::EndEditHint hint);
    virtual void commitData(QWidget *editor);

private:
    YACReaderDeletingProgress *deletingProgress;
    bool showDelete;
    QPropertyAnimation *showDeletingProgressAnimation;

    void resizeEvent(QResizeEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void leaveEvent(QEvent *event);
    void performDrag();
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);

    bool editing;
    QModelIndex currentIndexEditing;
    QWidget *myeditor;

    //drag from here
    QPoint startDragPos;
};

//---

class YACReaderRatingDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    YACReaderRatingDelegate(QWidget *parent = 0)
        : QStyledItemDelegate(parent) {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const;
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const;

private slots:
    void sendCloseEditor();
    void sendCommitData();
};

//---

class StarRating
{
public:
    enum EditMode { Editable,
                    ReadOnly };

    StarRating(int starCount = 1, int maxStarCount = 5);

    void paint(QPainter *painter, const QRect &rect,
               const QPalette &palette, EditMode mode) const;
    void paintSelected(QPainter *painter, const QRect &rect,
                       const QPalette &palette, EditMode mode, QColor color) const;
    void paintSelected(QPainter *painter, const QRect &rect,
                       const QPalette &palette, EditMode mode) const;
    QSize sizeHint() const;
    int starCount() const { return myStarCount; }
    int maxStarCount() const { return myMaxStarCount; }
    void setStarCount(int starCount) { myStarCount = starCount; }
    void setMaxStarCount(int maxStarCount) { myMaxStarCount = maxStarCount; }

private:
    QPolygonF starPolygon;
    QPolygonF diamondPolygon;
    int myStarCount;
    int myMaxStarCount;
};
Q_DECLARE_METATYPE(StarRating);
//---

class StarEditor : public QWidget
{
    Q_OBJECT

public:
    StarEditor(QWidget *parent = 0);

    QSize sizeHint() const;
    void setStarRating(const StarRating &starRating)
    {
        myStarRating = starRating;
    }
    StarRating starRating() { return myStarRating; }
    bool getShouldCommitData() { return shouldCommitData; };

signals:
    void editingFinished();
    void commitData();

protected:
    void paintEvent(QPaintEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void leaveEvent(QEvent *event);

private:
    int starAtPosition(int x);
    StarRating myStarRating;
    bool shouldCommitData;
};
#endif // YACREADER_TABLE_VIEW_H
