import QtQuick

import QtQuick.Controls
import QtQuick.Layouts

import QtQuick.Effects

import com.yacreader.ComicModel 1.0

import com.yacreader.ComicInfo 1.0
import com.yacreader.ComicDB 1.0
import com.yacreader.GridContentModel 1.0

import QtQuick.Controls.Basic
import QtQml.Models

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
            layer.enabled: true
            visible: false
        }

        MultiEffect {
            anchors.fill: backgroundImg
            source: backgroundImg
            blurEnabled: true
            blur: 1.0
            blurMax: Math.max(2, backgroundBlurRadius)
            opacity: backgroundBlurOpacity
            visible: backgroundBlurVisible
        }

        color: backgroundColor
        width: parent.width - (info_container.visible ? info_container.width : 0)
        SplitView.fillWidth: true
        SplitView.minimumWidth: coverWidth + 100
        height: parent.height
        anchors.margins: 0

        DelegateChooser {
            id: appDelegate
            role: "item_kind"

            DelegateChoice {
                roleValue: GridContentModel.FolderItem

                FolderGridDelegate {
                    id: folderCell
                    width: grid.cellWidth
                    height: grid.cellHeight
                    selected: currentIndexHelper.focusedFolderRow === index

                    onFocusRequested: {
                        comicsSelectionHelper.clear()
                        grid.focusItemFromPointer(index)
                    }
                    onOpenRequested: currentIndexHelper.openFolder(index)
                    onContextMenuRequested: localPosition => {
                        var coordinates = main.mapFromItem(folderCell.interactionItem,
                                                           localPosition.x,
                                                           localPosition.y)
                        contextMenuHelper.requestItemContextMenu(Qt.point(coordinates.x, coordinates.y), folderCell.index)
                    }
                }
            }

            DelegateChoice {
                roleValue: GridContentModel.ComicItem

                ComicGridDelegate {
                    id: comicCell
                    width: grid.cellWidth
                    height: grid.cellHeight
                    currentViewIndex: grid.currentIndex
                    selectionHelper: comicsSelectionHelper

                    onActivateRequested: viewRow => currentIndexHelper.activateItem(viewRow)
                    onClearFolderFocusRequested: currentIndexHelper.clearFolderFocus()
                    onContextMenuRequested: localPosition => {
                        var coordinates = main.mapFromItem(comicCell.interactionItem,
                                                           localPosition.x,
                                                           localPosition.y)
                        contextMenuHelper.requestItemContextMenu(Qt.point(coordinates.x, coordinates.y), comicCell.index)
                    }
                    onFocusViewRowRequested: viewRow => grid.focusItemFromPointer(viewRow)
                    onRateRequested: (sourceRow, rating) => comicRatingHelper.rate(sourceRow, rating)
                    onSelectRangeRequested: (from, to) => currentIndexHelper.selectComicRange(from, to)
                    onSetCurrentViewRowRequested: viewRow => grid.setCurrentIndexFromPointer(viewRow)
                    onSetCurrentComicRowRequested: sourceRow => {
                        grid.setCurrentIndexFromPointer(currentIndexHelper.viewRowForComicRow(sourceRow))
                    }
                    onStartDragRequested: dragManager.startDrag()
                }
            }

            DelegateChoice {
                roleValue: GridContentModel.SpacerItem
                Item {
                    width: grid.cellWidth
                    height: grid.cellHeight
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

            function capturePosition() {
                const probeX = Math.max(1, grid.cellWidth / 2)
                const viewRow = grid.indexAt(probeX, grid.contentY + 1)
                if (viewRow < 0) {
                    return {
                        "header": true,
                        "viewRow": -1,
                        "offset": grid.contentY - grid.originY,
                        "itemExtent": Math.max(1, -grid.originY)
                    }
                }

                const item = grid.itemAtIndex(viewRow)
                return {
                    "header": false,
                    "viewRow": viewRow,
                    "offset": item ? grid.contentY - item.y : 0,
                    "itemExtent": grid.cellHeight
                }
            }

            function restorePosition(viewRow, offset, oldItemExtent) {
                if (viewRow < 0) {
                    const currentExtent = Math.max(1, -grid.originY)
                    const restoredOffset = oldItemExtent === currentExtent
                            ? offset
                            : offset * currentExtent / Math.max(1, oldItemExtent)
                    grid.contentY = grid.originY + restoredOffset
                    return
                }

                grid.positionViewAtIndex(viewRow, GridView.Beginning)
                const restoredOffset = oldItemExtent === grid.cellHeight
                        ? offset
                        : offset * grid.cellHeight / Math.max(1, oldItemExtent)
                const maximumY = Math.max(grid.originY, grid.contentHeight - grid.height + grid.originY)
                grid.contentY = Math.max(grid.originY, Math.min(maximumY, grid.contentY + restoredOffset))
            }

            property Component currentComicView: Component {
                id: currentComicView
                Rectangle {
                    id: currentComicViewTopView
                    color: "#00000000"

                    function handlesWheelAt(position) {
                        const synopsisPosition = synopsisScroller.mapFromItem(currentComicViewTopView,
                                                                              position.x,
                                                                              position.y)
                        return synopsisPosition.x >= 0 && synopsisPosition.x <= synopsisScroller.width
                                && synopsisPosition.y >= 0 && synopsisPosition.y <= synopsisScroller.height
                                && synopsisScroller.contentHeight > synopsisScroller.availableHeight
                    }

                    height: currentIndexHelper.currentComicBannerVisible ? 270 : 20

                    Rectangle {
                        color: currentComicBackgroundColor

                        id: currentComicVisualView

                        width: main.width
                        height: 250

                        visible: currentIndexHelper.currentComicBannerVisible

                        //cover
                        Image {
                            id: currentCoverElement
                            anchors.fill: parent

                            anchors.leftMargin: 15
                            anchors.topMargin: 15
                            anchors.bottomMargin: 15
                            anchors.rightMargin: 15
                            horizontalAlignment: Image.AlignLeft
                            anchors {horizontalCenter: parent.horizontalCenter; top: parent.top; topMargin: 0}
                            source: comicsList.comicCoverUrlForHash(currentComicInfo.hash.toString())
                            fillMode: Image.PreserveAspectFit
                            smooth: true
                            mipmap: true
                            asynchronous : true
                            cache: false //TODO clear cache only when it is needed

                            layer.enabled: showDropShadow
                            layer.effect: MultiEffect {
                                shadowEnabled: true
                                shadowColor: currentComicCoverShadowColor
                                shadowBlur: 1.0
                                blurMax: 8
                                shadowHorizontalOffset: 0
                                shadowVerticalOffset: 0
                            }
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

                                color: infoTextColor
                                font.family: "Arial"
                                font.bold: true
                                font.pixelSize: 21
                                wrapMode: Text.WordWrap

                                text: currentComic?.getTitleIncludingNumber() ?? ""
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
                                property string infoFlowTextColor: infoMetadataTextColor

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
                                    id: currentComicInfoArc
                                    color: currentComicDetailsFlowView.infoFlowTextColor
                                    font: currentComicDetailsFlowView.infoFont
                                    text: currentComicInfo.getStoryArcInfoString()
                                    rightPadding: 20
                                    visible : currentComicInfo.getStoryArcInfoString().length > 0
                                }

                                Text {
                                    id: currentComicInfoAlternate
                                    color: currentComicDetailsFlowView.infoFlowTextColor
                                    font: currentComicDetailsFlowView.infoFont
                                    text: currentComicInfo.getAlternateSeriesString()
                                    rightPadding: 20
                                    visible : currentComicInfo.getStoryArcInfoString().length > 0
                                }

                                Text {
                                    id: currentComicInfoSeriesGroup
                                    color: currentComicDetailsFlowView.infoFlowTextColor
                                    font: currentComicDetailsFlowView.infoFont
                                    text: currentComicInfo.seriesGroup ? currentComicInfo.seriesGroup : ""
                                    rightPadding: 20
                                    visible: currentComicInfo.seriesGroup ? true : false
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
                                    color: themeLinkColor
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

                                contentWidth: -1
                                contentItem: currentComicInfoSinopsis

                                id: synopsisScroller

                                clip: true

                                Text {
                                    Layout.maximumWidth: 960

                                    width: synopsisScroller.width

                                    id: currentComicInfoSinopsis
                                    color: infoTextColor
                                    font.family: "Arial"
                                    font.pixelSize: 14
                                    wrapMode: Text.WordWrap

                                    text: '<html><head><style>a { color: ' + themeLinkColorStr + '; text-decoration: none; }</style></head><body>' + (currentComicInfo.synopsis ?? "") + '</body></html>'
                                    visible: currentComicInfo.synopsis ?? false
                                                                          textFormat: Text.RichText
                                }
                            }
                        }

                        Button {
                            containmentMask: null
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
                                border.color: buttonColor
                                radius: height / 2
                                color: buttonColor
                            }

                            contentItem: Text {
                                renderType: Text.NativeRendering
                                verticalAlignment: Text.AlignVCenter
                                horizontalAlignment: Text.AlignHCenter
                                font.family: "Arial"
                                font.pointSize: 12
                                font.bold: true
                                color: buttonTextColor
                                text: readButton.text
                            }

                            layer.enabled: showDropShadow && !readButton.pressed
                            layer.effect: MultiEffect {
                                shadowEnabled: true
                                shadowColor: buttonShadowColor
                                shadowBlur: 1.0
                                blurMax: 8
                                shadowHorizontalOffset: 0
                                shadowVerticalOffset: 0
                            }
                        }
                    }
                }
            }

            property Component rootFolderHeader: Component {
                ContinueReadingGridHeader {
                    id: continueReadingHeader
                    width: main.width
                    contentModel: currentIndexHelper.rootContinueReadingModel
                    sectionVisible: currentIndexHelper.globalContinueReadingEnabled
                    onOpenRequested: index => currentIndexHelper.openContinueReadingComic(index)
                    onContextMenuRequested: (index, position) => {
                        var coordinates = main.mapFromItem(continueReadingHeader, position.x, position.y)
                        currentIndexHelper.requestContinueReadingComicContextMenu(Qt.point(coordinates.x, coordinates.y), index)
                    }
                }
            }

            GridView {
                id:grid
                objectName: "grid"
                anchors.fill: parent
                cellHeight: cellCustomHeight
                header: currentIndexHelper.rootFolder ? scrollView.rootFolderHeader : scrollView.currentComicView
                focus: true
                model: comicsList
                delegate: appDelegate
                anchors.topMargin: 0
                anchors.bottomMargin: 10
                anchors.leftMargin: 0
                anchors.rightMargin: 0
                pixelAligned: true
                highlightFollowsCurrentItem: true

                currentIndex: -1
                cacheBuffer: 0
                readonly property var wheelAwareHeader: headerItem

                interactive: true

                // Clears the selection when the click does not land on an item. The
                // MouseArea must be a child of the view content item, because a plain
                // child of the view stays behind the flickable itself and never gets
                // mouse events. Its geometry follows the viewport (in content
                // coordinates), so the empty space after the last row is included too.
                MouseArea {
                    parent: grid.contentItem
                    z: -1
                    x: grid.contentX
                    y: grid.contentY
                    width: grid.width
                    height: grid.height
                    acceptedButtons: Qt.LeftButton

                    onClicked: mouse => {
                        if (mouse.modifiers !== Qt.NoModifier)
                            return

                        comicsSelectionHelper.clear()
                        currentIndexHelper.clearFolderFocus()
                        grid.currentIndex = -1
                        grid.forceActiveFocus()
                    }
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

                function firstVisibleSelectableIndex() {
                    if (count === 0)
                        return -1

                    const columns = Math.max(1, numCellsPerRow())
                    const visibleRow = Math.max(0, Math.floor((contentY - originY) / cellHeight))
                    const candidate = Math.min(visibleRow * columns, count - 1)
                    return currentIndexHelper.nearestSelectableRow(candidate, 1)
                }

                function setCurrentIndexFromPointer(index) {
                    var previousContentX = contentX
                    var previousContentY = contentY
                    currentIndex = index
                    contentX = previousContentX
                    contentY = previousContentY
                }

                function focusItemFromPointer(index) {
                    var previousContentX = contentX
                    var previousContentY = contentY
                    currentIndexHelper.focusItem(index)
                    currentIndex = index
                    contentX = previousContentX
                    contentY = previousContentY
                }

                onWidthChanged: {
                    calculateCellWidths(cellCustomWidth);
                }

                function calculateCellWidths(cWidth) {
                    var wholeCells = Math.max(1, Math.floor(width / cWidth));
                    var rest = width - (cWidth * wholeCells)

                    grid.cellWidth = cWidth + Math.floor(rest / wholeCells);
                    currentIndexHelper.setGridColumnCount(wholeCells)
                }

                MouseArea {
                    id: gridWheelArea
                    anchors.fill: parent
                    acceptedButtons: Qt.NoButton

                    onWheel: (wheel) => {
                        if (grid.wheelAwareHeader && grid.wheelAwareHeader.handlesWheelAt) {
                            const headerPosition = grid.wheelAwareHeader.mapFromItem(gridWheelArea,
                                                                                    wheel.x,
                                                                                    wheel.y)
                            if (grid.wheelAwareHeader.handlesWheelAt(headerPosition)) {
                                wheel.accepted = false
                                return
                            }
                        }

                        if (grid.contentHeight <= grid.height)
                            return

                            var newValue = Math.min(
                                (grid.contentHeight - grid.height + grid.originY),
                                                    Math.max(grid.originY, grid.contentY - wheel.angleDelta.y)
                            )
                            grid.contentY = newValue
                    }
                }

                ScrollBar.vertical: ScrollBar {
                    visible: grid.contentHeight > grid.height

                    contentItem: Item {
                        implicitWidth: 12
                        implicitHeight: 26
                        Rectangle {
                            color: scrollbarColor
                            anchors.fill: parent
                            anchors.topMargin: 6
                            anchors.leftMargin: 3
                            anchors.rightMargin: 2
                            anchors.bottomMargin: 6
                            border.color: scrollbarBorderColor
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

                    if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
                        event.accepted = true
                        currentIndexHelper.activateItem(grid.currentIndex)
                        return
                    }

                    const cursorKey = event.key === Qt.Key_Right || event.key === Qt.Key_Left
                                      || event.key === Qt.Key_Up || event.key === Qt.Key_Down
                    if (cursorKey && grid.currentIndex < 0) {
                        const initialIndex = grid.firstVisibleSelectableIndex()
                        if (initialIndex >= 0) {
                            comicsSelectionHelper.clear()
                            currentIndexHelper.focusItem(initialIndex)
                            grid.currentIndex = initialIndex
                        }
                        event.accepted = true
                        return
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

                    ci = currentIndexHelper.nearestSelectableRow(ci,
                                                               event.key === Qt.Key_Left || event.key === Qt.Key_Up ? -1 : 1)

                    event.accepted = true;
                    grid.currentIndex = -1
                    comicsSelectionHelper.clear();
                    currentIndexHelper.focusItem(ci);
                    grid.currentIndex = ci;
                }

                DropArea {
                    id: gridDropArea
                    anchors.fill: parent

                    function visibleItemIndexAt(x, y) {
                        const contentPosition = grid.contentItem.mapFromItem(gridDropArea, x, y)
                        const viewIndex = grid.indexAt(contentPosition.x, contentPosition.y)
                        if (viewIndex < 0)
                            return -1

                        const item = grid.itemAtIndex(viewIndex)
                        if (!item || !item.interactionItem)
                            return -1

                        const localPosition = item.interactionItem.mapFromItem(gridDropArea, x, y)
                        return localPosition.x >= 0
                                && localPosition.x <= item.interactionItem.width
                                && localPosition.y >= 0
                                && localPosition.y <= item.interactionItem.height
                                ? viewIndex
                                : -1
                    }

                    onEntered: drag => {
                                   if(drag.hasUrls)
                                   {
                                       if(dropManager.canDropImage(drag.urls)
                                               || dropManager.canDropUrls(drag.urls, drag.action))
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

                    onDropped: drop => {
                                   if(drop.hasUrls && dropManager.canDropImage(drop.urls))
                                   {
                                       var coverIndex = gridDropArea.visibleItemIndexAt(drop.x, drop.y);
                                       if (coverIndex !== -1) {
                                           dropManager.droppedImageAt(drop.urls, coverIndex);
                                           drop.accepted = true;
                                       }
                                   }
                                   else if(drop.hasUrls && dropManager.canDropUrls(drop.urls, drop.action))
                                   {
                                       dropManager.droppedFiles(drop.urls, drop.action);
                                   }
                                   else{
                                       if (dropManager.canDropFormats(drop.formats))
                                       {
                                           var realIndex = grid.indexAt(drop.x,drop.y + grid.contentY);
                                           var destIndex = grid.count;
                                           if (realIndex !== -1) {
                                               var destItem = grid.itemAtIndex(realIndex);
                                               var destLocalX = grid.mapToItem(destItem,
                                                                               drop.x,
                                                                               drop.y + grid.contentY).x;
                                               destIndex = destLocalX < (grid.cellWidth / 2) ? realIndex : realIndex + 1;
                                           }
                                           dropManager.droppedComicsForResortingAt("", destIndex);
                                       }
                                   }
                               }
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

            Loader {
                id: infoView
                width: info_container.width
                sourceComponent: currentIndexHelper.focusedFolderRow >= 0
                                 ? folderInfoComponent
                                 : currentIndexHelper.selectedComicCount > 1
                                   ? selectedComicsInfoComponent
                                 : currentIndexHelper.hasComicSelection
                                   ? comicInfoComponent
                                   : currentIndexHelper.currentLocationInfo.kind === "folder"
                                     ? folderInfoComponent
                                     : currentIndexHelper.currentLocationInfo.kind === "library"
                                       ? libraryInfoComponent
                                     : currentIndexHelper.currentLocationInfo.name
                                       ? listInfoComponent
                                       : emptyInfoComponent
            }

            Component {
                id: comicInfoComponent
                ComicInfoView { width: infoView.width }
            }

            Component {
                id: selectedComicsInfoComponent
                SelectedComicsInfoView {
                    width: infoView.width
                    selectionInfo: currentIndexHelper.selectedComicsInfo
                }
            }

            Component {
                id: folderInfoComponent
                FolderInfoView {
                    width: infoView.width
                    folderInfo: currentIndexHelper.focusedFolderRow >= 0
                                ? currentIndexHelper.focusedFolderInfo
                                : currentIndexHelper.currentLocationInfo
                }
            }

            Component {
                id: libraryInfoComponent
                LibraryInfoView {
                    width: infoView.width
                    libraryInfo: currentIndexHelper.currentLocationInfo
                }
            }

            Component {
                id: listInfoComponent
                ListInfoView {
                    width: infoView.width
                    listInfo: currentIndexHelper.currentLocationInfo
                }
            }

            Component {
                id: emptyInfoComponent
                EmptyInfoView { width: infoView.width }
            }

            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.NoButton

                onWheel: (wheel) => {
                    if (infoFlickable.contentHeight <= infoFlickable.height)
                        return

                        var newValue = Math.min(
                            (infoFlickable.contentHeight - infoFlickable.height),
                                                Math.max(infoFlickable.originY, infoFlickable.contentY - wheel.angleDelta.y)
                        )
                        infoFlickable.contentY = newValue
                }
            }

            ScrollBar.vertical: ScrollBar {
                visible: infoFlickable.contentHeight > infoFlickable.height

                contentItem: Item {
                    implicitWidth: 12
                    implicitHeight: 26
                    Rectangle {
                        color: infoScrollbarColor
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
