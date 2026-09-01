import QtQuick
import QtQuick.Layouts

Rectangle {
    id: root

    required property var selectionInfo

    property int panelMargin: 30
    property color secondaryTextColor: infoMetadataTextColor

    readonly property bool hasSeries: (selectionInfo.seriesCount ?? 0) > 0

    color: "transparent"
    height: content.implicitHeight + panelMargin * 2

    component MetadataText: Text {
        font.family: fontFamily
        font.pointSize: fontSize + 1
    }

    function pagesText() {
        const pages = root.selectionInfo.pageCount ?? 0
        const unknown = root.selectionInfo.unknownPageCount ?? 0
        if (pages === 0)
            return qsTr("Unknown")
        if (unknown > 0)
            return qsTr("%1 (%2 unknown)").arg(pages).arg(unknown)
        return pages
    }

    function seriesText() {
        const count = root.selectionInfo.seriesCount ?? 0
        if (count === 1)
            return root.selectionInfo.seriesName ?? ""
        return qsTr("%1 series").arg(count)
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
            readonly property var covers: root.selectionInfo.covers ?? []
            coverSource: covers.length > 0 ? covers[0] : ""
            stackedCoverSources: covers
        }

        Text {
            Layout.fillWidth: true
            Layout.topMargin: 6
            text: qsTr("%1 comics selected").arg(root.selectionInfo.count ?? 0)
            color: infoTextColor
            font.family: "Arial"
            font.bold: true
            font.pixelSize: 21
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
        }

        GridLayout {
            Layout.fillWidth: true
            Layout.topMargin: 6
            columns: 2
            columnSpacing: 18
            rowSpacing: 9

            MetadataText { text: qsTr("Series"); color: root.secondaryTextColor; visible: root.hasSeries }
            MetadataText { text: root.seriesText(); color: infoTextColor; Layout.fillWidth: true; visible: root.hasSeries }

            MetadataText { text: qsTr("Read"); color: root.secondaryTextColor }
            MetadataText { text: root.selectionInfo.readCount ?? 0; color: infoTextColor; Layout.fillWidth: true }

            MetadataText { text: qsTr("In progress"); color: root.secondaryTextColor }
            MetadataText { text: root.selectionInfo.inProgressCount ?? 0; color: infoTextColor; Layout.fillWidth: true }

            MetadataText { text: qsTr("Unread"); color: root.secondaryTextColor }
            MetadataText { text: root.selectionInfo.unreadCount ?? 0; color: infoTextColor; Layout.fillWidth: true }

            MetadataText { text: qsTr("Total pages"); color: root.secondaryTextColor }
            MetadataText { text: root.pagesText(); color: infoTextColor; Layout.fillWidth: true }

            MetadataText { text: qsTr("Total size"); color: root.secondaryTextColor }
            MetadataText { text: root.selectionInfo.size ?? ""; color: infoTextColor; Layout.fillWidth: true }
        }
    }
}
