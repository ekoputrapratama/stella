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
  property var flashableFiles: []
  MessageDialog {
    id: dialog
    text: ""
  }

  GroupBox {
    id: mode
    contentHeight: 45
    anchors.right: parent.right
    anchors.rightMargin: 10
    title: "Mode"
    anchors.top: parent.top
    anchors.topMargin: 10
    anchors.left: parent.left
    anchors.leftMargin: 10
    RadioButton {
      id: firehoseRb
      text: qsTr("Firehose")
      anchors.left: parent.left
      anchors.leftMargin: 10
      anchors.top: parent.top
      anchors.topMargin: 8
      checked: true
      onCheckedChanged: {
        if (firehoseRb.checked) {
          programPicker.visible = true
          rawPicker.visible = true
          patchPicker.visible = true
        } else {
          programPicker.visible = false
          rawPicker.visible = false
          patchPicker.visible = false
        }
      }
    }

    RadioButton {
      id: dloadRb
      text: qsTr("Dload")
      anchors.left: firehoseRb.right
      anchors.leftMargin: 10
      anchors.top: parent.top
      anchors.topMargin: 8
      onCheckedChanged: {
        if (dloadRb.checked) {
          
        } else {
          
        }
      }
    }

    RadioButton {
      id: sideloadRb
      text: qsTr("Sideload")
      anchors.left: dloadRb.right
      anchors.leftMargin: 10
      anchors.top: parent.top
      anchors.topMargin: 8
      onCheckedChanged: {
        if(sideloadRb.checked) {
          addZip.visible = true;
          flashableZipTable.visible = true;
        } else {
          addZip.visible = false;
          flashableZipTable.visible = false;
        }
      }
    }
  }
  // Sideload controls
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
    anchors.top: mode.bottom
    anchors.topMargin: 10
    anchors.left: parent.left
    anchors.leftMargin: 10
    visible: false
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
    visible: false
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

  // Firehose controls
  FilePicker {
    id: programPicker
    anchors.right: parent.right
    anchors.rightMargin: 10
    anchors.top: mode.bottom
    anchors.topMargin: 10
    anchors.left: parent.left
    anchors.leftMargin: 10
    label: "Programmer Path"
    labelWidth: 110
    selectFolder: false
  }

  FilePicker {
    id: rawPicker
    anchors.right: parent.right
    anchors.rightMargin: 10
    anchors.top: programPicker.bottom
    anchors.topMargin: 10
    anchors.left: parent.left
    anchors.leftMargin: 10
    label: "Rawprogram XML"
    labelWidth: 110
  }

  FilePicker {
    id: patchPicker
    anchors.right: parent.right
    anchors.rightMargin: 10
    anchors.top: rawPicker.bottom
    anchors.topMargin: 10
    anchors.left: parent.left
    anchors.leftMargin: 10
    label: "Patch XML"
    labelWidth: 110
  }

  Button {
    id: flashBtn
    y: 427
    text: qsTr("Start Flash")
    anchors.left: parent.left
    anchors.leftMargin: 10
    anchors.bottom: parent.bottom
    anchors.bottomMargin: 10
    enabled: false
    onClicked: {
      let device = Vars.currentDevice;
      
      if(sideloadRb.checked) {
        if(device.state === 'sideload') {
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
