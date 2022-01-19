import QtQuick 2.15

import QtQuick.Controls 2.15
import QtQuick.Layouts 1.12

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

                    read: comicInfo ? comicInfo.read ?? false : false

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

                    read: comicInfo ? comicInfo.read ?? false : false

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

                    active: comicInfo ? comicInfo.isFavorite ?? false : false

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
                    rating: comicInfo ? comicInfo.rating ?? 0 : 0

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

                    text: comic ? comic.getTitleIncludingNumber() ?? "" : ""
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

                        active: comicInfo ? comicInfo.isFavorite ?? false : false

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
                        rating: comicInfo ? comicInfo.rating ?? 0 : 0

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
                    text: comicInfo ? comicInfo.volume ?? "" : ""
                    rightPadding: 20
                    visible: comicInfo ? comicInfo.volume ?? false : false
                }

                Text {
                    id: numbering
                    color: infoColor
                    font: mainContainer.infoFont
                    text: (comicInfo ? comicInfo.number ?? "" : "") + "/" + (comicInfo ? comicInfo.count ?? "" : "")
                    rightPadding: 20
                    visible : comicInfo ? comicInfo.number ?? false : false
                }

                Text {
                    id: genre
                    color: infoColor
                    font: mainContainer.infoFont
                    text: comicInfo ? comicInfo.genere ?? "" : ""
                    rightPadding: 20
                    visible: comicInfo ? comicInfo.genere ?? false : false
                }

                Text {
                    id: date
                    color: infoColor
                    font: mainContainer.infoFont
                    text: comicInfo ? comicInfo.date ?? "" : ""
                    rightPadding: 20
                    visible: comicInfo ? comicInfo.date ?? false : false
                }

                Text {
                    id: pages
                    color: infoColor
                    font: mainContainer.infoFont
                    text: comicInfo ? comicInfo.numPages ?? "" : "" + " pages"
                    rightPadding: 20
                    visible: comicInfo ? comicInfo.numPages ?? false : false
                }

                Text {
                    id: showInComicVine
                    font: mainContainer.infoFont
                    color: "#ffcc00"
                    text: "Show in Comic Vine"
                    visible: comicInfo ? comicInfo.comicVineID ?? false : false
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            Qt.openUrlExternally("http://www.comicvine.com/comic/4000-%1/".arg(comicInfo ? comicInfo.comicVineID ?? "" : ""));
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
                text: '<html><head><style>
                        a {
                            color: #FFCB00;
                            text-decoration:none;
                        }
                    </style></head><body>' + (comicInfo ? comicInfo.synopsis ?? "" : "") + '</body></html>'
                visible: comicInfo ? comicInfo.synopsis ?? false : false
                textFormat: Text.RichText
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

                visible: comicInfo ? (comicInfo.getWriters().length +
                                      comicInfo.getPencillers().length +
                                      comicInfo.getInkers().length +
                                      comicInfo.getColorists().length +
                                      comicInfo.getLetterers().length +
                                      comicInfo.getCoverArtists().length > 0) : false
            }

            Flow {
                Layout.fillWidth: true
                spacing: 20
                Repeater {
                    id: writers
                    model: comicInfo ? comicInfo.getWriters().length : null
                    Column{
                        Text {
                            color: infoTitleColor
                            font.family: "Arial"
                            font.pixelSize: 15

                            text: comicInfo ? comicInfo.getWriters()[index] : ""
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
                    model: comicInfo ? comicInfo.getPencillers().length : null
                    Column{
                        Text {
                            color: infoTitleColor
                            font.family: "Arial"
                            font.pixelSize: 15

                            text: comicInfo ? comicInfo.getPencillers()[index] : ""
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
                    model: comicInfo ? comicInfo.getInkers().length : null
                    Column{
                        Text {
                            color: infoTitleColor
                            font.family: "Arial"
                            font.pixelSize: 15

                            text: comicInfo ? comicInfo.getInkers()[index] : ""
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
                    model: comicInfo ? comicInfo.getColorists().length : null
                    Column{
                        Text {
                            color: infoTitleColor
                            font.family: "Arial"
                            font.pixelSize: 15

                            text: comicInfo ? comicInfo.getColorists()[index] : ""
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
                    model: comicInfo ? comicInfo.getLetterers().length : null
                    Column{
                        Text {
                            color: infoTitleColor
                            font.family: "Arial"
                            font.pixelSize: 15

                            text: comicInfo ? comicInfo.getLetterers()[index] : ""
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
                    model: comicInfo ? comicInfo.getCoverArtists().length : ""
                    Column{
                        Text {
                            color: infoTitleColor
                            font.family: "Arial"
                            font.pixelSize: 15

                            text: comicInfo ? comicInfo.getCoverArtists()[index] : ""
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

                    text: comicInfo ? comicInfo.publisher ?? "" : ""

                    visible: comicInfo ? comicInfo.publisher ?? false : false
                }

                Text {
                    id: format

                    color: infoTitleColor
                    font.family: "Arial"
                    font.pixelSize: 15

                    text: comicInfo ? comicInfo.format ?? "" : ""

                    visible: comicInfo ? comicInfo.format ?? false : false
                }

                Text {
                    id: color

                    color: infoTitleColor
                    font.family: "Arial"
                    font.pixelSize: 15

                    text: (comicInfo ? comicInfo.color : false) ? qsTr("color") : qsTr("b/w")

                    visible: comicInfo ? comicInfo.color ?? false : false
                }

                Text {
                    id: age_rating

                    color: infoTitleColor
                    font.family: "Arial"
                    font.pixelSize: 15

                    text: comicInfo ? comicInfo.ageRating ?? "" : ""

                    visible: comicInfo ? comicInfo.ageRating ?? false : false
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

                visible: comicInfo ? comicInfo.getCharacters().length > 0 : false
            }

            Flow {
                Layout.fillWidth: true
                spacing: 20
                Repeater {
                    id: characters
                    model: comicInfo ? comicInfo.getCharacters().length : null

                    Text {
                        color: infoTitleColor
                        font.family: "Arial"
                        font.pixelSize: 15

                        text: comicInfo ? comicInfo.getCharacters()[index] : ""
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
