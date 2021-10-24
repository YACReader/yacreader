import QtQuick 2.15

import QtGraphicalEffects 1.0

Item {
    width: 20
    height: 20

    property bool active

    signal activeChangedByUser(bool active)

    MouseArea {
        anchors.fill: favorites_button_compact
        onClicked: {
            activeChangedByUser(!active);
        }
    }

    Image {
        anchors.centerIn: parent
        id: favorites_button_compact
        source: "info-favorites.png"
    }

    ColorOverlay {
        anchors.fill: favorites_button_compact
        source: favorites_button_compact
        color: active ? favCheckedColor : favUncheckedColor
    }
}

