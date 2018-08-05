import QtQuick 2.9

import QtQuick.Controls 1.4

Image {
    id: ratingImage
    anchors {bottom: realCell.bottom; right: pageImage.left; bottomMargin: 5; rightMargin: Math.floor(pages.width)+12}
    source: "star.png"
    width: 13
    height: 11

    MouseArea  {
        anchors.fill: parent
        onPressed: {
            console.log("rating");
            comicsSelectionHelper.clear();
            comicsSelectionHelper.selectIndex(index);
            grid.currentIndex = index;
            ratingConextMenu.popup();
        }
    }

    MenuBar
    {
        Menu {
            id: ratingConextMenu
            MenuItem { text: "1"; enabled: true; iconSource:"star_menu.png"; onTriggered: comicRatingHelper.rate(index,1) }
            MenuItem { text: "2"; enabled: true; iconSource:"star_menu.png"; onTriggered: comicRatingHelper.rate(index,2) }
            MenuItem { text: "3"; enabled: true; iconSource:"star_menu.png"; onTriggered: comicRatingHelper.rate(index,3) }
            MenuItem { text: "4"; enabled: true; iconSource:"star_menu.png"; onTriggered: comicRatingHelper.rate(index,4) }
            MenuItem { text: "5"; enabled: true; iconSource:"star_menu.png"; onTriggered: comicRatingHelper.rate(index,5) }

        }
    }
}

