import QtQuick 2.5

import QtQuick.Controls 1.2
import QtGraphicalEffects 1.0
import QtQuick.Controls.Styles 1.4

import com.yacreader.ComicModel 1.0

Rectangle {
    id: main

    color: "#2e2e2e"

    width: parent.width
    height: parent.height
    anchors.margins: 0

    FlowView {
        id: flow
        objectName: "flow"
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
        id: indicator_container
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
        height: parent.height - y

        color: "#2e2e2e"

        ScrollView {
            anchors.fill: parent
            anchors.margins: 0

            horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff

            style: ScrollViewStyle {
                transientScrollBars: false
                incrementControl: Item {}
                decrementControl: Item {}
                handle: Item {
                    implicitWidth: 10
                    implicitHeight: 26
                    Rectangle {
                        color: "#424246"
                        anchors.fill: parent
                        anchors.topMargin: 6
                        anchors.leftMargin: 4
                        anchors.rightMargin: 4
                        anchors.bottomMargin: 6
                    }
                }

                scrollBarBackground: Item {
                    implicitWidth: 14
                    implicitHeight: 26
                }
            }

            ComicInfo {
                width: info_container.width - 14
            }
        }
    }

    DropArea {
        anchors.fill: parent

        onEntered: {
            if(drag.hasUrls)
            {
                if(dropManager.canDropUrls(drag.urls, drag.action))
                {
                    drag.accepted = true;
                }else
                    drag.accepted = false;
            }
            else if (dropManager.canDropFormats(drag.formats)) {
                drag.accepted = true;
            } else
                drag.accepted = false;
        }

        onDropped: {
            if(drop.hasUrls && dropManager.canDropUrls(drop.urls, drop.action))
            {
                dropManager.droppedFiles(drop.urls, drop.action);
            }
        }
    }
}
