import QtQuick 2.6

import QtGraphicalEffects 1.0

Row {
    spacing: 0
    property int rating : 0
    property int mouseIndex : 0

    signal ratingChangedByUser(int rating)

    Repeater {
        id: rating_compact
        model: 5
        Item {
            width: 25
            height: 20

            Image {
                id: star
                source: "info-rating.png"
            }

            ColorOverlay {
                anchors.fill: star
                source: star
                color: index < (mouseIndex > 0 ? mouseIndex : rating) ? ratingSelectedColor : ratingUnselectedColor
            }

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true

                onPositionChanged: {
                    mouseIndex = index + 1;
                }

                onClicked: {
                    ratingChangedByUser(mouseIndex);
                }

                onExited: {
                    mouseIndex = 0;
                }
            }
        }
    }


}
