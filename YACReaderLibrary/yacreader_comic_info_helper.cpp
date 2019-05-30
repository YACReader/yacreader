#include "yacreader_comic_info_helper.h"

#include "comic_model.h"

YACReaderComicInfoHelper::YACReaderComicInfoHelper(QObject *parent)
    : QObject(parent), model(nullptr)
{
}

void YACReaderComicInfoHelper::setModel(ComicModel *model)
{
    this->model = model;
}

void YACReaderComicInfoHelper::rate(int index, int rating)
{
    if (model != nullptr)
        model->updateRating(rating, model->index(index, 0));
}

void YACReaderComicInfoHelper::setRead(int index, bool read)
{
    YACReaderComicReadStatus status;
    read ? (status = YACReaderComicReadStatus::Read) : (status = YACReaderComicReadStatus::Unread);

    if (model != nullptr)
        model->setComicsRead(QModelIndexList() << model->index(index, 0), status);
}

void YACReaderComicInfoHelper::addToFavorites(int index)
{
    if (model != nullptr)
        model->addComicsToFavorites(QModelIndexList() << model->index(index, 0));
}

void YACReaderComicInfoHelper::removeFromFavorites(int index)
{
    if (model != nullptr)
        model->deleteComicsFromFavorites(QModelIndexList() << model->index(index, 0));
}
