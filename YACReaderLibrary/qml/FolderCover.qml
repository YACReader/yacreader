import QtQuick
import QtQuick.Effects

Item {
    id: root

    required property url coverSource
    // Covers of the stack, from front to back. The first entry is the front
    // cover, so only the two next ones are stacked behind it.
    property var stackedCoverSources: []
    property bool selected: false
    property bool showRecentIndicator: false
    property bool showFinishedMark: false
    property real cornerRadius: 10

    readonly property int stackedCount: stackedCoverSources.length
    readonly property url midCoverSource: stackedCount > 1 ? stackedCoverSources[1] : ""
    readonly property url backCoverSource: stackedCount > 2 ? stackedCoverSources[2] : ""

    // A cover image with rounded corners and an outline. The mask is inset by
    // one pixel so that the alpha fades out inside the item, which keeps the
    // edges smooth when the cover is rotated.
    component RoundedCover: Item {
        id: cover

        required property url coverSource
        property real cornerRadius: 10
        property color outlineColor: "transparent"

        Image {
            id: coverImage
            anchors.fill: parent
            source: cover.coverSource
            fillMode: Image.PreserveAspectCrop
            smooth: true
            mipmap: true
            asynchronous: true
            cache: true
            visible: false
        }

        Item {
            id: coverMask
            anchors.fill: parent
            layer.enabled: true
            layer.smooth: true
            visible: false

            Rectangle {
                anchors.fill: parent
                anchors.margins: 1
                radius: cover.cornerRadius
                color: "black"
            }
        }

        MultiEffect {
            anchors.fill: parent
            source: coverImage
            maskEnabled: true
            maskSource: coverMask
            maskThresholdMin: 0.5
            maskSpreadAtMin: 1.0
        }

        Rectangle {
            anchors.fill: parent
            radius: cover.cornerRadius
            color: "transparent"
            border.color: cover.outlineColor
            border.width: 1
        }
    }

    Rectangle {
        anchors.fill: parent
        rotation: -4
        radius: root.cornerRadius
        color: placeholderFolder1Color
        border.color: placeholderFolder1BorderColor
        border.width: 1
        visible: root.backCoverSource.toString().length === 0
    }

    RoundedCover {
        anchors.fill: parent
        rotation: -4
        opacity: 0.5
        coverSource: root.backCoverSource
        cornerRadius: root.cornerRadius
        outlineColor: placeholderFolder1BorderColor
        visible: root.backCoverSource.toString().length > 0
    }

    Rectangle {
        anchors.fill: parent
        rotation: 3
        radius: root.cornerRadius
        color: placeholderFolder2Color
        border.color: placeholderFolder2BorderColor
        border.width: 1
        visible: root.midCoverSource.toString().length === 0
    }

    RoundedCover {
        anchors.fill: parent
        rotation: 3
        opacity: 0.75
        coverSource: root.midCoverSource
        cornerRadius: root.cornerRadius
        outlineColor: placeholderFolder2BorderColor
        visible: root.midCoverSource.toString().length > 0
    }

    RoundedCover {
        anchors.fill: parent
        coverSource: root.coverSource
        cornerRadius: root.cornerRadius
        outlineColor: folderCoverBorderColor
    }

    Rectangle {
        width: 10
        height: 10
        radius: 5
        anchors { left: parent.left; top: parent.top; topMargin: 10; leftMargin: 10 }
        color: newItemColor
        visible: root.showRecentIndicator
    }

    Image {
        z: 2
        width: 23
        height: 23
        source: "tick.svg"
        visible: root.showFinishedMark
        anchors { right: parent.right; top: parent.top; topMargin: 9; rightMargin: 9 }
        asynchronous: true
    }

    Rectangle {
        z: 2
        anchors.fill: parent
        anchors.margins: -3
        radius: root.cornerRadius + 3
        color: "transparent"
        border.color: cellSelectedBorderColor
        border.width: 3
        opacity: root.selected ? 1 : 0

        Behavior on opacity { NumberAnimation { duration: 150 } }
    }
}
