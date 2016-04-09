import QtQuick 2.6

import QtGraphicalEffects 1.0

Row {
    spacing: 6

    Repeater {
        id: rating_compact
        model: 5
        Image {
            source: "info-rating.png"

            ColorOverlay {
                anchors.fill: rating_compact.itemAt(index)
                source: rating_compact.itemAt(index)
                color: index <= 3 ? "#ffffff" : "#1c1c1c"
            }
        }
    }
}
