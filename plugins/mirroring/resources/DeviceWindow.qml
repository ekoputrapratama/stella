import QtQuick 2.12
import QtQuick.Controls 2.12
import DeviceView 1.0
import "device.js" as Device

import "/common/utils.mjs" as Utils
import "/common/AdbUtils.mjs" as Adb
import "/common/PathUtils.mjs" as Path
import "/common/vars.mjs" as Vars

ApplicationWindow {
  id: w
  width: 480
  height: 800
  visible: true
  required property string serial
  property string progressMessage: ""
  property string progressFile: ""

  DeviceView {
    id: deviceView
    visible: true
    width: w.width
    height: w.height
  }

  DropArea {
    anchors.fill: parent
    enabled: true
    onEntered: {
      console.log("drop enter",JSON.stringify(Object.keys(drag)))
      if(!drag.hasUrls) {
        drag.accepted = false;
      }
    }
    onDropped: {
      console.log("onDrop", JSON.stringify(Object.keys(drop)))
      let files = drop.urls;
      let promise = Promise.resolve();

      for(let file of files) {
        let ext = Utils.getFileExtension(file);
        if(ext === "apk") {
          promise = promise.then(_ => {
            const path = file.replace("file://", "");
            const name = stella.filename(path);
            progressMessage = "Installing apk";
            progressFile = name;
            return Adb.install(serial, path);
          });
        } else {
          promise = promise.then(_ => {
            const path = file.replace("file://", "");
            const name = stella.filename(path);
            progressMessage = "Sending file";
            progressFile = name;
            return Adb.push(serial, path, "/sdcard/Stella/files");
          });
        }
      }
      promise = promse.then(_ => {
        console.log("handling drop finished")
      })
    }
  }

  Item {
    id: preloader
    anchors.centerIn: parent
    width: parent.width
    Label {
      id: imgLabel
      width: parent.width
      anchors.bottom: img.top
      text: qsTr("Connecting")
      horizontalAlignment: Label.AlignHCenter
    }
    AnimatedImage {
      id: img
      anchors.top: imgLabel.bottom
      source: "icons/wait.gif"
      anchors.horizontalCenter: parent.horizontalCenter
    }
  }

  Pane {
    id: dropProgress
    anchors.centerIn: parent
    width: parent.width * 0.5
    height: 100
    visible: false
    Column {
      anchors.fill: parent
      Label {
        id: progressLabel
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.topMargin: 50
        text: progressMessage
        // horizontalAlignment: Label.AlignHCenter
      }
      Label {
        id: fileLabel
        anchors.left: parent.left
        anchors.right: parent.right
        text: progressFile
        // horizontalAlignment: Label.AlignHCenter
      }
      ProgressBar {
        id: progress
        height: 50
        anchors.left: parent.left
        anchors.right: parent.right
        indeterminate: true
      }
    }
  }

  function onDeviceConnected() {
    preloader.visible = false;
  }
  
  Component.onCompleted: {
    deviceView.onDeviceConnected.connect(onDeviceConnected);
    let device = Vars.devices[serial];
    const s = Vars.separator;
    const targetDir = "/data/local/tmp";
    const minicapLibPath = Path.join(stella.minicapDir, "android-" + device.sdk, device.cpuAbi, "minicap.so");
    const minicapBinPath = Path.join(stella.minicapDir, device.cpuAbi, "minicap");

    if(Path.exists(minicapLibPath)) {
      Adb.push(device, minicapLibPath, targetDir).then(_=> {
        return Adb.push(device, minicapBinPath, targetDir);
      }).then(_ => {
        let args = []
        let execPath = targetDir + "/minicap";
        if(Utils.hasBusyBox(device)) {
          args = ["busybox", "chmod", "+x", execPath];
        } else {
          args = ["chmod", "777", execPath];
        }

        return Adb.shell(device.serial, args);
      }).then(_ => {
        deviceView.connectToDevice(serial);
      }).catch(error =>{
        console.log("cannot start minicap", error)
      }) 
    }
  }
  onClosing: {
    deviceView.destroy();
  }
}
