#ifndef YACREADER_HISTORY_CONTROLLER_H
#define YACREADER_HISTORY_CONTROLLER_H

#include <QObject>

#include <QModelIndex>

class YACReaderHistoryController : public QObject
{
    Q_OBJECT
public:
    explicit YACReaderHistoryController(QObject *parent = 0);

signals:
    void enabledForward(bool enabled);
    void enabledBackward(bool enabled);
    void modelIndexSelected(QModelIndex mi);

public slots:
    void clear();
    void backward();
    void forward();
    void updateHistory(const QModelIndex & mi);

protected:
    int currentFolderNavigation;
    QList<QModelIndex> history;

};

#endif // YACREADER_HISTORY_CONTROLLER_H
