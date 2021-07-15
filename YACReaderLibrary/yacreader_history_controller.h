#ifndef YACREADER_HISTORY_CONTROLLER_H
#define YACREADER_HISTORY_CONTROLLER_H

#include <QObject>

#include <QModelIndex>

class YACReaderHistoryController;

class YACReaderLibrarySourceContainer
{
public:
    enum SourceType {
        None,
        Folder,
        List
    };

    explicit YACReaderLibrarySourceContainer();
    explicit YACReaderLibrarySourceContainer(const QModelIndex &sourceModelIndex, YACReaderLibrarySourceContainer::SourceType type);
    QModelIndex getSourceModelIndex() const;
    YACReaderLibrarySourceContainer::SourceType getType() const;

    bool operator==(const YACReaderLibrarySourceContainer &other) const;
    bool operator!=(const YACReaderLibrarySourceContainer &other) const;

protected:
    QModelIndex sourceModelIndex;
    YACReaderLibrarySourceContainer::SourceType type;

    friend class YACReaderHistoryController;
};

Q_DECLARE_METATYPE(YACReaderLibrarySourceContainer)

class YACReaderHistoryController : public QObject
{
    Q_OBJECT
public:
    explicit YACReaderHistoryController(QObject *parent = nullptr);

signals:
    void enabledForward(bool enabled);
    void enabledBackward(bool enabled);
    void modelIndexSelected(YACReaderLibrarySourceContainer);

public slots:
    void clear();
    void backward();
    void forward();
    void updateHistory(const YACReaderLibrarySourceContainer &source);
    YACReaderLibrarySourceContainer currentSourceContainer();

protected:
    int currentFolderNavigation;
    QList<YACReaderLibrarySourceContainer> history;
};

#endif // YACREADER_HISTORY_CONTROLLER_H
