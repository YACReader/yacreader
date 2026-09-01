#ifndef YACREADER_HISTORY_CONTROLLER_H
#define YACREADER_HISTORY_CONTROLLER_H

#include "content_view_state.h"

#include <QModelIndex>
#include <QObject>

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
    ContentViewState getViewState() const;

    bool operator==(const YACReaderLibrarySourceContainer &other) const;
    bool operator!=(const YACReaderLibrarySourceContainer &other) const;

protected:
    QModelIndex sourceModelIndex;
    YACReaderLibrarySourceContainer::SourceType type;
    ContentViewState viewState;

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
    void backward(const ContentViewState &currentViewState);
    void forward(const ContentViewState &currentViewState);
    void updateHistory(const YACReaderLibrarySourceContainer &source);
    void recordViewStateForCurrentEntry(const ContentViewState &state);
    YACReaderLibrarySourceContainer lastSourceContainer();
    YACReaderLibrarySourceContainer currentSourceContainer();

protected:
    int currentFolderNavigation;
    QList<YACReaderLibrarySourceContainer> history;
};

#endif // YACREADER_HISTORY_CONTROLLER_H
