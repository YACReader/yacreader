import QtQuick 2.15
import QtQuick.Controls 2.15

import Qt5Compat.GraphicalEffects

import com.yacreader.ComicModel 1.0

Rectangle {
    id: mainFlowContainer

    property url backgroundImageURL;

    property real backgroundBlurRadius : 100; //85;
    property real backgroundBlurOpacity : 0.25; //0.35;
    property bool backgroundBlurVisible : true;

    property real additionalBottomSpace : 0;

    property real verticalPadding: 12

    property real itemsSpacing: 17

    signal currentCoverChanged(int index)

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
        onWheel: wheel => {

            if(list.moving)
                return;

                var ci
                if(wheel.angleDelta.y < 0) {
                    ci = Math.min(list.currentIndex+1, list.count - 1);
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
        objectName: "list"
        anchors.fill: parent

        property int previousIndex;

        orientation: Qt.Horizontal
        pixelAligned: true

        model: comicsList

        spacing: itemsSpacing
        anchors.leftMargin: Math.floor(verticalPadding * 1.1)

        snapMode: ListView.SnapToItem

        highlightFollowsCurrentItem: true
        highlightRangeMode: ListView.StrictlyEnforceRange
        preferredHighlightEnd: 50

        highlightMoveDuration: 250

        onCurrentIndexChanged: currentIndex => {
                                   if (list.currentIndex !== -1) {
                                       mainFlowContainer.currentCoverChanged(list.currentIndex);
        }
                               }

        delegate: Component {

            //cover
            Rectangle {
                width: Math.floor((list.height - (verticalPadding * 2)) * 0.65);
                height: list.height - (verticalPadding * 2);
                anchors.verticalCenter: parent.verticalCenter

                color:"transparent"

                scale: mouseArea.containsMouse ? 1.025 : 1

                Behavior on scale {
                    NumberAnimation { duration: 90 }
                }

            BusyIndicator {
                scale: 0.5
                anchors.centerIn: parent
                running: coverElement.status === Image.Loading
                }

                DropShadow {
                    transparentBorder: true
                    anchors.fill: coverElement
                    horizontalOffset: 0
                    verticalOffset: 0
                    radius: 6
                    //samples: 17
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


                //mark
                Image {
                    id: mark
                    width: 23
                    height: 23
                    source: read_column&&show_marks?"tick.png":has_been_opened&&show_marks?"reading.png":""
                    anchors {right: coverElement.right; top: coverElement.top; topMargin: 9; rightMargin: 9}
                    asynchronous : true
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

                    hoverEnabled: true

                    onDoubleClicked: {
                        list.currentIndex = index;
                        currentIndexHelper.selectedItem(index);
                    }

                    onReleased: mouse => {
                        list.currentIndex = index;

                        if(mouse.button === Qt.RightButton) // context menu is requested
                        {
                            var coordinates = mainFlowContainer.mapFromItem(coverElement,mouseX,mouseY)
                            contextMenuHelper.requestedContextMenu(Qt.point(coordinates.x,coordinates.y));
                        }

                        mouse.accepted = true;
                    }
                }
            }
        }

        focus: true
        Keys.onPressed: event => {

            if (event.modifiers & Qt.ControlModifier || event.modifiers & Qt.ShiftModifier)
                return;
            var ci
            if (event.key === Qt.Key_Right || event.key === Qt.Key_Down) {
                ci = Math.min(list.currentIndex+1, list.count - 1);
            }
            else if (event.key === Qt.Key_Left || event.key === Qt.Key_Up) {
                ci = Math.max(0,list.currentIndex-1);
            } else {
                return;
            }

            list.currentIndex = ci;

            event.accepted = true;
        }

    }
}
