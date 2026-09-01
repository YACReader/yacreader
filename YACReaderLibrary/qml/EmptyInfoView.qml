import QtQuick
import QtQuick.Layouts

Rectangle {
    id: root

    color: "transparent"
    height: 240

    ColumnLayout {
        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
            margins: 30
        }
        spacing: 8

        Text {
            Layout.fillWidth: true
            text: qsTr("Nothing selected")
            color: infoTextColor
            font.family: "Arial"
            font.bold: true
            font.pixelSize: 21
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
        }

        Text {
            Layout.fillWidth: true
            text: qsTr("Select a comic or folder to see its information.")
            color: infoMetadataTextColor
            font.family: "Arial"
            font.pixelSize: 14
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
        }
    }
}
