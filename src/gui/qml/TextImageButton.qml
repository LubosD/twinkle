import QtQuick 2.0

Rectangle {
    id: backgroundRect
    width: 150
    height: 30
    radius: 0

    property alias image: img.source
    property alias text: text.text
    property alias color: backgroundRect.color
    signal clicked

    color: "red"
    z: 2

    Image {
        id: img
        width: height
        anchors.top: parent.top
        anchors.topMargin: 2
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 2
        anchors.left: parent.left
        anchors.leftMargin: 5
        smooth: true
        source: "qrc:/qtquickplugin/images/template_image.png"
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        onClicked: parent.clicked()
    }

    Text {
        id: text
        text: "Button text"
        font.bold: true
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        anchors.right: parent.right
        anchors.rightMargin: 0
        anchors.left: img.right
        anchors.leftMargin: 5
        anchors.top: parent.top
        anchors.topMargin: 0
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 0
        color: "white"
        font.pixelSize: 12
    }

    states: State {
        name: "pressed"; when: mouseArea.pressed
        PropertyChanges { target: backgroundRect; color: Qt.darker(color) }
    }
}

