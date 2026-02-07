import QtQuick 2.12
import QtQuick.Controls 2.12

import "/common/utils.mjs" as Utils
import "/common/vars.mjs" as Vars

import "../" 1.0

Item {
  id: element
  FilePicker {
    id: filePicker
    x: 10
    y: 10
    width: 300
    selectFolder: true
    label: "ROM Folder"
    labelWidth: 110
    anchors.right: parent.right
    anchors.rightMargin: 10
    anchors.left: parent.left
    anchors.leftMargin: 10
  }

  GroupBox {
    id: mode
    contentHeight: 45
    anchors.right: parent.right
    anchors.rightMargin: 10
    title: "Mode"
    anchors.top: filePicker.bottom
    anchors.topMargin: 10
    anchors.left: parent.left
    anchors.leftMargin: 10
    RadioButton {
      id: fastbootRb
      text: qsTr("Fastboot")
      anchors.left: parent.left
      anchors.leftMargin: 10
      anchors.top: parent.top
      anchors.topMargin: 8
      checked: true
      onCheckedChanged: {
        if (fastbootRb.checked) {
          keepDataChk.visible = true
          lockCrcChk.visible = true
        } else {
          keepDataChk.visible = false
          lockCrcChk.visible = false
        }
      }
    }

    RadioButton {
      id: edlRb
      text: qsTr("EDL")
      anchors.left: fastbootRb.right
      anchors.leftMargin: 10
      anchors.top: parent.top
      anchors.topMargin: 8
      onCheckedChanged: {
        if (fastbootRb.checked) {
          keepDataChk.visible = true
          lockCrcChk.visible = true
        } else {
          keepDataChk.visible = false
          lockCrcChk.visible = false
        }
      }
    }
  }

  CheckBox {
    id: keepDataChk
    x: 8
    y: 8
    anchors.top: mode.bottom
    anchors.topMargin: 10
    anchors.left: parent.left
    anchors.leftMargin: 10
    text: qsTr("Keep data")
  }

  CheckBox {
    id: lockCrcChk
    x: 8
    y: 8
    anchors.top: mode.bottom
    anchors.topMargin: 10
    anchors.left: keepDataChk.right
    anchors.leftMargin: 10
    text: qsTr("Lock CRC")
  }



  Button {
    id: flashBtn
    y: 427
    text: qsTr("Start Flash")
    anchors.left: parent.left
    anchors.leftMargin: 10
    anchors.bottom: parent.bottom
    anchors.bottomMargin: 10
    // enabled: false
    onClicked: {
      let device = Vars.currentDevice
      let path = filePicker.selected.replace("file://", "")
      console.log("firmware path", path)
    }
  }
}

/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
##^##*/

