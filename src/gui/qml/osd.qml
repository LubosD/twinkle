import QtQuick 1.1

Rectangle {
    id: rectangle1
    width: 310
    height: 55
    color: "black"

    Image {
        id: image1
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 5
        anchors.top: parent.top
        anchors.topMargin: 5
        anchors.left: parent.left
        anchors.leftMargin: 5
        source: "qrc:/icons/images/twinkle48.png"
        width: height
    }

    ImageButton {
        id: hangup
        objectName: "hangup"
        x: 262
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 15
        anchors.top: parent.top
        anchors.topMargin: 15
        anchors.right: parent.right
        anchors.rightMargin: 10
        width: height
        image: "qrc:/icons/images/osd_hangup.png"
    }

    ImageButton {
        id: mute
        objectName: "mute"
        x: 222
        width: height
        image: "qrc:/icons/images/osd_mic_on.png"
        anchors.bottomMargin: 15
        anchors.topMargin: 15
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.rightMargin: 14
        anchors.right: hangup.left
    }

    Text {
        id: callerName
        objectName: "callerName"
        x: 56
        y: 5
        width: 158
        height: 21
        text: "Caller name"
        clip: true
        verticalAlignment: Text.AlignVCenter
        font.bold: true
        font.pixelSize: 12
        color: "white"
    }

    Text {
        id: callTime
        objectName: "callTime"
        x: 56
        y: 27
        width: 158
        height: 20
        text: "Time"
        clip: true
        verticalAlignment: Text.AlignVCenter
        font.pixelSize: 12
        color: "white"
    }

    MouseArea {
        anchors.fill: parent
        property variant previousPosition
        onPressed: {
            previousPosition = Qt.point(mouseX, mouseY)
        }
        onPositionChanged: {
            if (pressedButtons == Qt.LeftButton) {
                var dx = mouseX - previousPosition.x
                var dy = mouseY - previousPosition.y
                viewerWidget.pos = Qt.point(viewerWidget.pos.x + dx,
                                            viewerWidget.pos.y + dy)
            }
        }
    }
}

