import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls 1.4 as QQC1
import QtQuick.Dialogs 1.3
import "/common/utils.mjs" as Utils
import "/common/vars.mjs" as Vars

import "../" 1.0

Item {
  id: control
  anchors.fill: parent
  property string packageType: "odin"
  property var flashableFiles: []
  MessageDialog {
    id: dialog
    text: ""
  }
  SimpleComboBox {
    id: packageTypeCmb
    x: 8
    y: 8
    width: 350
    height: 50
    labelWidth: 110
    label: "Package Type"
    items: ["Odin", "Heimdall", "Flashable Zip"]
    onCurrentValueChanged: {
      console.log("onCurrentValueChanged")
      switch (packageTypeCmb.selected) {
      case "Odin":
        control.packageType = "odin"
        packagePicker.visible = false
        flashableZipTable.visible = false
        addZip.visible = false;
        blPicker.visible = true
        apPicker.visible = true
        cscPicker.visible = true
        cpPicker.visible = true
        userDataPicker.visible = true
        break
      case "Heimdall":
        control.packageType = "heimdall"
        packagePicker.visible = true
        flashableZipTable.visible = false
        addZip.visible = false;
        blPicker.visible = false
        apPicker.visible = false
        cscPicker.visible = false
        cpPicker.visible = false
        userDataPicker.visible = false
        break
      case "Flashable Zip":
        control.packageType = "flashable-zip"
        flashableZipTable.visible = true
        addZip.visible = true;
        packagePicker.visible = false
        blPicker.visible = false
        apPicker.visible = false
        cscPicker.visible = false
        cpPicker.visible = false
        userDataPicker.visible = false
        break
      }
    }
  }

  FilePicker {
    id: packagePicker
    anchors.right: parent.right
    anchors.rightMargin: 10
    anchors.top: packageTypeCmb.bottom
    anchors.topMargin: 10
    anchors.left: parent.left
    anchors.leftMargin: 10
    label: "Firmware Package"
    labelWidth: 110
    selectFolder: false
  }
  FilePicker {
    id: blPicker
    anchors.right: parent.right
    anchors.rightMargin: 10
    anchors.top: packageTypeCmb.bottom
    anchors.topMargin: 10
    anchors.left: parent.left
    anchors.leftMargin: 10
    label: "BL"
    labelWidth: 110
    selectFolder: false
  }

  FilePicker {
    id: apPicker
    anchors.right: parent.right
    anchors.rightMargin: 10
    anchors.top: blPicker.bottom
    anchors.topMargin: 10
    anchors.left: parent.left
    anchors.leftMargin: 10
    label: "AP"
    labelWidth: 110
    selectFolder: false
  }
  FilePicker {
    id: cpPicker
    anchors.right: parent.right
    anchors.rightMargin: 10
    anchors.top: apPicker.bottom
    anchors.topMargin: 10
    anchors.left: parent.left
    anchors.leftMargin: 10
    label: "CP"
    labelWidth: 110
    selectFolder: false
  }
  FilePicker {
    id: cscPicker
    anchors.right: parent.right
    anchors.rightMargin: 10
    anchors.top: cpPicker.bottom
    anchors.topMargin: 10
    anchors.left: parent.left
    anchors.leftMargin: 10
    label: "CSC"
    labelWidth: 110
    selectFolder: false
  }
  FilePicker {
    id: userDataPicker
    anchors.right: parent.right
    anchors.rightMargin: 10
    anchors.top: cscPicker.bottom
    anchors.topMargin: 10
    anchors.left: parent.left
    anchors.leftMargin: 10
    label: "USERDATA"
    labelWidth: 110
    selectFolder: false
  }
  FileDialog {
    id: zipPicker
    selectFolder: false
    onAccepted: {
      const path = zipPicker.fileUrl.toString().replace("file://", "");
      const filename = stella.filename(path);
      if(!filename.endsWith(".zip")) {
        dialog.title = "Error";
        dialog.icon = StandardIcon.Critical;
        dialog.text = "The file you selected is not a valid flashable zip file.";
        dialog.open();
        return;
      }
      let file = {num: `${flashableZipTable.rowCount + 1}`, filename};
      flashableZipTable.model.append(file);
      file.path = path;
      flashableFiles[flashableFiles.length] = file;
    }
  }
  Button {
    id: addZip
    text: qsTr("Add Zip")
    anchors.top: packageTypeCmb.bottom
    anchors.topMargin: 10
    anchors.left: parent.left
    anchors.leftMargin: 10
    onClicked: {
      zipPicker.open();
    }
  }
  QQC1.TableView {
    id: flashableZipTable
    anchors.right: parent.right
    anchors.rightMargin: 10
    anchors.top: addZip.bottom
    anchors.topMargin: 10
    anchors.left: parent.left
    anchors.leftMargin: 10
    QQC1.TableViewColumn {
        role: "num"
        title: "#"
        width: 30
    }
    QQC1.TableViewColumn {
        role: "filename"
        title: "Filename"
        width: 350
    }
    model: ListModel {}
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
      let device = Vars.currentDevice;

      if(packageType === "flashable-zip" && device.state !== "sideload") {
        dialog.title = "Error";
        dialog.icon = StandardIcon.Critical;
        dialog.text = "Cannot flash file when device is not in sideload mode.";
        dialog.open();
        return;
      }

      if(packageType === "flashable-zip") {
        let promise = Promise.resolve();
        for(let i=0;i<flashableFiles.length;i++) {
          promise = promise.then(_ => {
            let file = flashableFiles[i];
            return Utils.sideload(device.serial, file.path).then(_ => {
              console.log(`flashing file ${file.filename} finished!`);
            });
          });
        }
      }
    }
  }
  Button {
    id: rebootBtn
    y: 427
    text: qsTr("Reboot Device")
    anchors.left: flashBtn.right
    anchors.leftMargin: 10
    anchors.bottom: parent.bottom
    anchors.bottomMargin: 10
    // enabled: false
    onClicked: function () {
      let device = Vars.currentDevice
      flasher.reboot("odin")
    }
  }
  function length(o) {
    return Utils.length(o);
  }
  function updateButtonState() {
    if(length(Vars.devices) > 0) {
      flashBtn.enabled = true;
      // restoreBtn.enabled = true;
    } else {
      flashBtn.enabled = false;
      // restoreBtn.enabled = false;
    }
  }
  Component.onCompleted: {
    if(!Vars.initialized) {
      Vars.events.once('app-ready',() => {
        updateButtonState();
      });
    } else {
      updateButtonState();
    }

    Vars.events.on('device-added', () => {
      updateButtonState();
    });
    Vars.events.on('device-changed', () => {
      updateButtonState();
    });
    Vars.events.on('device-removed', () => {
      updateButtonState();
    });
  }
}

/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
##^##*/
