import QtQuick 2.15

import QtQuick.Controls 2.15
import QtQuick.Layouts 1.12

import QtGraphicalEffects 1.0

import com.yacreader.ComicModel 1.0

import com.yacreader.ComicInfo 1.0
import com.yacreader.ComicDB 1.0

SplitView {
    orientation: Qt.Horizontal
    handle: Rectangle {
        border.width : 0
        implicitWidth: 10
        color: info_container.color
    }

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
    width: parent.width - (info_container.visible ? info_container.width : 0)
    SplitView.fillWidth: true
    SplitView.minimumWidth: coverWidth + 100
    height: parent.height
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

            DropShadow {
                anchors.fill: realCell
                horizontalOffset: 0
                verticalOffset: 0
                radius: 8.0
                samples: 17
                color: "#FF000000"
                source: realCell
                visible: (Qt.platform.os === "osx") ? false : true;
            }

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
                //border.color: ((dummyValue || !dummyValue) && comicsSelectionHelper.isSelectedIndex(index))?selectedBorderColor:borderColor;
                //border.width: ?1:0;
                anchors.horizontalCenter: parent.horizontalCenter

                Rectangle
                {
                    id: mouseOverBorder

                    property bool commonBorder : false

                    property int lBorderwidth : 2
                    property int rBorderwidth : 2
                    property int tBorderwidth : 2
                    property int bBorderwidth : 2

                    property int commonBorderWidth : 1

                    z : -1

                    color: "#00000000"

                    anchors
                    {
                        left: parent.left
                        right: parent.right
                        top: parent.top
                        bottom: parent.bottom

                        topMargin    : commonBorder ? -commonBorderWidth : -tBorderwidth
                        bottomMargin : commonBorder ? -commonBorderWidth : -bBorderwidth
                        leftMargin   : commonBorder ? -commonBorderWidth : -lBorderwidth
                        rightMargin  : commonBorder ? -commonBorderWidth : -rBorderwidth
                    }

                    border.color: (Qt.platform.os === "osx") ? selectedBorderColor : "#ffcc00"
                    border.width: 3

                    opacity: (dummyValue || !dummyValue) && comicsSelectionHelper.isSelectedIndex(index) ? 1 : 0

                    Behavior on opacity {
                        NumberAnimation { duration: 300 }
                    }

                    radius : 2
                }


                MouseArea  {
                    id: mouseArea
                    drag.target: realCell

                    drag.minimumX: 0
                    drag.maximumX: 0
                    drag.minimumY: 0
                    drag.maximumY: 0

                    anchors.fill: parent
                    acceptedButtons: Qt.LeftButton | Qt.RightButton

                    hoverEnabled: true

                    onDoubleClicked: {
                        comicsSelectionHelper.clear();

                        comicsSelectionHelper.selectIndex(index);
                        grid.currentIndex = index;
                        currentIndexHelper.selectedItem(index);
                    }

                    function selectAll(from,to)
                    {
                        for(var i = from;i<=to;i++)
                        {
                            comicsSelectionHelper.selectIndex(i);
                        }
                    }

                    onPressed: mouse => {
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

                        if(mouse.button === Qt.RightButton) // context menu is requested
                        {
                            if(!comicsSelectionHelper.isSelectedIndex(index)) //the context menu is requested outside the current selection, the selection will be
                            {
                                currentIndexHelper.setCurrentIndex(index)
                                grid.currentIndex = index;
                            }

                            var coordinates = main.mapFromItem(realCell,mouseX,mouseY)
                            contextMenuHelper.requestedContextMenu(Qt.point(coordinates.x,coordinates.y));
                            mouse.accepted = false;

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

                            if(mouse.button !== Qt.RightButton && !(mouse.modifiers & Qt.ControlModifier || mouse.modifiers & Qt.ShiftModifier)) //just left button click
                            {
                                if(comicsSelectionHelper.isSelectedIndex(index)) //the context menu is requested outside the current selection, the selection will be
                                {

                                }
                                else
                                {
                                    currentIndexHelper.setCurrentIndex(index)
                                }

                                grid.currentIndex = index;
                            }
                        }

                    }

                    onReleased: mouse => {
                        if(mouse.button === Qt.LeftButton && !(mouse.modifiers & Qt.ControlModifier || mouse.modifiers & Qt.ShiftModifier))
                        {
                            if(comicsSelectionHelper.isSelectedIndex(index))
                            {
                                currentIndexHelper.setCurrentIndex(index)
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
                    onPressed: {
                        console.log("rating");
                        comicsSelectionHelper.clear();
                        comicsSelectionHelper.selectIndex(index);
                        grid.currentIndex = index;
                        ratingConextMenu.popup();
                    }
                }

                Menu {
                    background: Rectangle {
                        implicitWidth: 42
                        implicitHeight: 100
                        //border.color: "#222"
                        //color: "#444"
                    }

                    id: ratingConextMenu

                    Action { text: "1"; enabled: true; icon.source:"star_menu.png"; onTriggered: comicRatingHelper.rate(index,1) }
                    Action { text: "2"; enabled: true; icon.source:"star_menu.png"; onTriggered: comicRatingHelper.rate(index,2) }
                    Action { text: "3"; enabled: true; icon.source:"star_menu.png"; onTriggered: comicRatingHelper.rate(index,3) }
                    Action { text: "4"; enabled: true; icon.source:"star_menu.png"; onTriggered: comicRatingHelper.rate(index,4) }
                    Action { text: "5"; enabled: true; icon.source:"star_menu.png"; onTriggered: comicRatingHelper.rate(index,5) }

                    delegate: MenuItem {
                        implicitHeight: 30
                    }
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

        property Component currentComicView: Component {
            id: currentComicView
            Rectangle {
                id: currentComicViewTopView
                color: "#00000000"

                height: showCurrentComic ? 270 : 20

                Rectangle {
                    color: (Qt.platform.os === "osx") ? "#88FFFFFF" : "#88000000"

                    id: currentComicVisualView

                    width: main.width
                    height: 250

                    visible: showCurrentComic

                    //cover
                    Image {
                        id: currentCoverElement
                        anchors.fill: parent

                        anchors.leftMargin: 15
                        anchors.topMargin: 15
                        anchors.bottomMargin: 15
                        anchors.rightMargin: 15
                        horizontalAlignment: Image.AlignLeft
                        anchors {horizontalCenter: parent.horizontalCenter; top: realCell.top; topMargin: 0}
                        source: comicsList.getCoverUrlPathForComicHash(currentComicInfo.hash.toString())
                        fillMode: Image.PreserveAspectFit
                        smooth: true
                        mipmap: true
                        asynchronous : true
                        cache: false //TODO clear cache only when it is needed
                    }

                    DropShadow {
                        anchors.fill: currentCoverElement
                        horizontalOffset: 0
                        verticalOffset: 0
                        radius: 8.0
                        samples: 17
                        color: "#FF000000"
                        source: currentCoverElement
                        visible: (Qt.platform.os === "osx") ? false : true;
                    }

                    ColumnLayout
                    {
                        id: currentComicInfoView

                        x: currentCoverElement.anchors.rightMargin + currentCoverElement.paintedWidth + currentCoverElement.anchors.rightMargin
                        //y: currentCoverElement.anchors.topMargin

                        anchors.top: currentCoverElement.top
                        anchors.right: parent.right
                        anchors.left: readButton.left

                        spacing: 9

                        Text {
                            Layout.topMargin: 7
                            Layout.fillWidth: true
                            Layout.rightMargin: 20

                            Layout.alignment: Qt.AlignTop | Qt.AlignLeft

                            id: currentComicInfoTitleView

                            color: infoTitleColor
                            font.family: "Arial"
                            font.bold: true
                            font.pixelSize: 21
                            wrapMode: Text.WordWrap

                            text: currentComic.getTitleIncludingNumber()
                        }

                        Flow {
                            spacing: 0
                            Layout.alignment: Qt.AlignTop | Qt.AlignLeft
                            Layout.fillWidth: true
                            Layout.fillHeight: false

                            id: currentComicDetailsFlowView
                            property font infoFont: Qt.font({
                                                                family: "Arial",
                                                                pixelSize: 14
                                                            });
                            property string infoFlowTextColor: infoTextColor

                            Text {
                                id: currentComicInfoVolume
                                color: currentComicDetailsFlowView.infoFlowTextColor
                                font: currentComicDetailsFlowView.infoFont
                                text: currentComicInfo.volume ? currentComicInfo.volume : ""
                                rightPadding: 20
                                visible: currentComicInfo.volume ? true : false
                            }

                            Text {
                                id: currentComicInfoNumbering
                                color: currentComicDetailsFlowView.infoFlowTextColor
                                font: currentComicDetailsFlowView.infoFont
                                text: currentComicInfo.number + "/" + currentComicInfo.count
                                rightPadding: 20
                                visible : currentComicInfo.number ? true : false
                            }

                            Text {
                                id: currentComicInfoGenre
                                color: currentComicDetailsFlowView.infoFlowTextColor
                                font: currentComicDetailsFlowView.infoFont
                                text: currentComicInfo.genere ? currentComicInfo.genere : ""
                                rightPadding: 20
                                visible: currentComicInfo.genere ? true : false
                            }

                            Text {
                                id: currentComicInfoDate
                                color: currentComicDetailsFlowView.infoFlowTextColor
                                font: currentComicDetailsFlowView.infoFont
                                text: currentComicInfo.date ? currentComicInfo.date : ""
                                rightPadding: 20
                                visible: currentComicInfo.date ? true : false
                            }

                            Text {
                                id: currentComicInfoPages
                                color: currentComicDetailsFlowView.infoFlowTextColor
                                font: currentComicDetailsFlowView.infoFont
                                text: (currentComicInfo.numPages ? currentComicInfo.numPages : "") + " pages"
                                rightPadding: 20
                                visible: currentComicInfo.numPages ? true : false
                            }

                            Text {
                                id: currentComicInfoShowInComicVine
                                font: currentComicDetailsFlowView.infoFont
                                color: "#ffcc00"
                                text: "Show in Comic Vine"
                                visible: currentComicInfo.comicVineID ? true : false
                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: {
                                        Qt.openUrlExternally("http://www.comicvine.com/comic/4000-%1/".arg(comicInfo.comicVineID));
                                    }
                                }
                            }
                        }

                        ScrollView {
                            Layout.topMargin: 6
                            Layout.rightMargin: 30
                            Layout.bottomMargin: 5
                            Layout.fillWidth: true
                            Layout.maximumHeight: (currentComicVisualView.height * 0.32)
                            Layout.maximumWidth: 960

                            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

                            contentItem: currentComicInfoSinopsis

                            id: synopsisScroller

                            clip: true

                            Text {
                                Layout.maximumWidth: 960

                                width: synopsisScroller.width

                                id: currentComicInfoSinopsis
                                color: infoTitleColor
                                font.family: "Arial"
                                font.pixelSize: 14
                                wrapMode: Text.WordWrap

                                text: '<html><head><style>
                                a {
                                    color: #FFCB00;
                                    text-decoration:none;
                                }
                                </style></head><body>' + currentComicInfo.synopsis + '</body></html>'
                                visible: currentComicInfo.synopsis
                                textFormat: Text.RichText
                            }
                        }
                    }

                    Button {
                        text: "Read"
                        id: readButton
                        x: currentCoverElement.anchors.rightMargin + currentCoverElement.paintedWidth + currentCoverElement.anchors.rightMargin
                        anchors.bottom: currentCoverElement.bottom
                        anchors.bottomMargin: 15

                        onClicked: comicOpener.triggerOpenCurrentComic()
                            background: Rectangle {
                                implicitWidth: 100
                                implicitHeight: 30
                                border.width: readButton.activeFocus ? 2 : 1
                                border.color: "#FFCC00"
                                radius: height / 2
                                color: "#FFCC00"
                            }

                            contentItem: Text {
                                renderType: Text.NativeRendering
                                verticalAlignment: Text.AlignVCenter
                                horizontalAlignment: Text.AlignHCenter
                                font.family: "Arial"
                                font.pointSize: 12
                                font.bold: true
                                color: "white"
                                text: readButton.text
                            }
                    }


                    DropShadow {
                        anchors.fill: readButton
                        horizontalOffset: 0
                        verticalOffset: 0
                        radius: 8.0
                        samples: 17
                        color: "#AA000000"
                        source: readButton
                        visible: ((Qt.platform.os === "osx") ? false : true) && !readButton.pressed
                    }
                }
            }
        }

        GridView {
            id:grid
            objectName: "grid"
            anchors.fill: parent
            cellHeight: cellCustomHeight
            header: currentComicView
            focus: true
            model: comicsList
            delegate: appDelegate
            anchors.topMargin: 0
            anchors.bottomMargin: 10
            anchors.leftMargin: 0
            anchors.rightMargin: 0
            pixelAligned: true
            highlightFollowsCurrentItem: true

            currentIndex: 0
            cacheBuffer: 0

            //disable flickable behaviour
            interactive: false

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
               onWheel: {
                   if (grid.contentHeight <= grid.height) {
                       return;
                   }

                   var newValue =  Math.min((grid.contentHeight - grid.height - (showCurrentComic ? 270 : 20)), (Math.max(grid.originY , grid.contentY - event.angleDelta.y)));
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

           Keys.onPressed: {
               if (event.modifiers & Qt.ControlModifier || event.modifiers & Qt.ShiftModifier) {
                   event.accepted = true
                   return;
               }

               var numCells = grid.numCellsPerRow();
               var ci = 0;
               if (event.key === Qt.Key_Right) {
                   ci = Math.min(grid.currentIndex+1,grid.count - 1);
               }
               else if (event.key === Qt.Key_Left) {
                   ci = Math.max(0,grid.currentIndex-1);
               }
               else if (event.key === Qt.Key_Up) {
                   ci = Math.max(0,grid.currentIndex-numCells);
               }
               else if (event.key === Qt.Key_Down) {
                   ci = Math.min(grid.currentIndex+numCells,grid.count - 1);
               } else {
                   return;
               }

               event.accepted = true;
               grid.currentIndex = -1
               comicsSelectionHelper.clear();
               currentIndexHelper.setCurrentIndex(ci);
               grid.currentIndex = ci;
           }
        }
    }
}

Rectangle {
    id: info_container
    objectName: "infoContainer"
    SplitView.preferredWidth: 350
    SplitView.minimumWidth: 350
    SplitView.maximumWidth: 960
    height: parent.height

    color: infoBackgroundColor

    visible: showInfo

    Flickable{
        id: infoFlickable
        anchors.fill: parent
        anchors.margins: 0

        contentWidth: infoView.width
        contentHeight: infoView.height

        ComicInfoView {
            id: infoView
            width: info_container.width
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
}
