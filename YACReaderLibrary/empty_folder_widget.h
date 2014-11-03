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

    //Drops
    void copyComicsToCurrentFolder(QList<QPair<QString, QString> >);
    void moveComicsToCurrentFolder(QList<QPair<QString, QString> >);

public slots:
    void onItemClicked(const QModelIndex & mi);

protected:
    QLabel * iconLabel;
    QLabel * titleLabel;
    QListView * foldersView;
    QModelIndex parent;
    QStringListModel * subfoldersModel;
    void paintEvent(QPaintEvent *);

    //Drop to import
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
};

#endif // EMPTY_FOLDER_WIDGET_H
