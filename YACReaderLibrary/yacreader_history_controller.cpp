#include "yacreader_history_controller.h"

YACReaderHistoryController::YACReaderHistoryController(QObject *parent)
    : QObject(parent)
{
}

void YACReaderHistoryController::clear()
{
    currentFolderNavigation = 0;
    history.clear();
    history.append(YACReaderLibrarySourceContainer(QModelIndex(), YACReaderLibrarySourceContainer::Folder)); // root folder is always the first item

    emit enabledBackward(false);
    emit enabledForward(false);
}

void YACReaderHistoryController::backward(const ContentViewState &currentViewState)
{
    if (currentFolderNavigation > 0) {
        history[currentFolderNavigation].viewState = currentViewState;
        currentFolderNavigation--;
        emit modelIndexSelected(history.at(currentFolderNavigation));
        emit enabledForward(true);
    }

    if (currentFolderNavigation == 0)
        emit enabledBackward(false);
}

void YACReaderHistoryController::forward(const ContentViewState &currentViewState)
{
    if (currentFolderNavigation < history.count() - 1) {
        history[currentFolderNavigation].viewState = currentViewState;
        currentFolderNavigation++;
        emit modelIndexSelected(history.at(currentFolderNavigation));
        emit enabledBackward(true);
    }

    if (currentFolderNavigation == history.count() - 1)
        emit enabledForward(false);
}

void YACReaderHistoryController::recordViewStateForCurrentEntry(const ContentViewState &state)
{
    if (!history.isEmpty())
        history[currentFolderNavigation].viewState = state;
}

void YACReaderHistoryController::updateHistory(const YACReaderLibrarySourceContainer &source)
{
    // remove history from current index
    if (!source.sourceModelIndex.isValid() && history.count() == 1)
        return;

    int numElementsToRemove = history.count() - (currentFolderNavigation + 1);
    while (numElementsToRemove > 0) {
        numElementsToRemove--;
        history.removeLast();
    }

    if (source != history.at(currentFolderNavigation)) {
        history.append(source);

        emit enabledBackward(true);
        currentFolderNavigation++;
    }

    emit enabledForward(false);
}

YACReaderLibrarySourceContainer YACReaderHistoryController::lastSourceContainer()
{
    return history.last();
}

YACReaderLibrarySourceContainer YACReaderHistoryController::currentSourceContainer()
{
    return history.at(currentFolderNavigation);
}

//------------------------------------------------------------------------------

YACReaderLibrarySourceContainer::YACReaderLibrarySourceContainer()
    : sourceModelIndex(QModelIndex()), type(None)
{
}

YACReaderLibrarySourceContainer::YACReaderLibrarySourceContainer(const QModelIndex &sourceModelIndex, YACReaderLibrarySourceContainer::SourceType type)
    : sourceModelIndex(sourceModelIndex), type(type)
{
}

QModelIndex YACReaderLibrarySourceContainer::getSourceModelIndex() const
{
    return sourceModelIndex;
}

YACReaderLibrarySourceContainer::SourceType YACReaderLibrarySourceContainer::getType() const
{
    return type;
}

ContentViewState YACReaderLibrarySourceContainer::getViewState() const
{
    return viewState;
}

bool YACReaderLibrarySourceContainer::operator==(const YACReaderLibrarySourceContainer &other) const
{
    return sourceModelIndex == other.sourceModelIndex && type == other.type;
}

bool YACReaderLibrarySourceContainer::operator!=(const YACReaderLibrarySourceContainer &other) const
{
    return !(*this == other);
}
