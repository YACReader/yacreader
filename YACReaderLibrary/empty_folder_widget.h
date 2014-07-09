#ifndef EMPTY_FOLDER_WIDGET_H
#define EMPTY_FOLDER_WIDGET_H

#include <QWidget>
#include <QModelIndex>

class QLabel;
class QListView;
class QStringListModel;

class EmptyFolderWidget : public QWidget
{
    Q_OBJECT
public:
    explicit EmptyFolderWidget(QWidget *parent = 0);
    void setSubfolders(const QModelIndex & mi, const QStringList & foldersNames);
signals:
    void subfolderSelected(QModelIndex, int);

public slots:
    void onItemClicked(const QModelIndex & mi);

protected:
    QLabel * iconLabel;
    QLabel * titleLabel;
    QListView * foldersView;
    QModelIndex parent;
    QStringListModel * subfoldersModel;
    void paintEvent(QPaintEvent *);
};

#endif // EMPTY_FOLDER_WIDGET_H
