import QtQuick 2.3
import QtQuick.Controls 1.0
import QtQuick.Controls 1.1
import QtGraphicalEffects 1.0
import comicModel 1.0

Rectangle {
    id: main
    color: backgroundColor
    width: parent.width
    height: parent.height
    anchors.margins: 0

    function selectAll(from,to)
    {
        for(var i = from+1;i<to;i++)
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
                MouseArea {
                     anchors.fill: parent
                     onClicked: {
                         comicsSelectionHelper.clear();
                         comicsSelectionHelper.selectIndex(grid.currentIndex);
                     }
                 }

                Rectangle {
                    id: realCell

                    width: 156; height: 287
                    color: ((dummyValue || !dummyValue) && comicsSelectionHelper.isSelectedIndex(index)) || grid.currentIndex === index?selectedColor:cellColor;

                    anchors.horizontalCenter: parent.horizontalCenter

                    MouseArea  {
                         anchors.fill: parent
                         acceptedButtons: Qt.LeftButton | Qt.RightButton
                         onDoubleClicked: {

                             comicsSelectionHelper.clear();

                             comicsSelectionHelper.selectIndex(index);
                             grid.currentIndex = index;
                             comicsSelectionHelper.selectedItem(index);
                         }

                         onClicked: {
                             //grid.currentIndex = index
                             //comicsSelection.setCurrentIndex(index,0x0002)
                             var ci = grid.currentIndex;
                             if(mouse.button == Qt.RightButton || !(mouse.modifiers & Qt.ControlModifier || mouse.modifiers & Qt.ShiftModifier))
                             {
                                 comicsSelectionHelper.clear();
                             }

                             if(mouse.button == Qt.RightButton)
                                myContextMenu.popup();

                             if(mouse.modifiers & Qt.ShiftModifier)
                                if(index < ci)
                                    selectAll(index,ci);
                                else if (index > ci)
                                    selectAll(ci,index);

                             mouse.accepted = true;

                             comicsSelectionHelper.selectIndex(index)
                             grid.currentIndex = index;
                         }
                     }

                    //Menu emits the 'main' signals
                    Menu {
                      id: myContextMenu
                      MenuItem { text: "Open comic"; enabled: true; iconSource:"qrc:///images/openInYACReader.png"; onTriggered: openComicAction.trigger() }
                      MenuSeparator{}
                      MenuItem { text: "Open containing folder..."; enabled: true; iconSource: "qrc:///images/open.png"; onTriggered: openContainingFolderComicAction.trigger() }
                      MenuItem { text: "Update current folder"; enabled: true; iconSource: "qrc:///images/updateLibraryIcon.png"; onTriggered: updateCurrentFolderAction.trigger() }
                      MenuSeparator{}
                      MenuItem { text: "Reset comic rating"; onTriggered: resetComicRatingAction.trigger() }
                      MenuSeparator{}
                      MenuItem { text: "Edit"; enabled: true; iconSource:"qrc:///images/editComic.png"; onTriggered: editSelectedComicsAction.trigger() }
                      MenuItem { text: "Download tags from Comic Vine"; enabled: true; iconSource:"qrc:///images/getInfo.png"; onTriggered: getInfoAction.trigger() }
                      MenuItem { text: "Asign current order to comics"; enabled: true; iconSource:"qrc:///images/asignNumber.png"; onTriggered: asignOrderAction.trigger() }
                      MenuSeparator{}
                      MenuItem { text: "Select all comics"; enabled: true; iconSource:"qrc:///images/selectAll.png"; onTriggered: selectAllComicsAction.trigger() }
                      MenuSeparator{}
                      MenuItem { text: "Set as read"; enabled: true; iconSource:"qrc:///images/setReadButton.png"; onTriggered: setAsReadAction.trigger() }
                      MenuItem { text: "Set as unread"; enabled: true; iconSource:"qrc:///images/setUnread.png"; onTriggered: setAsNonReadAction.trigger() }
                      MenuItem { text: "Show or hide read marks"; enabled: true; iconSource:"qrc:///images/showMarks.png"; onTriggered: showHideMarksAction.trigger() }
                      MenuSeparator{}
                      MenuItem { text: "Delete selected comics"; enabled: true; iconSource:"qrc:///images/trash.png"; onTriggered: deleteComicsAction.trigger() }
                      MenuSeparator{}
                      MenuItem { text: "Fullscreen mode on/off"; onTriggered: toggleFullScreenAction.trigger() }
                      //MenuItem { text: "Show details"; onTriggered: cell.state = 'Details';
                    }


                }

                DropShadow {
                    anchors.fill: source
                    horizontalOffset: 0
                    verticalOffset: 0
                    radius: 3
                    samples: 24
                    color: "#40000000"
                    transparentBorder: true;
                    source: realCell;
                    enabled: dropShadow;
                    visible: dropShadow;
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
                    //smooth: true
                    mipmap: true
                    //antialiasing: true
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
                    anchors { top: realCell.top; left: realCell.left; leftMargin: 4; rightMargin: 4; topMargin: 234; }
                    width: 148
                    maximumLineCount: 2
                    wrapMode: Text.WordWrap
                    text: title
                    elide: Text.ElideRight
                    color: titleColor
                    clip: true
                    font.letterSpacing: 0.5
                }
                //number
                Text {
                    anchors {bottom: realCell.bottom; left: realCell.left; margins: 4}
                    text: number?"<b>#</b>"+number:""
                    color: textColor
                    font.letterSpacing: 0.5
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
                    font.letterSpacing: 0.5
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
                return Math.floor(width / 190);
            }

            onWidthChanged: {
                var numCells = numCellsPerRow();
                var rest = width % 190;

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
            comicsSelectionHelper.selectIndex(ci);
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


