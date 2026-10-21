import QtQuick

// Delegate for GridContentModel list card rows (the folders and lists of a folder of lists); required properties
// intentionally match its role names.
Rectangle {
    id: cell

    required property int index
    required property string title
    required property url cover_path
    required property string card_kind
    required property url icon_path
    required property bool selected

    signal openRequested()
    signal focusRequested()

    color: "transparent"

    scale: mouseArea.containsMouse ? 1.025 : 1
    Behavior on scale { NumberAnimation { duration: 90 } }

    Rectangle {
        id: realCell
        width: itemWidth
        height: itemHeight
        color: "transparent"
        anchors.horizontalCenter: parent.horizontalCenter

        MouseArea {
            id: mouseArea
            anchors.fill: parent
            acceptedButtons: Qt.LeftButton
            hoverEnabled: true

            onDoubleClicked: cell.openRequested()
            onPressed: cell.focusRequested()
        }
    }

    Item {
        id: coverElement
        width: coverWidth
        height: coverHeight
        anchors { horizontalCenter: parent.horizontalCenter; top: realCell.top }

        // a folder: the folder shape of the comic folders, with no image
        FolderCover {
            anchors.fill: parent
            coverSource: ""
            selected: cell.selected
            visible: cell.card_kind === "folder"
        }

        // a reading list: the cover of its first comic
        FolderCover.RoundedCover {
            anchors.fill: parent
            coverSource: cell.cover_path
            cornerRadius: 10
            outlineColor: folderCoverBorderColor
            visible: cell.card_kind !== "folder" && cell.cover_path.toString().length > 0
        }

        // a list without a cover: the art of its type
        Rectangle {
            anchors.fill: parent
            radius: 10
            color: placeholderFolder2Color
            border.color: placeholderFolder2BorderColor
            border.width: 1
            visible: cell.card_kind !== "folder" && cell.cover_path.toString().length === 0

            Image {
                anchors.centerIn: parent
                width: parent.width * 0.6
                height: width * 187 / 248
                sourceSize: Qt.size(width, height)
                source: cell.icon_path
                fillMode: Image.PreserveAspectFit
                asynchronous: true
            }
        }

        Rectangle {
            anchors.fill: parent
            anchors.margins: -3
            radius: 13
            color: "transparent"
            border.color: cellSelectedBorderColor
            border.width: 3
            opacity: cell.selected && cell.card_kind !== "folder" ? 1 : 0
            visible: opacity > 0

            Behavior on opacity { NumberAnimation { duration: 150 } }
        }
    }

    Text {
        z: 4
        anchors { top: coverElement.bottom; left: realCell.left; leftMargin: 4; rightMargin: 4; topMargin: 10 }
        width: itemWidth - 8
        maximumLineCount: 2
        wrapMode: Text.WordWrap
        text: cell.title
        elide: Text.ElideRight
        color: itemTitleColor
        font.letterSpacing: fontSpacing
        font.pointSize: fontSize
        font.family: fontFamily
    }
}
