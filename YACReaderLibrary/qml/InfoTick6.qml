import QtQuick 2.15

import Qt5Compat.GraphicalEffects

Item {

    property bool read

    signal readChangedByUser(bool read)

    MouseArea {
        anchors.fill: read_compact
        onClicked: {
            readChangedByUser(!read);
        }
    }

    Image {
        id: read_compact
        source: "info-tick.png"
    }

    ColorOverlay {
        anchors.fill: read_compact
        source: read_compact
        color: read ? readTickCheckedColor : readTickUncheckedColor
    }
}
