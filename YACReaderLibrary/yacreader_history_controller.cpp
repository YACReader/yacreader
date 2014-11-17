#include "yacreader_history_controller.h"

YACReaderHistoryController::YACReaderHistoryController(QObject *parent) :
    QObject(parent)
{
}

void YACReaderHistoryController::clear()
{
    currentFolderNavigation = 0;
    history.clear();
    history.append(QModelIndex()); //root folder is always the first item

    emit(enabledBackward(false));
    emit(enabledForward(false));
}

void YACReaderHistoryController::backward()
{
    if(currentFolderNavigation>0)
    {
        currentFolderNavigation--;
        emit(modelIndexSelected(history.at(currentFolderNavigation)));
        emit(enabledForward(true));
    }

    if(currentFolderNavigation==0)
        emit(enabledBackward(false));
}

void YACReaderHistoryController::forward()
{
    if(currentFolderNavigation<history.count()-1)
    {
        currentFolderNavigation++;
        emit(modelIndexSelected(history.at(currentFolderNavigation)));
        emit(enabledBackward(true));
    }

    if(currentFolderNavigation==history.count()-1)
        emit(enabledForward(false));
}

void YACReaderHistoryController::updateHistory(const QModelIndex &mi)
{
    //remove history from current index
    if(!mi.isValid() && history.count() == 1)
        return;

    int numElementsToRemove = history.count() - (currentFolderNavigation+1);
    while(numElementsToRemove>0)
    {
        numElementsToRemove--;
        history.removeLast();
    }

    if(mi!=history.at(currentFolderNavigation))
    {
        history.append(mi);

        emit(enabledBackward(true));
        currentFolderNavigation++;
    }

    emit(enabledForward(false));
}
