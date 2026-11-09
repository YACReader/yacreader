import QtQuick

import QtQuick.Controls.impl

Item {
    width: 20
    height: 20

    property bool active

    opacity: enabled ? 1.0 : 0.4

    signal activeChangedByUser(bool active)

    MouseArea {
        anchors.fill: favorites_button_compact
        onClicked: {
            activeChangedByUser(!active);
        }
    }

    ColorImage {
        anchors.centerIn: parent
        id: favorites_button_compact
        source: "info-favorites.svg"
        color: active ? favCheckedColor : favUncheckedColor
    }
}

