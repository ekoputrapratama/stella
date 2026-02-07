import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.12
import QtQuick.Dialogs 1.2

import "../" 1.0
// calculate percentage to show a progressbar
ApplicationWindow {
  visible: true
  title: qsTr("Flasher")
  minimumWidth: 850
  minimumHeight: 600
  width: 850
  height: 600
  x: Screen.width / 2 - width / 2
  y: Screen.height / 2 - height / 2
  
  MessageDialog {
    id: dialog
    text: ""
  }
  SimpleComboBox {
    id: deviceCmb
    x: 8
    y: 8
    width: 250
    height: 50
    labelWidth: 110
    label: "Vendor"
    items: ["Samsung", "Xiaomi", "Lenovo"]
    onCurrentValueChanged: {
      switch (deviceCmb.selected) {
        case "Xiaomi":
          flashViewLoader.source = "XiaomiFlashView.qml";
          break;
        case "Lenovo":
          flashViewLoader.source = "LenovoFlashView.qml";
          break;
        case "Samsung":
          flashViewLoader.source = "SamsungFlashView.qml";
          break;
      }
    }
  }
  Loader {
    id: flashViewLoader
    anchors.top: deviceCmb.bottom
    anchors.topMargin: 10
    anchors.right: parent.right
    anchors.rightMargin: 0
    anchors.left: parent.left
    anchors.leftMargin: 0
    anchors.bottom: parent.bottom
    anchors.bottomMargin: 0
    source: "SamsungFlashView.qml"
  }
  Component.onCompleted: {
    console.log("Hello world")
  }
}
