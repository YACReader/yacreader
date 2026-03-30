#include "current_comic_view_helper.h"

#include "comic_db.h"

ComicDB currentComicFromModel(ComicModel *model, bool &found)
{
    const auto comics = model->getAllComics();

    for (const auto &comic : comics) {
        if (comic.info.read == false) {
            found = true;
            return comic;
        }
    }

    found = false;
    return ComicDB();
}
