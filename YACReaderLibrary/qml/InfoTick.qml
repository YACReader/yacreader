import QtQuick 2.6

import QtGraphicalEffects 1.0

Item {

    Image {
        id: read_compact

        source: "info-tick.png"
    }

    ColorOverlay {
        anchors.fill: read_compact
        source: read_compact
        color: true ? "#e84852" : "#1c1c1c"
    }
}
