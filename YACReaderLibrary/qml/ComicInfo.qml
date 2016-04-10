import QtQuick 2.6

import QtQuick.Controls 1.4
import QtQuick.Layouts 1.2

import QtGraphicalEffects 1.0

import com.yacreader.ComicInfo 1.0

Rectangle {

    color : "transparent"
    id: mainContainer

    height: info.height + 2 * topMargin

    property string infoColor: "#b0b0b0"
    property font infoFont: Qt.font({

                                        family: "Arial",
                                        pointSize: 10.5
                                    });

    property int topMargin : 27

    property bool compact : width <= 650

    property ComicInfo comicInfo

    RowLayout
    {
        id:main_layout
        anchors.fill: parent

        //READ------------------------------------------------------------
        ColumnLayout
        {
            Layout.topMargin: topMargin
            Layout.maximumWidth: 61
            Layout.fillHeight: true
            id: readStatus

            Layout.alignment: Qt.AlignTop |
                              Qt.AlignHCenter

            Rectangle {
                color: "transparent"
                width:  61
                height: 24

                InfoTick {
                    x: 27
                    y: 5
                }
            }

            visible: !mainContainer.compact
        }

        //INFO------------------------------------------------------------
        ColumnLayout
        {
            id: info
            //width: parent.width
            //Layout.fillWidth: true

            Layout.alignment: Qt.AlignTop |
                              Qt.AlignLeft

            Layout.maximumWidth: mainContainer.compact ? mainContainer.width : 960

            Layout.leftMargin: mainContainer.compact ? 30 : 0

            RowLayout
            {
                Layout.topMargin: topMargin

                InfoTick {
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft
                }

                Item {
                    Layout.fillWidth: true
                }

                InfoFavorites {
                    Layout.topMargin: 1
                    Layout.rightMargin: 17
                    Layout.alignment: Qt.AlignTop
                }

                InfoRating {
                    Layout.alignment: Qt.AlignTop
                    Layout.rightMargin: 30
                }

                visible: mainContainer.compact
            }

            RowLayout
            {
                Text {
                    Layout.topMargin: mainContainer.compact ? 18 : topMargin
                    Layout.fillWidth: true
                    Layout.rightMargin: mainContainer.compact ? 30 : 0

                    id: title

                    color: "#ffffff"
                    font.family: "Arial"
                    font.bold: true
                    font.pointSize: mainContainer.compact ? 14 : 15.75;
                    wrapMode: Text.WordWrap

                    text: "#1" + "  -  " + "A Single Dream Can Spark A Revolution"
                }

                RowLayout
                {
                    visible: !mainContainer.compact

                    Layout.alignment: Qt.AlignTop
                    Layout.topMargin: topMargin

                    InfoFavorites {
                        Layout.topMargin: 1
                        Layout.rightMargin: 17
                        Layout.alignment: Qt.AlignTop
                    }

                    InfoRating {
                        Layout.alignment: Qt.AlignTop
                        Layout.rightMargin: 30
                    }
                }
            }

            Flow {
                spacing: 0

                Layout.fillWidth: true
                Text {
                    id: volume
                    color: infoColor
                    font: mainContainer.infoFont
                    text: "Clive Barker's Next Testament"
                    rightPadding: 20
                }

                Text {
                    id: numbering
                    color: infoColor
                    font: mainContainer.infoFont
                    text: "1/12"
                    rightPadding: 20
                }

                Text {
                    id: genre
                    color: infoColor
                    font: mainContainer.infoFont
                    text: "Terror"
                    rightPadding: 20
                }

                Text {
                    id: date
                    color: infoColor
                    font: mainContainer.infoFont
                    text: "21/03/2016"
                    rightPadding: 20
                }

                Text {
                    id: pages
                    color: infoColor
                    font: mainContainer.infoFont
                    text: "23 pages"
                    rightPadding: 20
                }

                Text {
                    id: showInComicVinw
                    color: "#ffcc00"
                    font: mainContainer.infoFont
                    text: "Show in Comic Vine"

                    onLinkActivated: Qt.openUrlExternally(link)
                }
            }

            Text {
                Layout.topMargin: 31
                Layout.rightMargin: 30
                Layout.fillWidth: true

                id: sinopsis
                color: "white"
                font.family: "Arial"
                font.pointSize: 11.75
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignJustify
                text: "The first original comic series created and written by Clive Barker is here!Julian Edmond, captain of industry, has left behind everything to begin a walkabout -- he believes he’s on a mission from God. While in the wasteland, he comes across a figure unlike any other, who calls himself Wick...and claims to be God. Their journey will span the globe, as neither man merely wants to make a mark on the world, but a scar.Clive Barker, with internationally acclaimed artist Haemi Jang (HELLRAISER: THE ROAD BELOW), come together to create the next legendary work in the canon of one of the great writers of our era. Ask your retailer about the ultra-limited edition signed Clive Barker variant cover, painted by the master of horror himself!"
            }

            Text {
                Layout.topMargin: 20
                Layout.bottomMargin: 5

                id: authors_title
                color: "white"
                font.family: "Arial"
                font.pointSize: 13.5
                font.bold: true

                text: "Authors"
            }

            Flow {
                Layout.fillWidth: true
                spacing: 20
                Repeater {
                    id: authors
                    model: 50
                    Column{
                        Text {
                            color: "white"
                            font.family: "Arial"
                            font.pointSize: 11.25

                            text: "Author"
                        }

                        Text {
                            color: "#b0b0b0"
                            font.family: "Arial"
                            font.pointSize: 9.75
                            font.italic: true
                            text: "writer"
                        }
                    }
                }
            }

            Text {
                Layout.topMargin: 20

                id: publisher_title
                color: "white"
                font.family: "Arial"
                font.pointSize: 13.5
                font.bold: true

                text: "Publiser"
            }

            Text {
                Layout.topMargin: 20
                Layout.bottomMargin: 5

                id: characters_title
                color: "white"
                font.family: "Arial"
                font.pointSize: 13.5
                font.bold: true

                text: "Characters"
            }

            Flow {
                Layout.fillWidth: true
                spacing: 20
                Repeater {
                    id: characters
                    model: 50

                    Text {
                        color: "white"
                        font.family: "Arial"
                        font.pointSize: 11.25

                        text: "character"
                    }
                }
            }
        }

        Item {
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.minimumWidth: 0
            Layout.preferredWidth: 0
        }
    }
}
