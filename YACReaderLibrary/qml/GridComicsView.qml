import QtQuick 2.3

import QtQuick.Controls 1.2
import comicModel 1.0

Rectangle {
    id: main
    color: backgroundColor
    width: parent.width
    height: parent.height
    anchors.margins: 0

    function selectAll(from,to)
    {
        for(var i = from;i<=to;i++)
        {
            comicsSelectionHelper.selectIndex(i);
        }
    }

    Component {
        id: appDelegate
        Rectangle
        {
            id: cell
            width: grid.cellWidth
            height: grid.cellHeight
            color: backgroundColor


            Rectangle {
                id: realCell

                property int position : 0
                property bool dragging: false;
                Drag.active: mouseArea.drag.active
                Drag.hotSpot.x: 32
                Drag.hotSpot.y: 32
                Drag.dragType: Drag.Automatic
                //Drag.mimeData: { "x": 1 }
                Drag.proposedAction: Qt.CopyAction
                Drag.onActiveChanged: {
                    if(!dragging)
                    {
                        comicsSelectionHelper.startDrag();
                        dragging = true;
                    }else
                        dragging = false;
                }

                width: 156; height: 287
                color: ((dummyValue || !dummyValue) && comicsSelectionHelper.isSelectedIndex(index))?selectedColor:cellColor;
                border.color: ((dummyValue || !dummyValue) && comicsSelectionHelper.isSelectedIndex(index))?selectedBorderColor:borderColor;
                border.width: (Qt.platform.os === "osx")?1:0;

                anchors.horizontalCenter: parent.horizontalCenter

                MouseArea  {
                    id: mouseArea
                    drag.target: realCell

                    drag.minimumX: 0
                    drag.maximumX: 0
                    drag.minimumY: 0
                    drag.maximumY: 0

                    anchors.fill: parent
                    acceptedButtons: Qt.LeftButton | Qt.RightButton

                    onDoubleClicked: {
                        comicsSelectionHelper.clear();

                        comicsSelectionHelper.selectIndex(index);
                        grid.currentIndex = index;
                        comicsSelectionHelper.selectedItem(index);
                    }

                    onPressed: {
                        //grid.currentIndex = index
                        //comicsSelection.setCurrentIndex(index,0x0002)
                        var ci = grid.currentIndex;
                        /*if(mouse.button != Qt.RightButton && !(mouse.modifiers & Qt.ControlModifier || mouse.modifiers & Qt.ShiftModifier))
                        {
                            if(!comicsSelectionHelper.isSelectedIndex(index))
                                comicsSelectionHelper.clear();
                        }*/

                        if(mouse.modifiers & Qt.ShiftModifier)
                            if(index < ci)
                            {
                                selectAll(index,ci);
                                grid.currentIndex = index;
                            }
                            else if (index > ci)
                            {
                                selectAll(ci,index);
                                grid.currentIndex = index;
                            }

                        mouse.accepted = true;

                        if(mouse.button == Qt.RightButton)
                        {

                            if(!comicsSelectionHelper.isSelectedIndex(index))
                            {
                                comicsSelectionHelper.setCurrentIndex(index)
                                grid.currentIndex = index;
                            }

                            var coordinates = main.mapFromItem(realCell,mouseX,mouseY)
                            contextMenuHelper.requestedContextMenu(Qt.point(coordinates.x,coordinates.y));

                        } else
                        {
                            if(mouse.modifiers & Qt.ControlModifier)
                            {
                                if(comicsSelectionHelper.isSelectedIndex(index))
                                {
                                    if(comicsSelectionHelper.numItemsSelected()>1)
                                    {
                                        comicsSelectionHelper.deselectIndex(index);
                                        if(grid.currentIndex === index)
                                            grid.currentIndex = comicsSelectionHelper.lastSelectedIndex();
                                    }
                                }
                                else
                                {
                                    comicsSelectionHelper.selectIndex(index);
                                    grid.currentIndex = index;
                                }
                            }
                        }

                    }

                    onReleased: {
                        if(mouse.button != Qt.RightButton && !(mouse.modifiers & Qt.ControlModifier || mouse.modifiers & Qt.ShiftModifier))
                        {
                            comicsSelectionHelper.setCurrentIndex(index)
                            grid.currentIndex = index;
                        }
                    }

                }

            }

            /**/

            //cover
            Image {
                id: coverElement
                width: 148
                height: 224
                anchors {horizontalCenter: parent.horizontalCenter; top: realCell.top; topMargin: 4}
                source: cover_path
                fillMode: Image.PreserveAspectCrop
                smooth: true
                mipmap: true
                asynchronous : true
                cache: false //TODO clear cache only when it is neede
            }
            //mark
            Image {
                id: mark
                width: 23
                height: 23
                source: read_column&&show_marks?"tick.png":has_been_opened&&show_marks?"reading.png":""
                anchors {right: coverElement.right; top: coverElement.top; topMargin: 11; rightMargin: 11}
                asynchronous : true
            }

            //title
            Text {
                id : titleText
                anchors { top: realCell.top; left: realCell.left; leftMargin: 4; rightMargin: 4; topMargin: 234; }
                width: 148
                maximumLineCount: 2
                wrapMode: Text.WordWrap
                text: title
                elide: Text.ElideRight
                color: titleColor
                clip: true
                font.letterSpacing: fontSpacing
                font.pointSize: fontSize
                font.family: fontFamily
            }
            //number
            Text {
                anchors {bottom: realCell.bottom; left: realCell.left; margins: 4}
                text: number?"<b>#</b>"+number:""
                color: textColor
                font.letterSpacing: fontSpacing
                font.pointSize: fontSize
                font.family: fontFamily
            }
            //page icon
            Image {
                id: pageImage
                anchors {bottom: realCell.bottom; right: realCell.right; bottomMargin: 5; rightMargin: 4; leftMargin: 4}
                source: "page.png"
            }
            //numPages
            Text {
                id: pages
                anchors {bottom: realCell.bottom; right: pageImage.left; margins: 4}
                text: has_been_opened?current_page+"/"+num_pages:num_pages
                color: textColor
                font.letterSpacing: fontSpacing
                font.pointSize: fontSize
                font.family: fontFamily
            }
            //rating icon
            Image {
                id: ratingImage
                anchors {bottom: realCell.bottom; right: pageImage.left; bottomMargin: 5; rightMargin: Math.floor(pages.width)+12}
                source: "star.png"

                MouseArea  {
                    anchors.fill: parent
                    onClicked: {
                        console.log("rating");
                        comicsSelectionHelper.clear();
                        comicsSelectionHelper.selectIndex(index);
                        grid.currentIndex = index;
                        ratingConextMenu.popup();

                    }
                }

                Menu {
                    id: ratingConextMenu
                    MenuItem { text: "1"; enabled: true; iconSource:"star_menu.png"; onTriggered: comicRatingHelper.rate(index,1) }
                    MenuItem { text: "2"; enabled: true; iconSource:"star_menu.png"; onTriggered: comicRatingHelper.rate(index,2) }
                    MenuItem { text: "3"; enabled: true; iconSource:"star_menu.png"; onTriggered: comicRatingHelper.rate(index,3) }
                    MenuItem { text: "4"; enabled: true; iconSource:"star_menu.png"; onTriggered: comicRatingHelper.rate(index,4) }
                    MenuItem { text: "5"; enabled: true; iconSource:"star_menu.png"; onTriggered: comicRatingHelper.rate(index,5) }

                }
            }

            //comic rating
            Text {
                id: comicRating
                anchors {bottom: realCell.bottom; right: ratingImage.left; margins: 4}
                text: rating>0?rating:"-"
                color: textColor
            }
        }
    }

    YACReaderScrollView{
        id: scrollView
        anchors.fill: parent
        anchors.margins: 0


        GridView {
            id:grid
            anchors.fill: parent
            cellHeight: 295
            highlight: appHighlight
            focus: true
            model: comicsList
            delegate: appDelegate
            anchors.topMargin: 20
            anchors.bottomMargin: 20
            anchors.leftMargin: 10
            anchors.rightMargin: 10
            pixelAligned: true
            //flickDeceleration: -2000
            snapMode: GridView.SnapToRow
            currentIndex: 0
            cacheBuffer: 0


            function numCellsPerRow() {
                return Math.floor(width / 185);
            }

            onWidthChanged: {
                var numCells = numCellsPerRow();
                var rest = width % 185;

                if(numCells > 0)
                {
                    cellWidth = Math.floor(width / numCells) ;
                    //console.log("numCells=",numCells,"rest=",rest,"cellWidth=",cellWidth,"width=",width);
                }
            }
        }
        focus: true
        Keys.onPressed: {
            if (event.modifiers & Qt.ControlModifier || event.modifiers & Qt.ShiftModifier)
                return;
            var numCells = grid.numCellsPerRow();
            var ci
            if (event.key === Qt.Key_Right) {
                ci = Math.min(grid.currentIndex+1,grid.count);
            }
            else if (event.key === Qt.Key_Left) {
                ci = Math.max(0,grid.currentIndex-1);
            }
            else if (event.key === Qt.Key_Up) {
                ci = Math.max(0,grid.currentIndex-numCells);
            }
            else if (event.key === Qt.Key_Down) {
                ci = Math.min(grid.currentIndex+numCells,grid.count);
            }

            event.accepted = true;
            //var ci = grid.currentIndex;
            grid.currentIndex = -1
            comicsSelectionHelper.clear();
            comicsSelectionHelper.setCurrentIndex(ci);
            grid.currentIndex = ci;
        }
        //}

        /*MouseArea {
        anchors.fill: parent
        onClicked: {
                        clicked.accepted = false;
            console.log("xx");
        }

        onWheel: {
            var newValue =  Math.max(0,scrollView.flickableItem.contentY - wheel.angleDelta.y)
            scrollView.flickableItem.contentY = newValue

        }
    }*/
        /*ScrollBar {
        flickable: grid;
    }

    PerformanceMeter {
        anchors {top: parent.top; left: parent.left; margins: 4}
        id: performanceMeter
        width: 128
        height: 64
        enabled: (dummyValue || !dummyValue)
    }*/
    }
}


