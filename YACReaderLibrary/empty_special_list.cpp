#include "empty_special_list.h"

EmptySpecialListWidget::EmptySpecialListWidget(QWidget *parent)
    : EmptyContainerInfo(parent)
{
    setUpDefaultLayout(true);
}

void EmptySpecialListWidget::showFavorites()
{
    currentType = Favorites;
    setPixmap(theme.emptyContainer.emptyFavoritesIcon);
    setText(tr("No favorites"));
}

void EmptySpecialListWidget::showReading()
{
    currentType = Reading;
    setPixmap(theme.emptyContainer.emptyCurrentReadingsIcon);
    setText(tr("You are not reading anything yet, come on!!"));
}

void EmptySpecialListWidget::showRecent()
{
    currentType = Recent;
    setPixmap(QPixmap());
    setText(tr("There are no recent comics!"));
}

void EmptySpecialListWidget::applyTheme(const Theme &theme)
{
    EmptyContainerInfo::applyTheme(theme);
    updateIcon();
}

void EmptySpecialListWidget::updateIcon()
{
    switch (currentType) {
    case Favorites:
        setPixmap(theme.emptyContainer.emptyFavoritesIcon);
        break;
    case Reading:
        setPixmap(theme.emptyContainer.emptyCurrentReadingsIcon);
        break;
    case Recent:
    case None:
        break;
    }
}
