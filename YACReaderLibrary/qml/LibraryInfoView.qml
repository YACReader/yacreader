import QtQuick
import QtQuick.Layouts

Rectangle {
    id: root

    required property var libraryInfo

    property int panelMargin: 30
    property color secondaryTextColor: infoMetadataTextColor

    color: "transparent"
    height: content.implicitHeight + panelMargin * 2

    component MetadataText: Text {
        font.family: fontFamily
        font.pointSize: fontSize + 1
    }

    ColumnLayout {
        id: content
        x: root.panelMargin
        y: root.panelMargin
        width: root.width - root.panelMargin * 2
        spacing: 12

        Text {
            Layout.fillWidth: true
            text: root.libraryInfo.name ?? ""
            color: infoTextColor
            font.family: "Arial"
            font.bold: true
            font.pixelSize: 21
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
        }

        Text {
            Layout.fillWidth: true
            text: qsTr("Library info")
            color: root.secondaryTextColor
            font.family: fontFamily
            font.pointSize: fontSize + 1
            font.bold: true
            horizontalAlignment: Text.AlignHCenter
        }

        Text {
            Layout.fillWidth: true
            Layout.topMargin: 12
            text: root.libraryInfo.path ?? ""
            color: themeLinkColor
            font.family: fontFamily
            font.pointSize: fontSize + 1
            font.underline: pathMouseArea.containsMouse
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere

            MouseArea {
                id: pathMouseArea
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: currentIndexHelper.requestOpenLibraryFolder()
            }
        }

        GridLayout {
            Layout.fillWidth: true
            Layout.topMargin: 6
            columns: 2
            columnSpacing: 18
            rowSpacing: 12

            MetadataText { text: qsTr("Number of folders"); color: root.secondaryTextColor }
            MetadataText { text: root.libraryInfo.folderCount ?? 0; color: infoTextColor; Layout.fillWidth: true }

            MetadataText { text: qsTr("Number of comics"); color: root.secondaryTextColor }
            MetadataText { text: root.libraryInfo.comicCount ?? 0; color: infoTextColor; Layout.fillWidth: true }

            MetadataText { text: qsTr("Number of read comics"); color: root.secondaryTextColor }
            MetadataText { text: root.libraryInfo.readComicCount ?? 0; color: infoTextColor; Layout.fillWidth: true }
        }
    }
}
