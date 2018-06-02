import QtQuick 2.6

import QtQuick.Controls 1.4
import QtQuick.Layouts 1.2

import QtGraphicalEffects 1.0

import com.yacreader.ComicInfo 1.0
import com.yacreader.ComicDB 1.0

Rectangle {

    color : "transparent"
    id: mainContainer

    height: info.height + 2 * topMargin

    property string infoColor: infoTextColor
    property font infoFont: Qt.font({

                                        family: "Arial",
                                        pixelSize: 14
                                    });

    property int topMargin : 27

    property bool compact : width <= 650

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

                    read: comicInfo.read

                    onReadChangedByUser: {
                        comicInfo.read = read;
                        comicInfoHelper.setRead(comic_info_index, read);
                    }
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

                    read: comicInfo.read

                    onReadChangedByUser: {
                        comicInfo.read = read;
                        comicInfoHelper.setRead(comic_info_index, read);
                    }
                }

                Item {
                    Layout.fillWidth: true
                }

                InfoFavorites {
                    Layout.topMargin: 1
                    Layout.rightMargin: 17
                    Layout.alignment: Qt.AlignTop

                    active: comicInfo.isFavorite

                    onActiveChangedByUser: {
                        if(active)
                            comicInfoHelper.addToFavorites(comic_info_index);
                        else
                            comicInfoHelper.removeFromFavorites(comic_info_index);

                        comicInfo.isFavorite = active;
                    }
                }

                InfoRating {
                    Layout.alignment: Qt.AlignTop
                    Layout.rightMargin: 30
                    rating: comicInfo.rating

                    onRatingChangedByUser: {
                        comicInfo.rating = rating;
                        comicInfoHelper.rate(comic_info_index, rating);
                    }
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

                    color: infoTitleColor
                    font.family: "Arial"
                    font.bold: true
                    font.pixelSize: mainContainer.compact ? 18 : 21;
                    wrapMode: Text.WordWrap

                    text: comic.getTitleIncludingNumber()
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

                        active: comicInfo.isFavorite

                        onActiveChangedByUser: {
                            if(active)
                                comicInfoHelper.addToFavorites(comic_info_index);
                            else
                                comicInfoHelper.removeFromFavorites(comic_info_index);

                            comicInfo.isFavorite = active;
                        }
                    }

                    InfoRating {
                        Layout.alignment: Qt.AlignTop
                        Layout.rightMargin: 30
                        rating: comicInfo.rating

                        onRatingChangedByUser: {
                            comicInfo.rating = rating;
                            comicInfoHelper.rate(comic_info_index, rating);
                        }
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
                    text: comicInfo.volume
                    rightPadding: 20
                    visible: comicInfo.volume
                }

                Text {
                    id: numbering
                    color: infoColor
                    font: mainContainer.infoFont
                    text: comicInfo.number + "/" + comicInfo.count
                    rightPadding: 20
                    visible : comicInfo.number
                }

                Text {
                    id: genre
                    color: infoColor
                    font: mainContainer.infoFont
                    text: comicInfo.genere
                    rightPadding: 20
                    visible: comicInfo.genere
                }

                Text {
                    id: date
                    color: infoColor
                    font: mainContainer.infoFont
                    text: comicInfo.date
                    rightPadding: 20
                    visible: comicInfo.date
                }

                Text {
                    id: pages
                    color: infoColor
                    font: mainContainer.infoFont
                    text: comicInfo.numPages + " pages"
                    rightPadding: 20
                    visible: comicInfo.numPages
                }

                Text {
                    id: showInComicVine
                    font: mainContainer.infoFont
                    color: "#ffcc00"
                    text: "Show in Comic Vine"
                    visible: comicInfo.comicVineID
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            Qt.openUrlExternally("http://www.comicvine.com/comic/4000-%1/".arg(comicInfo.comicVineID));
                        }
                    }
                }
            }

            Text {
                Layout.topMargin: 22
                Layout.rightMargin: 30
                Layout.bottomMargin: 5
                Layout.fillWidth: true

                id: sinopsis
                color: infoTitleColor
                font.family: "Arial"
                font.pixelSize: 15
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignJustify
                text: comicInfo.synopsis
                visible: comicInfo.synopsis
            }

            Text {
                Layout.topMargin: 25
                Layout.bottomMargin: 5

                id: authors_title
                color: infoTitleColor
                font.family: "Arial"
                font.pixelSize: 18
                font.bold: true

                text: qsTr("Authors")

                visible: comicInfo.getWriters().length +
                         comicInfo.getPencillers().length +
                         comicInfo.getInkers().length +
                         comicInfo.getColorists().length +
                         comicInfo.getLetterers().length +
                         comicInfo.getCoverArtists().length > 0
            }

            Flow {
                Layout.fillWidth: true
                spacing: 20
                Repeater {
                    id: writers
                    model: comicInfo.getWriters().length
                    Column{
                        Text {
                            color: infoTitleColor
                            font.family: "Arial"
                            font.pixelSize: 15

                            text: comicInfo.getWriters()[index]
                        }

                        Text {
                            color: infoTextColor
                            font.family: "Arial"
                            font.pixelSize: 13
                            font.italic: true
                            text: qsTr("writer")
                        }
                    }
                }

                Repeater {
                    id: pencilllers
                    model: comicInfo.getPencillers().length
                    Column{
                        Text {
                            color: infoTitleColor
                            font.family: "Arial"
                            font.pixelSize: 15

                            text: comicInfo.getPencillers()[index]
                        }

                        Text {
                            color: infoTextColor
                            font.family: "Arial"
                            font.pixelSize: 13
                            font.italic: true
                            text: qsTr("penciller")
                        }
                    }
                }

                Repeater {
                    id: inkers
                    model: comicInfo.getInkers().length
                    Column{
                        Text {
                            color: infoTitleColor
                            font.family: "Arial"
                            font.pixelSize: 15

                            text: comicInfo.getInkers()[index]
                        }

                        Text {
                            color: infoTextColor
                            font.family: "Arial"
                            font.pixelSize: 13
                            font.italic: true
                            text: qsTr("inker")
                        }
                    }
                }

                Repeater {
                    id: colorist
                    model: comicInfo.getColorists().length
                    Column{
                        Text {
                            color: infoTitleColor
                            font.family: "Arial"
                            font.pixelSize: 15

                            text: comicInfo.getColorists()[index]
                        }

                        Text {
                            color: infoTextColor
                            font.family: "Arial"
                            font.pixelSize: 13
                            font.italic: true
                            text: qsTr("colorist")
                        }
                    }
                }

                Repeater {
                    id: letterers
                    model: comicInfo.getLetterers().length
                    Column{
                        Text {
                            color: infoTitleColor
                            font.family: "Arial"
                            font.pixelSize: 15

                            text: comicInfo.getLetterers()[index]
                        }

                        Text {
                            color: infoTextColor
                            font.family: "Arial"
                            font.pixelSize: 13
                            font.italic: true
                            text: qsTr("letterer")
                        }
                    }
                }

                Repeater {
                    id: cover_artist
                    model: comicInfo.getCoverArtists().length
                    Column{
                        Text {
                            color: infoTitleColor
                            font.family: "Arial"
                            font.pixelSize: 15

                            text: comicInfo.getCoverArtists()[index]
                        }

                        Text {
                            color: infoTextColor
                            font.family: "Arial"
                            font.pixelSize: 13
                            font.italic: true
                            text: qsTr("cover artist")
                        }
                    }
                }
            }

            Text {
                Layout.topMargin: 25

                id: publisher_title
                color: infoTitleColor
                font.family: "Arial"
                font.pixelSize: 18
                font.bold: true

                text: qsTr("Publisher")

                visible: publisher.visible || format.visible || color.visible || age_rating.visible
            }

            Flow {
                Layout.fillWidth: true
                spacing: 20

                Text {
                    id: publisher

                    color: infoTitleColor
                    font.family: "Arial"
                    font.pixelSize: 15

                    text: comicInfo.publisher

                    visible: comicInfo.publisher
                }

                Text {
                    id: format

                    color: infoTitleColor
                    font.family: "Arial"
                    font.pixelSize: 15

                    text: comicInfo.format

                    visible: comicInfo.format
                }

                Text {
                    id: color

                    color: infoTitleColor
                    font.family: "Arial"
                    font.pixelSize: 15

                    text: comicInfo.color ? qsTr("color") : qsTr("b/w")

                    visible: comicInfo.color
                }

                Text {
                    id: age_rating

                    color: infoTitleColor
                    font.family: "Arial"
                    font.pixelSize: 15

                    text: comicInfo.ageRating

                    visible: comicInfo.ageRating
                }
            }

            Text {
                Layout.topMargin: 25
                Layout.bottomMargin: 5

                id: characters_title
                color: infoTitleColor
                font.family: "Arial"
                font.pixelSize: 18
                font.bold: true

                text: qsTr("Characters")

                visible: comicInfo.getCharacters().length > 0
            }

            Flow {
                Layout.fillWidth: true
                spacing: 20
                Repeater {
                    id: characters
                    model: comicInfo.getCharacters().length

                    Text {
                        color: infoTitleColor
                        font.family: "Arial"
                        font.pixelSize: 15

                        text: comicInfo.getCharacters()[index]
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
