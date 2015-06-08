import QtQuick 1.1

Rectangle {

    property alias image: img.source
    signal clicked

    color: "transparent"
    z: 2

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        onClicked: parent.clicked()
    }

    Image {
        id: img
        smooth: true
        anchors.fill: parent
    }

    states: State {
        name: "pressed"; when: mouseArea.pressed
        PropertyChanges { target: img; anchors.topMargin: 2; anchors.bottomMargin: -2 }
    }
}

