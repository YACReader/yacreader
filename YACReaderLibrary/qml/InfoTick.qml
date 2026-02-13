import QtQuick

import QtQuick.Controls.impl

Item {

    property bool read

    signal readChangedByUser(bool read)

    MouseArea {
        anchors.fill: read_compact
        onClicked: {
            readChangedByUser(!read);
        }
    }

    ColorImage {
        id: read_compact
        source: "info-tick.svg"
        color: read ? readTickCheckedColor : readTickUncheckedColor
    }
}
