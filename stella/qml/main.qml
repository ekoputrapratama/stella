import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.12
import QtQuick.Dialogs 1.2
import QtGraphicalEffects 1.0
import QtQuick.Controls 1.4 as QQC1
import QtQuick.Controls.Material 2.12
import QtQuick.Layouts 1.3 

import "/common/vars.mjs" as Vars
import '/common/queue.mjs' as Queue
import "/common/utils.mjs" as Utils

ApplicationWindow {
  id: mainWindow
  visible: true
  title: qsTr("Stella")
  minimumWidth: 850
  minimumHeight: 720
  width: 850
  height: 600
  // x: Screen.width / 2 - width / 2
  // y: Screen.height / 2 - height / 2

  // menuBar: MenuBar {
    // contentHeight: 20
    // Menu {
    //   implicitHeight: 50  
    //   title: "File"
    //   MenuItem { text: "Open..." }
    //   MenuItem { text: "Close" }
    // }
  // }

  WindowStateSaver {
    window: mainWindow
    windowName: "Stella"
    defaultX: Screen.width / 2 - width / 2
    defaultY: Screen.height / 2 - height / 2
  }

  MessageDialog {
    id: dialog
    text: ""
  }

  Timer {
    id: timer
  }
  function onConsoleLog(text) {
    if(typeof text !== 'string')
      text = JSON.stringify(text);

    // logs.append(text);
  }

  function checkAdbPermission() {
    let devices = Vars.devices;

    for(let serial in devices) {
      let device = devices[serial];

      if(device.state === 'unauthorized') {
        console.info(`Device ${device.model} cannot be unlocked through adb, removing it from the list.`)
      }
    }
  }

  function checkRootStatus(device){
    let result = stella.adb.shell(device.serial, ['su','-v']);
    let output = result.output;
    let suName = output.replace(/\r|\n/g,'')
    let names = output.split(/\n/);
    let isRooted = false;
    let isRootUser = false;
    if(names && names.length > 0 && !suName.endsWith('not found')){
      suName = Utils.clearText(names[0]);
      isRooted = true;
    }
    
    // on some recovery like Philz Touch recovery, the device gave 
    // device state instead of recovery state
    // but the shell was running as root
    if(!isRooted) {
      result = stella.adb.shell(device.serial, ['whoami']);
      let user = Utils.clearText(result.output);
      isRooted = user === "root";
      suName = "Unknown";
      isRootUser = user === "root";
    } else {
      result = stella.adb.shell(device.serial, ['whoami']);
      let user = Utils.clearText(result.output);
      isRootUser = user === "root";
    }

    console.info(`Root status     : ${isRooted}`);
    device.isRooted = isRooted;
    device.isRootUser = isRootUser;
    if(device.isRooted)
      console.info(`Superuser       : ${suName.replace(/\r\n/,'')}`);
  }

  function parseDevices(list) {
      
    let devices = Vars.devices;
    for(let d of list) {
      let sp = d.split(/\t/);
      let device = {
        serial: sp[0],
        state: sp[1],
        status: 'pending',
      }
      devices[sp[0]] = Object.assign(devices[sp[0]] || {}, device);
    }

    logs.append(`${Object.keys(devices).length} devices found`);
    return devices
  }

  function getDeviceInfo(device) {
    
    if(device.state === 'device' || device.state === 'recovery'){
      let manufacturer = stella.getProp(device['serial'], 'ro.product.manufacturer');
      let name = stella.getProp(device['serial'], 'ro.product.name');
      let brand = stella.getProp(device['serial'], 'ro.product.brand');
      let cpuAbi = stella.getProp(device['serial'], 'ro.product.cpu.abi');
      let version = stella.getProp(device['serial'], 'ro.build.version.release');
      let model = stella.getProp(device['serial'], 'ro.product.model');
      let sdk = stella.getProp(device['serial'], 'ro.build.version.sdk');
      
      let preview = stella.getProp(device['serial'], 'ro.build.version.preview_sdk');
      
      
      if(model === name) {
        model = stella.getProp(device['serial'], 'ro.product.device');
      }

      sdk = parseInt(sdk);

      try {
        
        if(parseInt(preview) > 0) {
          sdk += 1;
        }
        
      } catch(e) {
        console.log("cannot get preview sdk version")
      }

      if(!device.isUsbDevice) {
        let id = stella.getProp(device['serial'], 'ro.serialno');
        device['id'] = Utils.clearText(id);
      }

      manufacturer = Utils.clearText(manufacturer);
      brand = Utils.clearText(brand);
      name = Utils.clearText(name);
      model = Utils.clearText(model);
      cpuAbi = Utils.clearText(cpuAbi);
      version = Utils.clearText(version);

      device['manufacturer'] = manufacturer;
      device['name'] = name;
      device['brand'] = brand;
      device['model'] = model;
      device['sdk'] = sdk;
      device['cpuAbi'] = cpuAbi;
      device['version'] = parseFloat(version);

      console.info(`==================================`);
      if(device['state'] !== 'unauthorized'){
        console.info(`ID              : ${device.id}`);
        console.info(`Serial          : ${device.serial}`);
        console.info(`Mode            : ${device['state']}`);
        console.info(`Vendor ID       : ${device['vendorId']}`);
        console.info(`Product ID      : ${device['productId']}`);
        console.info(`Manufacturer    : ${manufacturer}`);
        console.info(`Brand           : ${brand}`);
        console.info(`Name            : ${name}`);
        console.info(`Model           : ${model}`);
        console.info(`CPU ABI         : ${cpuAbi}`);
        console.info(`Android Version : ${device.version}`);
        console.info(`Sdk Version     : ${device.sdk}`);
        checkRootStatus(device);
      } else {
        console.info("Cannot get device information")
      }
      console.info(`==================================`);
    } else {
      console.info(`==================================`);
      console.info(`Mode : ${device.state}`);
      if(device.manufacturer && device.manufacturer.length > 0) {
        console.info(`Manufacturer : ${device.manufacturer}`);
      }
      if(device.model && device.model.length > 0) {
        console.info(`Model : ${device.model}`);
      }
      if(device.vendorId && device.vendorId.length > 0) {
        console.info(`Vendor ID : ${device['vendorId']}`);
      }
      if(device.productId && device.productId.length > 0) {
        console.info(`Product ID : ${device['productId']}`);
      }
      console.info(`==================================`);
    }
  }

  

  function onDeviceAdded(device) {
    console.info('new device detected');
    // refreshDevices('add');
    Vars.events.emit('device-added',device);
    // console.log('device', JSON.stringify(device))
  }
  function onDeviceRemoved(device) {
    console.info('device removed');
    refreshDevices();
    Vars.events.emit('device-removed', device);
    // console.log('device', JSON.stringify(device))
  }
  function onDeviceChanged(device) {
    refreshDevices();
    console.info('device changed', JSON.stringify(device));
    // if(!Vars.devices[device.serial]) {
    //   Vars.devices[device.serial] = device;
    //   getDeviceInfo(Vars.devices[device.serial]);
    // }
    Vars.events.emit('device-changed', device);
  }

  function refreshDevices() {
    let devices = stella.getDevices().map(d => {
      let device = Object.assign({status: 'pending'}, Utils.cloneObject(d));
      return device;
    }).reduce((prev, current) => {
      if(current)
          prev[current['serial']] = current;
      return prev;
    }, {});
    
    // console.log('devices',JSON.stringify(stella.getDevices()))
    for(let serial in devices) {
      let device = devices[serial];
      if(device.state == "offline"){
        continue;
      } 
      getDeviceInfo(device);
    }
    
    let newSerials = Object.keys(devices);
    let serials = Object.keys(Vars.devices);

    for(let serial of serials) {
      let oldDevice = Vars.devices[serial];
      if(!newSerials.includes(serial) && !oldDevice.nextState) {
        delete Vars.devices[serial];
      } else if(newSerials.includes(serial) && oldDevice.nextState) {
        let newDevice = devices[serial];
        if(oldDevice.nextState === newDevice.state) {
          delete newDevice.nextState;
          Vars.devices[serial] = Object.assign(oldDevice, newDevice);
        }
      }
    }

    for(let serial of newSerials) {
      let newDevice = devices[serial];
      if(!serials.includes(serial)) {
        Vars.devices[serial] = newDevice;
      }
    }

    let queue = new Queue.DeviceQueue(devices);
    Vars.setQueue(queue);
  }
  Component {
    id: itemDelegate
    Item {
      width: grid.cellWidth
      height: grid.cellHeight
      required property string icon
      required property string name
      required property int index

      Card {
        id: item
        width: grid.cellWidth - 20
        height: grid.cellHeight - 20
        Material.elevation: 6
        // backgroundColor: "#2F3336"
        radius: 5
        anchors.centerIn:parent
        anchors.leftMargin: 5
        anchors.rightMargin: 5
        
        Item {
          id: iconContainer
          height: parent.height * 0.8
          anchors.top: parent.top
          anchors.topMargin: 0
          anchors.left: parent.left
          anchors.leftMargin: 0
          anchors.right: parent.right
          anchors.rightMargin: 0
          Image {
            id: iconImg
            source: icon
            height: parent.height - 40
            width: grid.cellWidth
            fillMode: Image.PreserveAspectFit
            anchors.centerIn: parent
          }
        }
        

        Label {
          text: name
          // color: "#ffffff"
          anchors.top: iconContainer.bottom
          anchors.left: parent.left
          anchors.leftMargin: 0
          anchors.right: parent.right
          anchors.rightMargin: 0
          anchors.bottom: parent.bottom
          anchors.bottomMargin: 0
          verticalAlignment: Label.AlignVCenter
          horizontalAlignment: Label.AlignHCenter
        }
        MouseArea {
          anchors.fill: parent
          onClicked: {
            grid.currentIndex = index;
            pluginsModel.launch(index);
          }
        }
      }
    }
  }
  
  ColumnLayout {
    id: myColumn
    anchors.fill: parent
    anchors.top: parent.top
    anchors.topMargin: 20
    anchors.left: parent.left
    anchors.leftMargin: 20
    anchors.right: parent.right
    anchors.rightMargin: 20
    anchors.bottom: parent.bottom
    // anchors.centerIn: parent
    spacing: 10 // Optional: Adds 10 pixels of spacing between items
    
    Image {
      id: logo
      source: "/stella.png"
      Layout.preferredWidth: 150
      Layout.preferredHeight: 150
      width: 150
      height: 150
      fillMode: Image.PreserveAspectFit
      Layout.fillHeight: false
      Layout.alignment: Qt.AlignHCenter
      Layout.bottomMargin: 10
    }
    Text {
      text: "All in one Android utilities"
      verticalAlignment: Label.AlignVCenter
      horizontalAlignment: Label.AlignHCenter
      color: "#ffffff"
      font.bold: true
      font.pixelSize: 24
      Layout.alignment: Qt.AlignHCenter
    }
    Item {
      id: gridContainer
      Layout.preferredHeight: 400
      Layout.topMargin: 0
      Layout.fillWidth: true
      property int column: 3
      GridView {
        id: grid
        cellWidth: parent.width / parent.column
        cellHeight: 200
        model: pluginsModel
        width: parent.width
        height: (grid.count % 3 > 0) ? cellHeight * Math.ceil(grid.count / 3) : cellHeight * Math.floor(grid.count / 3)
        delegate: itemDelegate
        // highlight: Rectangle { color: "lightsteelblue"; radius: 5 }
      }
    }
  }
  
  
  
  Component.onCompleted: {
    if(grid.height < (gridContainer.height)) {
      let rowCount;
      let columnCount = gridContainer.column;
      if(grid.count % 3 > 0) {
        rowCount = Math.ceil(grid.count / 3)
      } else {
        rowCount = Math.floor(grid.count / 3);
      }

      let emptySpace = (gridContainer.height) - (grid.height);
      let margin = (emptySpace / 2);
      // grid.anchors.topMargin = margin;
    }

    Vars.setStella(stella);
    Vars.setTimer(timer);
    Vars.setAdb(stella.adb);
    Vars.setFastboot(stella.fastboot);
    
    stella.adb.startServer(c => {
      refreshDevices();

      let devices = Vars.devices;
        
      let queue = new Queue.DeviceQueue(devices);
      Vars.setQueue(queue);
      for(let serial in devices) {
        let device = devices[serial];
        queue[serial] = device;
      }
      
      if(!queue.isEmpty() && queue.hasAuthorizedDevice()) {
        let device = queue.firstAuthorizedDevice;
        
        Vars.setCurrentDevice(Utils.cloneObject(device));
        // queue.removeBySerial(device.serial);
        // device = Vars.currentDevice;
        // removeKeyFiles(device.serial);
      } else if(!queue.isEmpty() && !queue.hasAuthorizedDevice()){
        let device = queue.first;
        Vars.setCurrentDevice(Utils.cloneObject(device));
      }
      Vars.events.emit('app-ready');
      Vars.setInitialized(true);
      try {
        stella.watcher.onDeviceAdded.connect(onDeviceAdded);
        stella.watcher.onDeviceRemoved.connect(onDeviceRemoved);
        stella.watcher.onDeviceChanged.connect(onDeviceChanged);
      } catch(e){
        console.log(e);
      }
    });
  }
}
