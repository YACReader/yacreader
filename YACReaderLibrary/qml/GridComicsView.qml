import QtQuick 2.3

import QtQuick.Controls 1.2
import comicModel 1.0
import QtGraphicalEffects 1.0

Rectangle {
    id: main
    clip: true

    Image {
        id: backgroundImg
        anchors.fill: parent
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
            color: "#00000000"


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
                        dragManager.startDrag();
                        dragging = true;
                    }else
                        dragging = false;
                }

                width: itemWidth
                height: itemHeight

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

                        var ci = grid.currentIndex; //save current index

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

                        if(mouse.button == Qt.RightButton) // context menu is requested
                        {

                            if(!comicsSelectionHelper.isSelectedIndex(index)) //the context menu is requested outside the current selection, the selection will be
                            {
                                comicsSelectionHelper.setCurrentIndex(index)
                                grid.currentIndex = index;
                            }

                            var coordinates = main.mapFromItem(realCell,mouseX,mouseY)
                            contextMenuHelper.requestedContextMenu(Qt.point(coordinates.x,coordinates.y));

                        } else //left button
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

                            if(mouse.button != Qt.RightButton && !(mouse.modifiers & Qt.ControlModifier || mouse.modifiers & Qt.ShiftModifier)) //just left button click
                            {
                                if(comicsSelectionHelper.isSelectedIndex(index)) //the context menu is requested outside the current selection, the selection will be
                                {

                                }
                                else
                                {
                                    comicsSelectionHelper.setCurrentIndex(index)
                                }

                                grid.currentIndex = index;
                            }
                        }

                    }

                    onReleased: {
                        if(mouse.button == Qt.LeftButton && !(mouse.modifiers & Qt.ControlModifier || mouse.modifiers & Qt.ShiftModifier))
                        {
                            if(comicsSelectionHelper.isSelectedIndex(index))
                            {
                                comicsSelectionHelper.setCurrentIndex(index)
                                grid.currentIndex = index;
                            }
                        }
                    }
                }
            }

            /**/

            //cover
            Image {
                id: coverElement
                width: coverWidth
                height: coverHeight
                anchors {horizontalCenter: parent.horizontalCenter; top: realCell.top; topMargin: 0}
                source: cover_path
                fillMode: Image.PreserveAspectCrop
                smooth: true
                mipmap: true
                asynchronous : true
                cache: false //TODO clear cache only when it is needed

            }

            //border
            Rectangle {
                width: coverElement.width
                height: coverElement.height
                anchors {horizontalCenter: parent.horizontalCenter; top: realCell.top; topMargin: 0}
                color: "transparent"
                border {
                    color: "#20FFFFFF"
                    width: 1
                }
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

            //title
            Text {
                id : titleText
                anchors { top: coverElement.bottom; left: realCell.left; leftMargin: 4; rightMargin: 4; topMargin: 4; }
                width: itemWidth - 8
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
                width: 8
                height: 10
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
                width: 13
                height: 11

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

    ScrollView{
        id: scrollView
        anchors.fill: parent
        anchors.margins: 0

        //QTBUG-39453
        //Another fu%$·#& bug in Qt
        //https://bugreports.qt.io/browse/QTBUG-39453
        //To solve this I am going to accept any input drag, drops will be filtered in "onDropped"
        DropArea {
            anchors.fill: parent

            /*
            onEntered: {
                console.log("onEntered");
                if(drag.hasUrls)
                {
                    console.log("HAS URLS -> ", drag.urls);
                    if(dropManager.canDropUrls(drag.urls, drag.action))
                    {
                        drag.accepted = true;
                        console.log("canDropUrls");
                    }else
                        drag.accepted = false;
                }
                else if (dropManager.canDropFormats(drag.formats)) {
                    drag.accepted = true;
                    console.log("canDropFormats");
                } else
                    drag.accepted = false;
            }*/


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

        GridView {
            id:grid
            objectName: "grid"
            anchors.fill: parent
            cellHeight: cellCustomHeight
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

            footer: Rectangle { //fix for the scroll issue, TODO find what causes the issue (some times the bottoms cells are hidden for the toolbar, no full scroll)
                height : 25
                width : parent.width
                color : "#00000000"
            }

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
               console.log("cWidth",cWidth,"wholeCells=",wholeCells,"rest=",rest,"cellWidth=",cellWidth,"width=",width);
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


