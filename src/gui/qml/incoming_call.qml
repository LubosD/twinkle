import QtQuick 2.0

Rectangle {
    id: rectanglePopup
    width: 400
    height: 70
    color: "black"

    signal moved

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

    Text {
        id: callerText
        objectName: "callerText"
        height: 22
        color: "#ffffff"
        text: "... calling"
        anchors.top: parent.top
        anchors.topMargin: 8
        anchors.left: image1.right
        anchors.leftMargin: 9
        anchors.right: parent.right
        anchors.rightMargin: 10
        font.pixelSize: 19
    }

    TextImageButton {
        id: buttonAnswer
        objectName: "buttonAnswer"
        x: 74
        y: 36
        width: 120
        height: 26
        color: "#00aa00"
        radius: 7
        text: qsTr("Answer")
        image: "qrc:/icons/images/popup_incoming_answer.png"
    }

    TextImageButton {
        id: buttonReject
        objectName: "buttonReject"
        y: 36
        width: 120
        height: 26
        radius: 7
        text: qsTr("Reject")
        anchors.left: buttonAnswer.right
        anchors.leftMargin: 15
        image: "qrc:/icons/images/popup_incoming_reject.png"
    }

    MouseArea {
        anchors.fill: parent
        property real lastMouseX: 0
        property real lastMouseY: 0
        onPressed: {
            lastMouseX = mouseX
            lastMouseY = mouseY
        }
        onMouseXChanged: viewerWidget.x += (mouseX - lastMouseX)
        onMouseYChanged: viewerWidget.y += (mouseY - lastMouseY)
    }
}

