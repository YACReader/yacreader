#ifndef EMPTY_SPECIAL_LIST_H
#define EMPTY_SPECIAL_LIST_H

#include <QtWidgets>
#include "empty_container_info.h"

class EmptySpecialListWidget : public EmptyContainerInfo
{
public:
    enum SpecialListType {
        None,
        Favorites,
        Reading,
        Recent
    };

    EmptySpecialListWidget(QWidget *parent = nullptr);

    void showFavorites();
    void showReading();
    void showRecent();

protected:
    void applyTheme(const Theme &theme) override;

private:
    void updateIcon();
    SpecialListType currentType = None;
};

#endif // EMPTY_SPECIAL_LIST_H
