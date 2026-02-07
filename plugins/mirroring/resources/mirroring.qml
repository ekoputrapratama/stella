import QtQml 2.12
import QtQuick 2.12
import QtQml.Models 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.12

import "/common/utils.mjs" as Utils
import "/common/vars.mjs" as Vars

ApplicationWindow {
  id: w
  visible: true
  title: qsTr("Mirroring")
  minimumWidth: 850
  minimumHeight: 600
  width: 850
  height: 600
  x: Screen.width / 2 - width / 2
  y: Screen.height / 2 - height / 2

  ListView {
    id: launcherList
    clip: true
    delegate: DeviceItemDelegate {
      MouseArea {
        anchors.fill: parent
        onClicked: {
          var component = Qt.createComponent(Qt.resolvedUrl("DeviceWindow.qml"));
          var deviceWindow = component.createObject(w, {serial});
          deviceWindow.width = 480
          deviceWindow.height = 800 
        }
      }
    }
    model: ListModel {id:deviceModel}
    anchors.fill: parent
    enabled: opacity == 1.0
  }

  function onDeviceAdded(device) {
    console.log("add devices");
  }

  function onDeviceRemoved() {

  }

  function refreshDeviceList() {
    let devices = Vars.queue.authorizedDevices;
    for(let device of devices) {
      let isUsbDevice = stella.adb.isUsbDevice(device.serial);
      deviceModel.append({
        name: device.model, 
        serial: device.serial,
        icon: (isUsbDevice) ? Qt.resolvedUrl("icons/usb.svg") : Qt.resolvedUrl("icons/hotspot.svg"),
      });
    }
  }

  Component.onCompleted: {
    if(!Vars.initialized) {
      Vars.events.once('app-ready',() => {
        refreshDeviceList();
      });
    } else {
      refreshDeviceList();
    }

    Vars.events.on('device-added', onDeviceAdded);
    Vars.events.on('device-removed', onDeviceRemoved);
  }

  onClosing: {
  }
}
