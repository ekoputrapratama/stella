import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.12
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.15
import QtQuick.Controls 1.4 as QQC1
import QtQuick.Controls.Styles 1.4 as QQCS1

import "../" 1.0
import "/common/utils.mjs" as Utils
import "/common/vars.mjs" as Vars
// calculate percentage to show a progressbar
ApplicationWindow {
  visible: true
  title: qsTr("Unlocker")
  minimumWidth: 850
  minimumHeight: 600
  width: 850
  height: 600
  x: Screen.width / 2 - width / 2
  y: Screen.height / 2 - height / 2
  property int currentProgress: 0
  property int maxProgress: 0
  property string progressMessage: ""

  MessageDialog {
    id: dialog
    text: ""
  }
  function length(o) {
    return Utils.length(o);
  }

  function wipeData() {
    let device = Vars.currentDevice;

    stella.adb.shell(device.serial, ['recovery', '--wipe_data'])
  }
  
  function updateControlState() {
    if(length(Vars.devices) > 0) {
      unlockBtn.enabled = true;
      enableAdbBtn.enabled = true;
      factoryResetBtn.enabled = true;
      manufacturerCmb.enabled = true;
      nameCmb.enabled = true;
      modelCmb.enabled = true;
    } else {
      unlockBtn.enabled = false;
      enableAdbBtn.enabled = false;
      factoryResetBtn.enabled = false;
      manufacturerCmb.enabled = false;
      nameCmb.enabled = false;
      modelCmb.enabled = false;
    }
  }

  function enableAdb() {
    let device = Vars.currentDevice;
    let sep = Vars.separator;

    console.info(`enabling adb for device ${device.model}`)
    Utils.mountPartition(device, ["system", "data"])
    
    let files = Vars.adbFiles.map(f => Utils.cloneObject(f));
    let f = files[0];
    
    for(let i=0; i<files.length; i++) {
      let f = files[i];

      if(Utils.fileExists(device, f)) {
        if(f.denied) {
          console.info(`access to file ${f.name} is denied`)
        }
      } else {
        console.info(`file ${f.name} doesn't exists`)
      }
    }
    
    let existingFiles = files.filter((i) => i.exists);
    let promise = Promise.resolve();

    existingFiles.forEach(file => {
      promise = promise.then(_ => {
        let f = file;
        let dest = stella.tempDir + sep + f.name;
        f.local = dest;
        return Utils.pullFile(device, f, dest).then((args) => {
          if(args[0] === 0) {
              f.pulled = true;
          } else {
              f.pulled = false;
          }
          return f;
        })
      })
    });
    
    promise = promise.then((args) => {
      let pulled = existingFiles.filter(i => i.pulled);
      let notPulled = Vars.adbFiles.length - pulled.length;
      console.info(`${pulled.length} file was pulled and ${notPulled} file cannot be pulled`);
      let p = Promise.resolve();
      if(pulled.length > 0){
        
        for(let file of pulled) {
          if(file.name === 'settings.db') {
            sqlite3.connect(file.local);
            sqlite3.open();
            let list = sqlite3.select("select name from global where name='adb_enabled'");
            
            if(list.length > 0) {
              sqlite3.exec("update global set value=1 where name='adb_enabled'");
            }
            sqlite3.close();
          } else if(file.name === 'persist.sys.usb.config') {
            sed.subtitute('mtp','mtp,adb', file.local);
          } else if(file.name === 'com.android.settings_preferences.xml') {
            sed.subtitute('name="enable_adb" value="false"','name="enable_adb" value="true"', file.local)
          } else if(file.name === 'persistent_properties') {
            
          } else if(file.name === 'build.prop' || file.name === 'default.prop') {
            sed.subtitute('persist.sys.usb.config=mtp','persist.sys.usb.config=adb', file.local)
          }
        }
      }
      return pulled;
    });

    promise = promise.then(_ => {
      let p = Promise.resolve();
      existingFiles.forEach(file => {
        p = p.then(_ => {
          let f = file;
          let dest = stella.tempDir + sep + f.name;
          return Utils.pushFile(device, f.local, f.path).then((args) => {
            return f;
          })
        })
      });
      return p;
    }).then(_=> {
      const pubkey = stella.getAdbPubKey();
      if(device.isRooted && !device.isRootUser){
        stella.shell(device.serial, ['su', '-c', `'echo "${pubkey}" > /data/misc/adb/adb_keys'`]);  
      } else {
        stella.shell(device.serial, ['echo', `"${pubkey}"`,">","/data/misc/adb/adb_keys"]);
      } 
      console.info("enabling adb finished, rebooting...")
      Utils.reboot(device.serial, '');
    });
  }

  // all method to bypass lockscreen
  function startUnlockFastboot() {
    if(!Vars.currentDevice) {
      console.error('no device to unlock');
      return;
    }
    let device = Vars.currentDevice;

    console.info(`temporarily flash custom recovery to device ${device.serial}`);
    device.nextState = 'recovery';
    // TODO: change to device specific twrp
    Utils.bootRecovery(device.serial, 'twrp-lenovo-vibe-k5-plus.img').then(_ => {
      console.info(`device ${device.serial} booted into custom recovery`);
      // some custom recovery has splash screen, so we need to delay the execution to remove key files
      delay(5000, () => {
        console.info(`try removing key files directly`);
        Utils.runShell(device.serial, ['rm', '/data/system/*.key']).then(([c, o, e]) => {
            
          if(c === 0) {
            console.info('removing key files success! rebooting device...');
          } else {
            console.info('removing key files failed with status code', c, o, e)
          }
          
          stella.shell(device.serial, ['reboot'], null);
          nextDevice();
        });
      });
    });
  }

  function startUnlockSideload() {
    if(!Vars.currentDevice) {
      console.error('no device to unlock');
      return;
    }
    let device = Vars.currentDevice;
    let sep = Vars.separator;
    let appDir = stella.appDir;
    let file = appDir + sep + 'files' + sep + 'screenlock-bypass.zip';
    Utils.sideload(device.serial, file).then(([c, o, e]) => {
      stella.shell(device.serial, ['reboot'], null);

      nextDevice();
    });
  }
  
  function startUnlockDevice() {
    if(!Vars.currentDevice) {
      console.error('no device to unlock');
      return;
    }
    let device = Vars.currentDevice;
    device.nextState = 'fastboot';
    console.info(`rebooting device ${device.serial} into fastboot`);
    Utils.reboot(device.serial, 'bootloader').then(() => {
      console.info(`device ${device.serial} booted into fastboot`);
      startUnlockFastboot();
    });
  }

  function downloadDeviceRecovery() {
    let manufacturer = manufacturerCmb.selected;
    let model = modelCmb.selected;
    let name = nameCmb.selected;
    let s = Vars.separator;
    let recoveryDb = stella.dataDir + s + "files" + s + "recovery.db";

    sqlite3.connect(recoveryDb);
    sqlite3.open();

    let links = sqlite3.select(`select link from recovery where manufacturer='${manufacturer}' and 
    name='${name}' and model='${model}'`);
    
    sqlite3.close();
  }
  
  function nextDevice(){
    let prevDevice = Vars.currentDevice;
    let queue = Vars.queue;
    
    if(prevDevice)
      queue.removeBySerial(prevDevice.serial);
    
    if(!queue.isEmpty()) {
      let device = queue.first;
      Vars.setCurrentDevice(device);
      if(device.state === 'fastboot') {
        startUnlockFastboot();
      } else if(device.state === 'device') {
        startUnlockDevice();
      } else if(device.state === 'sideload') {
        startUnlockSideload();
      } else if(device.state ==='unauthorized') {
        dialog.title = "Error";
        dialog.icon = StandardIcon.Critical;
        dialog.text = "Cannot communicate with device without adb.";
        dialog.open();
      }
    }
  }

  function refreshManufacturer() {
    let s = Vars.separator;
    let dataDir = stella.dataDir;
    let recoveryDb = dataDir + s + "files" + s + "recovery.db";
    manufacturerCmb.items = [];

    sqlite3.connect(recoveryDb);
    sqlite3.open();

    let manufacturers = sqlite3.select("select distinct manufacturer from recovery");
    manufacturerCmb.items = manufacturers.map(item => item.manufacturer);
    sqlite3.close();
  }

  function refreshDeviceName() {
    let manufacturer = manufacturerCmb.selected;
    let s = Vars.separator;
    let dataDir = stella.dataDir;
    let recoveryDb = dataDir + s + "files" + s + "recovery.db";
    nameCmb.items = [];

    sqlite3.connect(recoveryDb);
    sqlite3.open();

    let names = sqlite3.select(`select name from recovery where manufacturer='${manufacturer}'`);
    nameCmb.items = names.map(item => item.name);
    sqlite3.close();
  }

  function refreshDeviceModel() {
    const manufacturer = manufacturerCmb.selected;
    const name = nameCmb.selected;
    const s = Vars.separator;
    const dataDir = stella.dataDir;
    const recoveryDb = dataDir + s + "files" + s + "recovery.db";
    modelCmb.items = [];

    sqlite3.connect(recoveryDb);
    sqlite3.open();

    let models = sqlite3.select(`select model from recovery where manufacturer='${manufacturer}' and 
    name='${name}'`);
    modelCmb.items = models.map(item => item.model);
    sqlite3.close();
  }
  
  SimpleComboBox {
    id: manufacturerCmb
    anchors.left: parent.left
    anchors.leftMargin: 8
    anchors.topMargin: 8
    width: 350
    height: 50
    labelWidth: 150
    label: "Manufacturer"
    onCurrentValueChanged: {
      refreshDeviceName();
    }
  }

  SimpleComboBox {
    id: nameCmb
    anchors.left: parent.left
    anchors.top: manufacturerCmb.bottom
    anchors.topMargin: 8
    anchors.leftMargin: 8
    width: 350
    height: 50
    labelWidth: 150
    label: "Name"
    onCurrentValueChanged: {
      refreshDeviceModel();
    }
  }

  SimpleComboBox {
    id: modelCmb
    anchors.left: parent.left
    anchors.top: nameCmb.bottom
    anchors.topMargin: 8
    anchors.leftMargin: 8
    width: 350
    height: 50
    labelWidth: 150
    label: "Model"
  }
  
  QQC1.ProgressBar {
    id: progress
    height: 20
    value: currentProgress
    maximumValue: maxProgress
    anchors.left: parent.left
    anchors.leftMargin: 10
    anchors.right: parent.right
    anchors.rightMargin: 10
    anchors.bottom: unlockBtn.top
    anchors.bottomMargin: 5
    anchors.topMargin: 5
    style: QQCS1.ProgressBarStyle {
      background: Rectangle {
        radius: 4
        color: "#3C3F41"
        implicitWidth: 200
        implicitHeight: 20
      }
    }
    Text {
      text: progressMessage
      anchors.centerIn: parent
    }
  }

  Button {
    id: unlockBtn
    y: 427
    text: qsTr("Start Unlock")
    anchors.left: parent.left
    anchors.leftMargin: 10
    anchors.bottom: parent.bottom
    anchors.bottomMargin: 10
    enabled: false
    onClicked: {
      let queue = Vars.queue;
      if (queue.isEmpty()) {
        dialog.title = "Error"
        dialog.icon = StandardIcon.Critical
        dialog.text = "No device detected, please plug your device that you want to unlock."
        dialog.open()
        return
      }

      if(device.state === 'fastboot') {
        startUnlockFastboot();
      } else if(device.state === 'device') {
        startUnlockDevice();
      } else if(device.state === 'sideload') {
        startUnlockSideload();
      } else if(device.state ==='unauthorized') {
        dialog.title = "Error";
        dialog.icon = StandardIcon.Critical;
        dialog.text = "Cannot communicate with device without adb.";
        dialog.open();
      }
    }
  }
  Button {
    id: factoryResetBtn
    y: 427
    text: qsTr("Factory Reset")
    anchors.left: unlockBtn.right
    anchors.leftMargin: 10
    anchors.bottom: parent.bottom
    anchors.bottomMargin: 10
    enabled: false
    onClicked: {
      let device = Vars.currentDevice;

      if(device.state === 'recovery' || device.isRootUser) {
        wipeData();
      } else if(device.state === 'device') {
        
      }
    }
  }

  Button {
    id: enableAdbBtn
    y: 427
    text: qsTr("Enable ADB")
    anchors.left: factoryResetBtn.right
    anchors.leftMargin: 10
    anchors.bottom: parent.bottom
    anchors.bottomMargin: 10
    enabled: false
    onClicked: {
      let queue = Vars.queue;
      if (queue.isEmpty()) {
        dialog.title = "Error"
        dialog.icon = StandardIcon.Critical
        dialog.text = "No device detected, please plug your device that you want to unlock."
        dialog.open()
        return
      }
      let device = Vars.currentDevice;

      // on some recovery like Philz Touch recovery, the device gave 
      // device state instead of recovery state
      // but the shell was running as root
      if(device.state === 'device' || device.state === 'recovery') {
        enableAdb();
      } else if(device.state === 'sideload') {
        
      } else if(device.state === 'fastboot') {

      }
    }
  }
  
  Component.onCompleted: {
    if(!Vars.initialized) {
      Vars.events.once('app-ready', () => {
        updateControlState();
      });
    } else {
      updateControlState();
    }

    Vars.events.on('device-added', () => {
      updateControlState();
    });
    Vars.events.on('device-changed', () => {
      updateControlState();
    });
    Vars.events.on('device-removed', () => {
      updateControlState();
    });

    refreshManufacturer();
  }
}
