import QtQuick 2.6

import QtGraphicalEffects 1.0

Image {
    id: favorites_button_compact
    source: "info-favorites.png"

    ColorOverlay {
        anchors.fill: favorites_button_compact
        source: favorites_button_compact
        color: "#e84852"
    }
}

