import QtQuick

Rectangle {
    id: header

    required property var contentModel
    required property bool sectionVisible

    signal openRequested(int index)
    signal contextMenuRequested(int index, point position)

    readonly property int sectionHeight: 430
    readonly property int topMargin: 20

    function handlesWheelAt(position) {
        const listPosition = list.mapFromItem(header, position.x, position.y)
        return listPosition.x >= 0 && listPosition.x <= list.width
                && listPosition.y >= 0 && listPosition.y <= list.height
                && list.contentWidth > list.width
    }

    color: "transparent"
    height: list.count > 0 && sectionVisible ? sectionHeight : topMargin

    Rectangle {
        width: header.width
        height: header.sectionHeight - header.topMargin
        visible: list.count > 0 && header.sectionVisible
        color: continueReadingBackgroundColor

        Text {
            id: heading
            text: qsTr("Continue Reading...")
            color: continueReadingTextColor
            anchors { left: parent.left; top: parent.top; topMargin: 15; leftMargin: 25 }
            font.pointSize: 18
            font.weight: Font.DemiBold
        }

        ListView {
            id: list
            objectName: "continueReadingList"
            anchors {
                top: heading.bottom
                left: parent.left
                right: parent.right
                bottom: parent.bottom
                topMargin: 15
                bottomMargin: 20
                leftMargin: 25
                rightMargin: 20
            }
            orientation: Qt.Horizontal
            pixelAligned: true
            model: header.contentModel
            spacing: 20
            property int verticalPadding: 20

            WheelHandler {
                onWheel: event => {
                    if (list.contentWidth <= list.width)
                        return
                    list.contentX = Math.min(list.contentWidth - list.width - list.anchors.leftMargin,
                                             Math.max(list.originX, list.contentX - event.angleDelta.y))
                }
            }

            delegate: Rectangle {
                width: Math.floor((list.height - (list.verticalPadding * 2)) * 0.65)
                height: list.height - (list.verticalPadding * 2)
                color: "transparent"
                scale: mouseArea.containsMouse ? 1.025 : 1
                Behavior on scale { NumberAnimation { duration: 90 } }

                Image {
                    id: cover
                    anchors.fill: parent
                    source: cover_path
                    fillMode: Image.PreserveAspectCrop
                    smooth: true
                    mipmap: true
                    asynchronous: true
                    cache: true
                }

                Text {
                    anchors { top: cover.bottom; left: cover.left; right: cover.right; leftMargin: 4; rightMargin: 4; topMargin: 4 }
                    maximumLineCount: 2
                    wrapMode: Text.WordWrap
                    text: readable_title
                    elide: Text.ElideRight
                    color: itemTitleColor
                    font.letterSpacing: fontSpacing
                    font.pointSize: fontSize
                    font.family: fontFamily
                }

                Rectangle {
                    anchors.fill: cover
                    color: "transparent"
                    border.color: comicCoverBorderColor
                    border.width: 1
                }

                MouseArea {
                    id: mouseArea
                    anchors.fill: parent
                    acceptedButtons: Qt.LeftButton | Qt.RightButton
                    hoverEnabled: true

                    onDoubleClicked: {
                        list.currentIndex = index
                        header.openRequested(index)
                    }
                    onReleased: mouse => {
                        list.currentIndex = index
                        if (mouse.button === Qt.RightButton) {
                            var position = header.mapFromItem(cover, mouseX, mouseY)
                            header.contextMenuRequested(index, Qt.point(position.x, position.y))
                        }
                        mouse.accepted = true
                    }
                }
            }
        }
    }
}
