import QtQuick 2.3

import QtQuick.Controls 1.2
import QtGraphicalEffects 1.0

import com.yacreader.ComicModel 1.0

Rectangle {
    id: main

    color: "#2e2e2e"

    width: parent.width
    height: parent.height
    anchors.margins: 0

    FlowView {
        id: flow
        height: 256 //TODO dynamic size?

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right

        additionalBottomSpace: indicator.height
    }

    Image {
        id: top_shadow
        source: "info-top-shadow.png"
        width: parent.width
        fillMode: Image.TileHorizontally
    }

    Rectangle {
        width: parent.width
        y: 250

        Image {
            id: indicator
            source: "info-indicator.png"
        }

        Image {
            id: bottom_shadow
            x: indicator.width
            width: parent.width - indicator.width
            source: "info-shadow.png"
            fillMode: Image.TileHorizontally
        }
    }
    Rectangle {
        id: info_container
        width: parent.width
        y: flow.height + flow.additionalBottomSpace - 6
        height: parent.height

        color: "#2e2e2e"
    }


}
