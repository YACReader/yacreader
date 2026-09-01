import QtQuick
import QtQuick.Layouts

Rectangle {
    id: root

    required property var folderInfo

    property int panelMargin: 30
    property color secondaryTextColor: infoMetadataTextColor

    component MetadataText: Text {
        font.family: fontFamily
        font.pointSize: fontSize + 1
    }

    color: "transparent"
    height: content.implicitHeight + panelMargin * 2

    function formattedDate(timestamp) {
        if (!timestamp)
            return qsTr("Unknown")
        return new Date(timestamp * 1000).toLocaleDateString(Qt.locale(), Locale.ShortFormat)
    }

    ColumnLayout {
        id: content
        x: root.panelMargin
        y: root.panelMargin
        width: root.width - root.panelMargin * 2
        spacing: 12

        FolderCover {
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: Math.min(220, content.width)
            Layout.preferredHeight: Layout.preferredWidth * coverHeight / coverWidth
            coverSource: root.folderInfo.cover ?? ""
        }

        Text {
            Layout.fillWidth: true
            Layout.topMargin: 6
            text: root.folderInfo.name ?? ""
            color: infoTextColor
            font.family: "Arial"
            font.bold: true
            font.pixelSize: 21
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
        }

        Text {
            Layout.fillWidth: true
            text: root.folderInfo.path ?? ""
            color: root.secondaryTextColor
            font.family: "Arial"
            font.pixelSize: 13
            wrapMode: Text.WrapAnywhere
            horizontalAlignment: Text.AlignHCenter
            visible: text.length > 0
        }

        GridLayout {
            Layout.fillWidth: true
            columns: 2
            columnSpacing: 18
            rowSpacing: 9

            MetadataText { text: qsTr("Items"); color: root.secondaryTextColor }
            MetadataText { text: root.folderInfo.itemCount ?? 0; color: infoTextColor; Layout.fillWidth: true }

            MetadataText { text: qsTr("Type"); color: root.secondaryTextColor }
            MetadataText { text: root.folderInfo.typeName ?? ""; color: infoTextColor; Layout.fillWidth: true }

            MetadataText { text: qsTr("Reading status"); color: root.secondaryTextColor }
            MetadataText { text: root.folderInfo.finished ? qsTr("Read") : qsTr("Unread"); color: infoTextColor; Layout.fillWidth: true }

            MetadataText { text: qsTr("Collection status"); color: root.secondaryTextColor }
            MetadataText { text: root.folderInfo.completed ? qsTr("Completed") : qsTr("In progress"); color: infoTextColor; Layout.fillWidth: true }

            MetadataText { text: qsTr("Added"); color: root.secondaryTextColor }
            MetadataText { text: root.formattedDate(root.folderInfo.added); color: infoTextColor; Layout.fillWidth: true }

            MetadataText { text: qsTr("Updated"); color: root.secondaryTextColor }
            MetadataText { text: root.formattedDate(root.folderInfo.updated); color: infoTextColor; Layout.fillWidth: true }
        }
    }
}
