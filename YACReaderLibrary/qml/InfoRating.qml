import QtQuick 2.6

import QtGraphicalEffects 1.0

Row {
    spacing: 6

    Repeater {
        id: rating_compact
        model: 5
        Item {
            width: 20
            height: 20

            Image {
                id: star
                source: "info-rating.png"
            }

            ColorOverlay {
                anchors.fill: star
                source: star
                color: index <= 2 ? "#ffffff" : "#1c1c1c"
            }
        }
    }
}
