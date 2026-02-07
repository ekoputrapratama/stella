import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12

Pane {
  id: container
  width: 760
  height: 500
  visible: true
  anchors.fill: parent
  Label {
    text: "Main Form"
  }

  Component.onCompleted: {
    let devices = bnr.getDevices();
    console.log("devices", JSON.stringify(devices))
  }
}
