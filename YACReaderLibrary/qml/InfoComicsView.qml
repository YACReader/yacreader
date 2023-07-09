import QtQuick 2.15

import QtQuick.Controls 2.15

import com.yacreader.ComicModel 1.0

Rectangle {
    id: main

    color: infoBackgroundColor

    //width: parent.width
    //height: parent.height
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
        source: topShadow
        width: parent.width
        fillMode: Image.TileHorizontally
    }

    Rectangle {
        id: indicator_container
        width: parent.width
        y: 250

        Image {
            id: indicator
            source: infoIndicator
        }

        Image {
            id: bottom_shadow
            x: indicator.width
            width: parent.width - indicator.width
            source: infoShadow
            fillMode: Image.TileHorizontally
        }
    }

    Rectangle {
        id: info_container
        width: parent.width
        y: flow.height + flow.additionalBottomSpace - 6
        height: parent.height - y

        clip: true

        color: infoBackgroundColor

        Flickable{
            id: infoFlickable
            anchors.fill: parent
            anchors.margins: 0

            contentWidth: infoView.width
            contentHeight: infoView.height

            ComicInfoView {
                id: infoView
                width: info_container.width - 14
            }

            WheelHandler {
                onWheel: {
                    if (infoFlickable.contentHeight <= infoFlickable.height) {
                        return;
                    }

                    var newValue =  Math.min((infoFlickable.contentHeight - infoFlickable.height), (Math.max(infoFlickable.originY , infoFlickable.contentY - event.angleDelta.y)));
                    infoFlickable.contentY = newValue;
                }
            }

            ScrollBar.vertical: ScrollBar {
                visible: infoFlickable.contentHeight > infoFlickable.height

                contentItem: Item {
                    implicitWidth: 12
                    implicitHeight: 26
                    Rectangle {
                        color: "#424246"
                        anchors.fill: parent
                        anchors.topMargin: 6
                        anchors.leftMargin: 5
                        anchors.rightMargin: 4
                        anchors.bottomMargin: 6
                        radius: 2
                    }
                }
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
        }

        onDropped: {
            if(drop.hasUrls && dropManager.canDropUrls(drop.urls, drop.action))
            {
                dropManager.droppedFiles(drop.urls, drop.action);
            }
        }
    }
}
