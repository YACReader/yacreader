import QtQuick

import QtQuick.Controls.impl

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

            ColorImage {
                id: star
                source: "info-rating.svg"
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
