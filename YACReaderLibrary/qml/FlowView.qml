import QtQuick 2.3

import QtGraphicalEffects 1.0

import com.yacreader.ComicModel 1.0

Rectangle {
    property url backgroundImageURL;

    property real backgroundBlurRadius : 100;
    property real backgroundBlurOpacity : 0.25;
    property bool backgroundBlurVisible : true;

    property real additionalBottomSpace;

    property real verticalPadding: 11

    Rectangle {
        id: background
        color: "#2A2A2A"
        anchors.fill: backgroundImg
    }

    Image {
        id: backgroundImg
        width: parent.width
        height: parent.height + additionalBottomSpace
        source: backgroundImage
        fillMode: Image.PreserveAspectCrop
        smooth: true
        mipmap: true
        asynchronous : true
        cache: false //TODO clear cache only when it is needed
        opacity: 0
        visible: false
    }

    FastBlur {
        anchors.fill: backgroundImg
        source: backgroundImg
        radius: backgroundBlurRadius
        opacity: backgroundBlurOpacity
        visible: backgroundBlurVisible
    }

    anchors.margins: 0


    MouseArea {
        anchors.fill : list
        onWheel: {
                var ci
                if(wheel.angleDelta.y < 0) {
                    ci = Math.min(list.currentIndex+1, list.count);
                }
                else if(wheel.angleDelta.y > 0) {
                    ci = Math.max(0,list.currentIndex-1);
                } else {
                    return;
                }

                list.currentIndex = ci;
        }
    }

    ListView {
        id: list
        anchors.fill: parent

        property int previousIndex;

        orientation: Qt.Horizontal
        pixelAligned: true

        model: comicsList

        spacing: 16
        anchors.leftMargin: 11

        snapMode: ListView.SnapToItem

        highlightFollowsCurrentItem: true
        highlightRangeMode: ListView.StrictlyEnforceRange
        preferredHighlightEnd: 50

        delegate: Component {

            //cover
            Rectangle {
                width: Math.floor((list.height - (verticalPadding * 2)) * 0.65);
                height: list.height - (verticalPadding * 2);
                anchors.verticalCenter: parent.verticalCenter

                color:"transparent"

                DropShadow {
                    anchors.fill: coverElement
                    horizontalOffset: 0
                    verticalOffset: 0
                    radius: 6
                    samples: 17
                    color: "#BB000000"
                    source: coverElement
                    visible: (Qt.platform.os === "osx") ? false : true;
                }

                Image {
                    id: coverElement
                    anchors.fill: parent
                    source: cover_path
                    fillMode: Image.PreserveAspectCrop
                    smooth: true
                    mipmap: true
                    asynchronous : true
                    cache: false
                }

                //border
                Rectangle {
                    width: coverElement.width
                    height: coverElement.height
                    anchors.centerIn: coverElement
                    color: "transparent"
                    border {
                        color: "#30FFFFFF"
                        width: 1
                    }
                }

                MouseArea  {
                    id: mouseArea
                    anchors.fill: parent
                    acceptedButtons: Qt.LeftButton | Qt.RightButton

                    onClicked: {
                        gotoIndex(index);
                    }
                }

                NumberAnimation {
                    id: anim;
                    target: list;
                    property: "contentX";
                    duration: Math.min(850, Math.max(350, 75 * Math.abs(list.currentIndex - list.previousIndex)))
                }

                function gotoIndex(idx) {
                    var pos = list.contentX;
                    var destPos;
                    list.previousIndex = list.currentIndex
                    list.positionViewAtIndex(idx, ListView.Beginning);
                    destPos = list.contentX;
                    anim.from = pos;
                    anim.to = destPos;
                    anim.running = true;
                }
            }
        }

        focus: true
        Keys.onPressed: {
            if (event.modifiers & Qt.ControlModifier || event.modifiers & Qt.ShiftModifier)
                return;
            var ci
            if (event.key === Qt.Key_Right) {
                ci = Math.min(list.currentIndex+1, list.count);
            }
            else if (event.key === Qt.Key_Left) {
                ci = Math.max(0,list.currentIndex-1);
            } else {
                return;
            }

            list.currentIndex = ci;

            event.accepted = true;
        }

    }
}
