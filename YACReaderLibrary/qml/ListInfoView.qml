import QtQuick
import QtQuick.Layouts

Rectangle {
    id: root

    required property var listInfo

    property int panelMargin: 30
    property color secondaryTextColor: infoMetadataTextColor

    color: "transparent"
    height: content.implicitHeight + panelMargin * 2

    ColumnLayout {
        id: content
        x: root.panelMargin
        y: root.panelMargin
        width: root.width - root.panelMargin * 2
        spacing: 12

        Image {
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: Math.min(110, content.width)
            Layout.preferredHeight: 95
            source: root.listInfo.icon ?? ""
            fillMode: Image.PreserveAspectFit
            visible: source.toString().length > 0
        }

        Text {
            Layout.fillWidth: true
            Layout.topMargin: 8
            text: root.listInfo.name ?? ""
            color: infoTextColor
            font.family: "Arial"
            font.bold: true
            font.pixelSize: 21
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
        }

        Text {
            Layout.fillWidth: true
            text: (root.listInfo.itemCount ?? 0) === 1
                  ? qsTr("1 comic")
                  : qsTr("%1 comics").arg(root.listInfo.itemCount ?? 0)
            color: root.secondaryTextColor
            font.family: fontFamily
            font.pointSize: fontSize + 1
            horizontalAlignment: Text.AlignHCenter
        }

        Text {
            Layout.fillWidth: true
            text: (root.listInfo.recentDays ?? 0) === 1
                  ? qsTr("Last day")
                  : qsTr("Last %1 days").arg(root.listInfo.recentDays ?? 0)
            color: root.secondaryTextColor
            font.family: fontFamily
            font.pointSize: fontSize + 1
            horizontalAlignment: Text.AlignHCenter
            visible: (root.listInfo.recentDays ?? 0) > 0
        }

        Text {
            Layout.fillWidth: true
            text: (root.listInfo.sublistCount ?? 0) === 1
                  ? qsTr("1 sublist")
                  : qsTr("%1 sublists").arg(root.listInfo.sublistCount ?? 0)
            color: root.secondaryTextColor
            font.family: fontFamily
            font.pointSize: fontSize + 1
            horizontalAlignment: Text.AlignHCenter
            visible: (root.listInfo.sublistCount ?? 0) > 0
        }
    }
}
