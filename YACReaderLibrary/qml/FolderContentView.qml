import QtQuick 2.15

import QtQuick.Controls 2.15
import QtQuick.Layouts 1.12

import QtGraphicalEffects 1.0

import com.yacreader.ComicModel 1.0

import com.yacreader.ComicInfo 1.0
import com.yacreader.ComicDB 1.0

Rectangle {
    id: main

    property int continuReadingHeight: 430;
    property int topContentMargin: 20;

    color: backgroundColor
    anchors.margins: 0

    Component {
        id: appDelegate
        Rectangle
        {
            id: cell
            width: grid.cellWidth
            height: grid.cellHeight
            color: "#00000000"

            scale: mouseArea.containsMouse ? 1.025 : 1

            Behavior on scale {
                NumberAnimation { duration: 90 }
            }

            Rectangle {
                id: realCell

                property int position : 0

                width: itemWidth
                height: itemHeight

                color: "transparent"
                anchors.horizontalCenter: parent.horizontalCenter

                MouseArea  {
                    id: mouseArea

                    anchors.fill: parent
                    acceptedButtons: Qt.LeftButton | Qt.RightButton

                    hoverEnabled: true

                    onDoubleClicked: {
                        openHelper.openFolder(index);
                    }

                    onPressed: mouse => {
                                   var ci = grid.currentIndex; //save current index

                                   mouse.accepted = true;

                                   if(mouse.button === Qt.RightButton) // context menu is requested
                                   {
                                       var coordinates = main.mapFromItem(realCell,mouseX,mouseY)
                                       contextMenuHelper.requestedFolderContextMenu(Qt.point(coordinates.x,coordinates.y), index);
                                       mouse.accepted = false;

                                   }
                               }

                }
            }

            /**/

            Rectangle {
                transform: Rotation { origin.x: coverWidth / 2; origin.y: coverHeight / 2; angle: -4}
                width: coverElement.width
                height: coverElement.height
                radius: 10
                anchors {horizontalCenter: parent.horizontalCenter; top: realCell.top; topMargin: 0}
                color: "#20000000"
                border {
                    color: "#20FFFFFF"
                    width: 1
                }
            }

            Rectangle {
                transform: Rotation { origin.x: coverWidth / 2; origin.y: coverHeight / 2; angle: 3}
                width: coverElement.width
                height: coverElement.height
                radius: 10
                anchors {horizontalCenter: parent.horizontalCenter; top: realCell.top; topMargin: 0}
                color: "#88000000"
                border {
                    color: "#20FFFFFF"
                    width: 1
                }
            }

            Item {
                width: coverWidth
                height: coverHeight
                anchors {horizontalCenter: parent.horizontalCenter; top: realCell.top; topMargin: 0}
                id: coverElement

                Image {
                    id: coverImage
                    anchors.fill: parent
                    source: cover_path
                    fillMode: Image.PreserveAspectCrop
                    smooth: true
                    mipmap: true
                    asynchronous : true
                    cache: true

                    layer.enabled: true
                    layer.effect: OpacityMask {
                        anchors.fill: parent
                        cached: true
                        maskSource: Rectangle {
                            width: coverElement.width
                            height: coverElement.height
                            radius: 10
                        }
                    }
                }
            }

            //border
            Rectangle {
                width: coverElement.width
                height: coverElement.height
                radius: 10
                anchors {horizontalCenter: parent.horizontalCenter; top: realCell.top; topMargin: 0}
                color: "transparent"
                border {
                    color: "#20FFFFFF"
                    width: 1
                }
            }

            //folder name
            Text {
                id : titleText
                anchors { top: coverElement.bottom; left: realCell.left; leftMargin: 4; rightMargin: 4; topMargin: 10; }
                width: itemWidth - 8
                maximumLineCount: 2
                wrapMode: Text.WordWrap
                text: name
                elide: Text.ElideRight
                color: titleColor
                font.letterSpacing: fontSpacing
                font.pointSize: fontSize
                font.family: fontFamily
            }
        }
    }

    Rectangle {
        id: scrollView
        objectName: "topScrollView"
        anchors.fill: parent
        anchors.margins: 0
        children: grid

        color: "transparent"

        function scrollToOrigin() {
            grid.contentY = grid.originY
            grid.contentX = grid.originX
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
                else{
                    if (dropManager.canDropFormats(drop.formats))
                    {
                        var destItem = grid.itemAt(drop.x,drop.y + grid.contentY);
                        var destLocalX = grid.mapToItem(destItem,drop.x,drop.y + grid.contentY).x
                        var realIndex = grid.indexAt(drop.x,drop.y + grid.contentY);

                        if(realIndex === -1)
                            realIndex = grid.count - 1;

                        var destIndex = destLocalX < (grid.cellWidth / 2) ? realIndex : realIndex + 1;
                        dropManager.droppedComicsForResortingAt(drop.getDataAsString(), destIndex);
                    }
                }
            }
        }

        property Component continueReadingView: Component {
            id: continueReadingView
            Rectangle {
                id: continueReadingTopView
                color: "#00000000"

                height: list.count > 0 ? main.continuReadingHeight : main.topContentMargin

                Rectangle {
                    color: continueReadingBackgroundColor

                    id: continueReadingBackground

                    width: main.width
                    height: main.continuReadingHeight - main.topContentMargin

                    visible: list.count > 0

                    Text {
                        id: continueReadingText
                        text: qsTr("Continue Reading...")
                        color: continueReadingColor
                        anchors.left: parent.left
                        anchors.top: parent.top
                        anchors.topMargin: 15
                        anchors.bottomMargin: 20
                        anchors.leftMargin: 25
                        anchors.rightMargin: 0
                        font.pointSize: 18
                        font.weight: Font.DemiBold
                    }

                    ListView {
                        id: list
                        objectName: "list"
                        anchors { top: continueReadingText.bottom; left: parent.left; right: parent.right; bottom: parent.bottom; }

                        property int previousIndex;
                        property int verticalPadding: 20

                        orientation: Qt.Horizontal
                        pixelAligned: true

                        model: comicsList

                        spacing: 20
                        anchors.topMargin: 15
                        anchors.bottomMargin: 20
                        anchors.leftMargin: 25
                        anchors.rightMargin: 20

                        WheelHandler {
                            onWheel: event => {
                                if (list.contentWidth <= list.width) {
                                    return;
                                }

                                var newValue =  Math.min(list.contentWidth - list.width - anchors.leftMargin, (Math.max(list.originX , list.contentX - event.angleDelta.y)));
                                list.contentX = newValue
                            }
                        }

                        delegate: Component {

                            //cover
                            Rectangle {
                                width: Math.floor((list.height - (list.verticalPadding * 2)) * 0.65);
                                height: list.height - (list.verticalPadding * 2);

                                color:"transparent"

                                scale: mouseArea.containsMouse ? 1.025 : 1

                                Behavior on scale {
                                    NumberAnimation { duration: 90 }
                                }

                                Image {
                                    id: coverElement
                                    anchors.fill: parent
                                    source: cover_path
                                    fillMode: Image.PreserveAspectCrop
                                    smooth: true
                                    mipmap: true
                                    asynchronous : true
                                    cache: true
                                }

                                //title
                                Text {
                                    id : comicTitleText
                                    anchors { top: coverElement.bottom; left: coverElement.left; right: coverElement.right; leftMargin: 4; rightMargin: 4; topMargin: 4; }
                                    width: itemWidth - 8
                                    maximumLineCount: 2
                                    wrapMode: Text.WordWrap
                                    text: readable_title
                                    elide: Text.ElideRight
                                    color: titleColor
                                    font.letterSpacing: fontSpacing
                                    font.pointSize: fontSize
                                    font.family: fontFamily
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
                                        openHelper.openComicFromContinueReadingList(index);
                                    }

                                    onReleased: mouse => {
                                                    list.currentIndex = index;

                                                    if(mouse.button === Qt.RightButton) // context menu is requested
                                                    {
                                                        var coordinates = main.mapFromItem(coverElement,mouseX,mouseY)
                                                        contextMenuHelper.requestedContinueReadingComicContextMenu(Qt.point(coordinates.x,coordinates.y), index);
                                                    }

                                                    mouse.accepted = true;
                                                }
                                }
                            }
                        }

                        focus: true
                    }
                }
            }
        }

        GridView {
            id:grid
            objectName: "grid"
            anchors.fill: parent
            cellHeight: cellCustomHeight
            header: continueReadingView
            focus: true
            model: foldersList
            delegate: appDelegate
            anchors.topMargin: 0
            anchors.bottomMargin: 10
            anchors.leftMargin: 0
            anchors.rightMargin: 0
            pixelAligned: true
            highlightFollowsCurrentItem: true

            currentIndex: 0
            cacheBuffer: 0

            interactive: true

            move: Transition {
                NumberAnimation { properties: "x,y"; duration: 250 }
            }

            moveDisplaced: Transition {
                NumberAnimation { properties: "x,y"; duration: 250 }
            }

            remove: Transition {
                ParallelAnimation {
                    NumberAnimation { property: "opacity"; to: 0; duration: 250 }

                }
            }

            removeDisplaced: Transition {
                NumberAnimation { properties: "x,y"; duration: 250 }
            }

            displaced: Transition {
                NumberAnimation { properties: "x,y"; duration: 250 }
            }

            function numCellsPerRow() {
                return Math.floor(width / cellCustomWidth);
            }

            onWidthChanged: {
                calculateCellWidths(cellCustomWidth);
            }

            function calculateCellWidths(cWidth) {
                var wholeCells = Math.floor(width / cWidth);
                var rest = width - (cWidth * wholeCells)

                grid.cellWidth = cWidth + Math.floor(rest / wholeCells);
            }

            WheelHandler {
                onWheel: event => {
                             if (grid.contentHeight <= grid.height) {
                                 return;
                             }

                             var newValue =  Math.min((grid.contentHeight - grid.height + grid.originY), (Math.max(grid.originY , grid.contentY - event.angleDelta.y)));
                             grid.contentY = newValue;
                         }
            }

            ScrollBar.vertical: ScrollBar {
                visible: grid.contentHeight > grid.height

                contentItem: Item {
                    implicitWidth: 12
                    implicitHeight: 26
                    Rectangle {
                        color: "#88424242"
                        anchors.fill: parent
                        anchors.topMargin: 6
                        anchors.leftMargin: 3
                        anchors.rightMargin: 2
                        anchors.bottomMargin: 6
                        border.color: "#AA313131"
                        border.width: 1
                        radius: 3.5
                    }
                }
            }
        }
    }
}
