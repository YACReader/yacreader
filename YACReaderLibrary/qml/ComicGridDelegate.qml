pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Basic
import QtQuick.Controls.impl

// Delegate for GridContentModel comic rows; required properties intentionally match its role names.
Rectangle {
    id: cell

    required property int index
    required property int source_index
    required property var number
    required property string title
    required property int num_pages
    required property bool read_column
    required property int current_page
    required property int rating
    required property bool has_been_opened
    required property url cover_path
    required property double added_date
    required property bool show_recent
    required property double recent_range

    required property int currentViewIndex
    required property var selectionHelper

    readonly property int selectionRevision: selectionHelper.selectionRevision
    readonly property bool selected: selectionRevision >= 0 && selectionHelper.isSelectedIndex(source_index)

    property alias interactionItem: realCell

    signal activateRequested(int viewRow)
    signal clearFolderFocusRequested()
    signal contextMenuRequested(point localPosition)
    signal focusViewRowRequested(int viewRow)
    signal rateRequested(int sourceRow, int rating)
    signal selectRangeRequested(int from, int to)
    signal setCurrentViewRowRequested(int viewRow)
    signal setCurrentComicRowRequested(int sourceRow)
    signal startDragRequested()

    color: "transparent"
    scale: mouseArea.containsMouse ? 1.025 : 1

    Behavior on scale { NumberAnimation { duration: 90 } }

    BorderImage {
        anchors {
            top: realCell.top
            left: realCell.left
            right: realCell.right
            bottom: realCell.bottom
            margins: -10
        }
        border { left: 10; top: 10; right: 10; bottom: 10 }
        horizontalTileMode: BorderImage.Stretch
        verticalTileMode: BorderImage.Stretch
        source: "prerendered_cover_shadow.png"
        visible: showDropShadow
    }

    Rectangle {
        id: realCell

        property bool dragging: false

        Drag.active: mouseArea.drag.active
        Drag.hotSpot.x: 32
        Drag.hotSpot.y: 32
        Drag.dragType: Drag.Automatic
        Drag.proposedAction: Qt.CopyAction
        Drag.onActiveChanged: {
            if (!dragging) {
                cell.startDragRequested()
                dragging = true
            } else {
                dragging = false
            }
        }

        width: itemWidth
        height: itemHeight
        color: cell.selected ? cellSelectedColor : cellColor
        anchors.horizontalCenter: parent.horizontalCenter

        Rectangle {
            z: -1
            color: "transparent"
            anchors {
                fill: parent
                margins: -2
            }
            border.color: cellSelectedBorderColor
            border.width: 3
            opacity: cell.selected ? 1 : 0
            radius: 2

            Behavior on opacity { NumberAnimation { duration: 300 } }
        }

        MouseArea {
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
                cell.selectionHelper.selectOnly(cell.source_index)
                cell.setCurrentViewRowRequested(cell.index)
                cell.activateRequested(cell.index)
            }

            onPressed: mouse => {
                const currentIndex = cell.currentViewIndex
                cell.clearFolderFocusRequested()

                if (mouse.modifiers & Qt.ShiftModifier) {
                    if (cell.index < currentIndex) {
                        cell.selectRangeRequested(cell.index, currentIndex)
                        cell.setCurrentViewRowRequested(cell.index)
                    } else if (cell.index > currentIndex) {
                        cell.selectRangeRequested(currentIndex, cell.index)
                        cell.setCurrentViewRowRequested(cell.index)
                    }
                }

                mouse.accepted = true

                if (mouse.button === Qt.RightButton) {
                    if (!cell.selectionHelper.isSelectedIndex(cell.source_index))
                        cell.focusViewRowRequested(cell.index)

                    cell.contextMenuRequested(Qt.point(mouseX, mouseY))
                    mouse.accepted = false
                } else {
                    if (mouse.modifiers & Qt.ControlModifier) {
                        if (cell.selectionHelper.isSelectedIndex(cell.source_index)) {
                            if (cell.selectionHelper.numItemsSelected() > 1) {
                                cell.selectionHelper.deselectIndex(cell.source_index)
                                if (cell.currentViewIndex === cell.index)
                                    cell.setCurrentComicRowRequested(cell.selectionHelper.lastSelectedIndex())
                            }
                        } else {
                            cell.selectionHelper.selectIndex(cell.source_index)
                            cell.setCurrentViewRowRequested(cell.index)
                        }
                    }

                    if (!(mouse.modifiers & Qt.ControlModifier || mouse.modifiers & Qt.ShiftModifier)) {
                        if (!cell.selectionHelper.isSelectedIndex(cell.source_index))
                            cell.focusViewRowRequested(cell.index)

                        cell.setCurrentViewRowRequested(cell.index)
                    }
                }
            }

            onReleased: mouse => {
                if (mouse.button === Qt.LeftButton
                        && !(mouse.modifiers & Qt.ControlModifier || mouse.modifiers & Qt.ShiftModifier)
                        && cell.selectionHelper.isSelectedIndex(cell.source_index)) {
                    cell.focusViewRowRequested(cell.index)
                }
            }
        }
    }

    Image {
        id: coverElement
        width: coverWidth
        height: coverHeight
        anchors { horizontalCenter: parent.horizontalCenter; top: realCell.top }
        source: cell.cover_path
        fillMode: Image.PreserveAspectCrop
        smooth: true
        mipmap: true
        asynchronous: true
        cache: false
    }

    Rectangle {
        width: 10
        height: 10
        radius: 5
        anchors { left: coverElement.left; top: coverElement.top; topMargin: 5; leftMargin: 5 }
        color: newItemColor
        visible: (((new Date() / 1000) - cell.added_date) < cell.recent_range) && cell.show_recent
    }

    Rectangle {
        width: coverElement.width
        height: coverElement.height
        anchors { horizontalCenter: parent.horizontalCenter; top: realCell.top }
        color: "transparent"
        border { color: comicCoverBorderColor; width: 1 }
    }

    Image {
        width: 23
        height: 23
        source: cell.read_column && show_marks ? "tick.svg"
                                               : cell.has_been_opened && show_marks ? "reading.svg" : ""
        anchors { right: coverElement.right; top: coverElement.top; topMargin: 9; rightMargin: 9 }
        asynchronous: true
    }

    Text {
        anchors { top: coverElement.bottom; left: realCell.left; leftMargin: 4; rightMargin: 4; topMargin: 4 }
        width: itemWidth - 8
        maximumLineCount: 2
        wrapMode: Text.WordWrap
        text: cell.title
        elide: Text.ElideRight
        color: itemTitleColor
        clip: true
        font.letterSpacing: fontSpacing
        font.pointSize: fontSize
        font.family: fontFamily
    }

    Text {
        anchors { bottom: realCell.bottom; left: realCell.left; margins: 4 }
        text: cell.number ? "<b>#</b>" + cell.number : ""
        color: itemDetailsColor
        font.letterSpacing: fontSpacing
        font.pointSize: fontSize
        font.family: fontFamily
    }

    ColorImage {
        id: pageImage
        anchors { bottom: realCell.bottom; right: realCell.right; bottomMargin: 6; rightMargin: 4; leftMargin: 4 }
        source: "page.svg"
        color: itemDetailsColor
        width: 8
        height: 10
    }

    Text {
        id: pages
        anchors { bottom: realCell.bottom; right: pageImage.left; margins: 4 }
        text: cell.has_been_opened ? cell.current_page + "/" + cell.num_pages : cell.num_pages
        color: itemDetailsColor
        font.letterSpacing: fontSpacing
        font.pointSize: fontSize
        font.family: fontFamily
    }

    ColorImage {
        id: ratingImage
        anchors { bottom: realCell.bottom; right: pageImage.left; bottomMargin: 6.5; rightMargin: Math.floor(pages.width) + 12 }
        source: "star.svg"
        color: itemDetailsColor
        width: 11
        height: 11

        MouseArea {
            anchors.fill: parent
            onPressed: {
                cell.selectionHelper.selectOnly(cell.source_index)
                cell.setCurrentViewRowRequested(cell.index)
                ratingLoader.active = true
                ratingLoader.item.popup()
            }
        }

        Loader {
            id: ratingLoader
            active: false
            sourceComponent: ratingContextMenuComponent
        }

        Component {
            id: ratingContextMenuComponent
            Menu {
                id: ratingMenu

                readonly property real menuItemPadding: 6
                property TextMetrics resetRatingTextMetrics: TextMetrics {
                    font: ratingMenu.font
                    text: resetRatingAction.text
                }

                implicitWidth: Math.ceil(resetRatingTextMetrics.advanceWidth) + 2 * menuItemPadding + leftPadding + rightPadding

                Action { text: "1"; onTriggered: cell.rateRequested(cell.source_index, 1) }
                Action { text: "2"; onTriggered: cell.rateRequested(cell.source_index, 2) }
                Action { text: "3"; onTriggered: cell.rateRequested(cell.source_index, 3) }
                Action { text: "4"; onTriggered: cell.rateRequested(cell.source_index, 4) }
                Action { text: "5"; onTriggered: cell.rateRequested(cell.source_index, 5) }
                MenuSeparator {}
                Action { id: resetRatingAction; text: qsTranslate("LibraryWindowActions", "Reset rating"); onTriggered: cell.rateRequested(cell.source_index, 0) }

                delegate: MenuItem {
                    implicitHeight: 30
                    padding: ratingMenu.menuItemPadding
                }
            }
        }
    }

    Text {
        anchors { bottom: realCell.bottom; right: ratingImage.left; margins: 4 }
        text: cell.rating > 0 ? cell.rating : "-"
        color: itemDetailsColor
    }
}
