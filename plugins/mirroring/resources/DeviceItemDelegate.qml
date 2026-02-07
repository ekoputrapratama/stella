import QtQuick 2.12

Rectangle {
  id: container
  property Item exampleItem
  width: ListView.view.width
  height: button.implicitHeight + 22
  required property string name
  required property string serial
  required property url icon
  signal clicked()

  gradient: Gradient {
      GradientStop {
          position: 0
          Behavior on color {ColorAnimation { duration: 100 }}
          color: tapHandler.pressed ? "#e0e0e0" : "#fff"
      }
      GradientStop {
          position: 1
          Behavior on color {ColorAnimation { duration: 100 }}
          color: tapHandler.pressed ? "#e0e0e0" : button.containsMouse ? "#f5f5f5" : "#eee"
      }
  }

  Image {
    id: image
    opacity: 0.7
    Behavior on opacity { 
      NumberAnimation { duration: 100 } 
    }
    source: icon
    anchors.left: parent.left
    anchors.top: parent.top
    anchors.topMargin: 10
    anchors.bottomMargin: 10
    anchors.bottom: parent.bottom
    anchors.rightMargin: 16
    fillMode: Image.PreserveAspectFit
  }

  Item {
    id: button
    anchors.top: parent.top
    anchors.left: image.right
    anchors.bottom: parent.bottom
    anchors.right:image.left
    implicitHeight: col.height
    height: implicitHeight
    width: buttonLabel.width + 20
    property alias containsMouse: hoverHandler.hovered

    TapHandler {
      id: tapHandler
      onTapped: container.clicked()
    }

    HoverHandler {
      id: hoverHandler
    }

    Column {
      spacing: 2
      id: col
      anchors.verticalCenter: parent.verticalCenter
      width: parent.width
      Text {
        id: buttonLabel
        anchors.left: parent.left
        anchors.leftMargin: 10
        anchors.right: parent.right
        anchors.rightMargin: 10
        text: container.name
        color: "black"
        font.pixelSize: 22
        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
        styleColor: "white"
        style: Text.Raised

      }
      Text {
        id: buttonLabel2
        anchors.left: parent.left
        anchors.leftMargin: 10
        text: container.serial
        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
        color: "#666"
        font.pixelSize: 12
      }
    }
  }

  Rectangle {
    height: 1
    color: "#ccc"
    anchors.bottom: parent.bottom
    anchors.left: parent.left
    anchors.right: parent.right
  }
}
