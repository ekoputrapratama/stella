import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.12
import QtQuick.Dialogs 1.2
import QtQuick.Controls 1.4 as QQC1
import QtQuick.Controls.Styles 1.4 as QQCS1
import QtQml.Models 2.15

import "../" 1.0
import "/common/utils.mjs" as Utils
import "/common/vars.mjs" as Vars
import "bnr.mjs" as Bnr
// calculate percentage to show a progressbar
ApplicationWindow {
  visible: true
  title: qsTr("Backup & Restore")
  minimumWidth: 850
  minimumHeight: 600
  width: 850
  height: 600
  x: Screen.width / 2 - width / 2
  y: Screen.height / 2 - height / 2
  
  property int apkCount: 0
  property int currentProgress: 0
  property int maxProgress: 0
  property string progressMessage: ""
  property bool backupIsRunning: false
  property bool restoreIsRunning: false
  property bool allChecked: false
  MessageDialog {
    id: dialog
    text: ""
  }
  Dialog {
    id: devicesDialog
    // visible: true
    title: "Select Device"

    contentItem: Rectangle {
      color: "lightskyblue"
      implicitWidth: 400
      implicitHeight: 100
      Text {
        text: "Hello blue sky!"
        color: "navy"
        anchors.centerIn: parent
      }
    }
  }

  function length(o) {
    return Utils.length(o);
  }

  function updateButtonState() {
    if(length(Vars.devices) > 0 && (!backupIsRunning || !restoreIsRunning)) {
      backupBtn.enabled = true;
      restoreBtn.enabled = true;
    } else {
      backupBtn.enabled = false;
      restoreBtn.enabled = false;
    }
  }

  function refreshPackage() {
    let device = Vars.currentDevice;
    listModel.clear();
    if(device && device.state === 'device') {
      apkCount = 0;
      if(backupRb.checked) {
        allChecked = true;
        Utils.getPackages(device, true, true).forEach(pkg => {
          apkCount++;
          apkTable.model.append({
            checked: true,
            pkg: pkg
          })
        });
      } else {
        allChecked = false;
        const sep = Vars.separator;
        const dir = bnr.backupDir + sep + device.serial;

        const apkDir = dir + sep + "apk";
        const apkList = stella.listDirectory(apkDir);
        apkList.forEach(pkg => {
          apkCount++;
          listModel.append({
            checked: false,
            pkg: pkg
          })
        });
      }
    }
  }

  function getCheckedPackages() {
    let device = Vars.currentDevice;
    let model = apkTable.model;
    let result = [];
    apkCount = 0;

    if(backupRb.checked) {
      let packages = Utils.getPackages(device);
     
      for(let p of packages){
        for(let i = 0; i < model.count; i++) {
          let pkg = model.get(i);
          if(pkg.checked && pkg.pkg === p.name) {
            result.push(p);
            apkCount++;
          }
        }  
      }
    } else {
      const sep = Vars.separator;
      const dir = bnr.backupDir + sep + device.serial;

      const apkDir = dir + sep + "apk";
      const apkList = stella.listDirectory(apkDir);
      apkList.forEach(name => {
        for(let i = 0; i < model.count; i++) {
          let pkg = model.get(i);
          if(pkg.checked && pkg.pkg === name) {
            result.push(name);
            apkCount++;
          }
        }
      });
    }
    return result;
  }

  function backupPartitions(partitions) {
    let device = Vars.currentDevice;
    const parts = Utils.listPartitions(device);

    if(parts.length > 0 && partitions.length > 0){
      promise = promise.then(_ => {
        console.info('backing up partitions...');
        stella.mkdir(dir + sep + "partitions");
        return Promise.resolve();
      });

      for(let [name, partition] of parts) {
        promise = promise.then(_=> {
          if(!partitions.includes(name)) {
            return Promise.resolve();
          }

          console.info(`backing up ${name} partition`);
          return Utils.runShell(device.serial, ['su','-c','dd',`if=${partition}`,`of=/sdcard/${name}.img`])
          .then(_ => {
            let dest = dir + sep + "partitions" + sep + name + ".img";
            return Utils.pullFile(device, `/sdcard/${name}.img`, dest).then(_=> {
              console.log("finished pulling partition, removing it from sdcard");
              return Utils.runShell(device.serial, ['rm', `/sdcard/${name}.img`]);
            });
          })
        });
      }
      promise.then(_ => {
        console.info("backup partitions finished!");
        return Promise.resolve();
      });
    }
  }

  function startRestore() {
    const device = Vars.currentDevice;
    const sep = Vars.separator;
    const dir = bnr.backupDir + sep + device.id;

    const apkDir = dir + sep + "apk";
    const apkList = getCheckedPackages();

    let promise = Promise.resolve();
    if(appChk.checked && apkList.length > 0) {
      promise = promise.then(_ => {
        currentProgress = 0;
        maxProgress = apkCount;
        return Promise.resolve();
      });
      
      console.info("restoring applications...")
      let installedApk = Utils.getPackages(device, true, true);

      for(let apk of apkList) {
        let path = apkDir + sep + apk;
        let name = stella.basename(path);

        promise = promise.then(_ => {
          let apkPath = apkDir + sep + apk;
          
          return Promise.resolve().then(_ => {
            progressMessage = `restoring apk ${name}`;
            if(!installedApk.includes(name)){
              console.info("restoring app", name);
              return Utils.installApk(device.serial, apkPath);
            }
            console.info(`${name} already installed`)
            return Utils.forceStopApp(device.serial, name);
          }).then(_ => {
          
            if(dataChk.checked) {
              return Promise.resolve().then(_ => {
                let apkPath = apkDir + sep + apk;
                let packageName = stella.basename(apkPath);
                if(device.isRooted) {
                  console.info(`restoring system data for package ${packageName}`);
                  let dataDir = dir + sep + "system-data" + sep + packageName;
                  let dest = "/data/data/" + packageName;
                  
                  return Utils.pushFile(device, dataDir, dest, true).then(_ => {
                    let ownerGroup = Utils.getOwnerGroup(device, dest);
                    console.info("updating ownership for folder", dest ,"to", ownerGroup);
                    return Utils.chown(device, dest, ownerGroup, true);
                  });
                }
              }).then(_ => {
                let apkPath = apkDir + sep + apk;
                let packageName = stella.basename(apkPath);
                let dataDir = dir + sep + "data" + sep + packageName;
                if(stella.exists(dataDir)){
                  console.info(`restoring data for package ${packageName}`);
                  let dest = "/sdcard/Android/data/" + packageName;
                  return Utils.pushFile(device, dataDir, dest);
                }

                return Promise.resolve()
              }).then(_ => {
                let apkPath = apkDir + sep + apk;
                let packageName = stella.basename(apkPath);
                let obbDir = dir + sep + "obb" + sep + packageName;
                if(stella.exists(obbDir)){
                  console.info(`restoring opaque binary blob files for package ${packageName}`);
                  let dest = "/sdcard/Android/obb/" + packageName;
                  return Utils.pushFile(device, obbDir, dest);
                }

                return Promise.resolve()
              }).then(_ => {
                let apkPath = apkDir + sep + apk;
                let packageName = stella.basename(apkPath);
                let mediaDir = dir + sep + "media" + sep + packageName;
                if(stella.exists(mediaDir)){
                  console.info(`restoring media files for package ${packageName}`);
                  let dest = "/sdcard/Android/media/" + packageName;
                  return Utils.pushFile(device, mediaDir, dest);
                }
                currentProgress++;
                return Promise.resolve();
              });
            }
            currentProgress++;
            return Promise.resolve();
          });
        });
      }

      promise = promise.then(_ => {
        console.info('restore applications finished');
        return Promise.resolve();
      });
    }

    if(contactsChk.checked && device.isRooted) {
      let dest = "/data/data/com.android.providers.contacts/databases";
      let contactsDir = dir + sep + "contacts" + sep + "databases";
      
      promise = promise.then(_ => {
        console.info("restoring contacts...");
        currentProgress = 0;
        maxProgress = 2;  
        return Utils.forceStopApp(device.serial, "com.android.providers.contacts");
      }).then(_ => {
        currentProgress++;
        return Utils.pushFile(device, contactsDir, dest, true).then(_ => {
          let ownerGroup = Utils.getOwnerGroup(device, dest);
          console.info("updating ownership for folder", dest);
          return Utils.chown(device, dest, ownerGroup, true);
        }).then(_ => {
          currentProgress++;
          console.info("restore contacts finished!")
          return Promise.resolve();
        })
      });
      
    } else if(contactsChk.checked && !device.isRooted){
      console.info("cannot restore contacts without root permission");
    }

    if(wifiChk.checked && device.isRooted) {
      
      let wifiDir = dir + sep + "misc" + sep + "wifi";
      promise = promise.then(_ => {
        console.info("restoring wifi configuration and saved network...");
        currentProgress = 0;
        maxProgress = 2;
        return Promise.resolve();
      }).then(_ => {
        let wifiConfig = wifiDir + sep + "softap.conf";
        let dest = "/data/misc/wifi/softap.conf";

        console.info("restoring wifi configuration...");
        return Utils.pushFile(device, wifiConfig, dest, true).then(_ => {
          let ownerGroup = Utils.getOwnerGroup(device, dest);
          console.info("updating ownership for file", dest);
          return Utils.chown(device, dest, ownerGroup, true);
        }).then(_ => {
          currentProgress++;
          if(device.version >= Vars.VERSION.LOLLIPOP && device.version < Vars.VERSION.PIE){
            wifiConfig = wifiDir + sep + "wpa_supplicant.conf";
            dest = "/data/misc/wifi/wpa_supplicant.conf";
          } else if(device.version === Vars.VERSION.PIE) {
            wifiConfig = dir + sep + "misc" + sep + "wifi" + sep + "WifiConfigStore.xml";
            dest = "/data/misc/wifi/WifiConfigStore.xml";
          }
          console.info("restoring wifi saved network...");
          return Utils.pushFile(device, wifiConfig, dest, true).then(_ => {
            let ownerGroup = Utils.getOwnerGroup(device, dest);
            console.info("updating ownership for file", dest);
            return Utils.chown(device, dest, ownerGroup, true);
          }).then(_=>{
            currentProgress++;
            return Promise.resolve();
          });
        })
      });
      
    } else if(contactsChk.checked && !device.isRooted){
      console.info("cannot restore contacts without root permission");
    }

     if(messagesChk.checked && device.isRooted) {
      promise = promise.then(_ => {
        console.info("restoring messages...");
        currentProgress = 0;
        maxProgress = 1;
        return Promise.resolve();
      }).then(_ => {
        let dest = "/data/data/com.android.providers.telephony/databases";
        let messagesDir = dir + sep + "messages" + sep + "databases";

        return Utils.pushFile(device, messagesDir, dest).then(_ => {
          let ownerGroup = Utils.getOwnerGroup(device, dest);
          console.info("updating ownership for folder", dest);
          return Utils.chown(device, dest, ownerGroup, true);
        });
      }).then(_ => {
        console.info("restore messages finished!");
        currentProgress++;
        return Promise.resolve();
      });
    } else if(messagesChk.checked && !device.isRooted) {
      console.info("cannot restore messages without root permission");
    }

    promise = promise.then(_ => {
      progressMessage = `restore finished!`;
      restoreIsRunning = false;
      updateButtonState();
    })
  }

  function startBackup() {
    let device = Vars.currentDevice;
    
    let promise = Promise.resolve();
    let sep = Vars.separator;
    let dir = bnr.backupDir + sep + device.id;

    let packages = getCheckedPackages();
    
    if(appChk.checked && packages.length > 0){
      
      let apkDir = dir + sep + "apk";
      promise = promise.then(_ => {
        currentProgress = 0;
        maxProgress = apkCount;
        progressMessage = `backing up applications...`;
        console.info(progressMessage);
        stella.mkdir(apkDir);
        return Promise.resolve();
      });
      

      for (let p of packages) {
        promise = promise.then(_ => {
          
          let dest = `${apkDir + sep + p.name}.apk`;
          console.info("backing up apk ", p.name);
          progressMessage = `backing up apk ${p.name}`;
          return Utils.pullFile(device, p.path, dest).then(_ => {
            if(dataChk.checked){
              return Promise.resolve().then(_ => {
                let dataDir = "/sdcard/Android/data/" + p.name;
                let dest = dir + sep + "data" + sep + p.name;
                stella.mkdir(dir + sep + "data");
                stella.rmdir(dest);
                if(Utils.fileExists(device, dataDir)){
                  console.info(`backing up user data for package ${p.name}`);
                  return Utils.pullFile(device, dataDir, dest);
                }
                return Promise.resolve();
              }).then(_ => {
                if(device.isRooted) {
                  let dataDir = "/data/data/" + p.name;
                  let dest = dir + sep + "system-data" + sep + p.name;
                  stella.mkdir(dir + sep + "system-data");
                  stella.rmdir(dest);
                  console.info(`backing up system data for package ${p.name}`);
                  return Utils.pullFile(device, dataDir, dest, true);
                }

                return Promsie.resolve();
              }).then(_ => {
                let mediaDir = "/sdcard/Android/media/"+p.name;
                let dest = dir + sep + "media" + sep + p.name;
                
                stella.mkdir(dir + sep + "media");
                stella.rmdir(dest);
                
                if(Utils.fileExists(device, mediaDir)) {  
                  console.info(`backing up media files for package ${p.name}`);
                  return Utils.pullFile(device, mediaDir, dest);
                }
                return Promise.resolve();
              }).then(_ => {
                let obbDir = "/sdcard/Android/obb/"+p.name;
                let dest = dir + sep + "obb" + sep + p.name;
                
                stella.mkdir(dir + sep + "obb");
                stella.rmdir(dest);

                if(Utils.fileExists(device, obbDir)) {  
                  console.info(`backing up opaque binary blob files for package ${p.name}`);
                  return Utils.pullFile(device, obbDir, dest);
                }
                return Promise.resolve();
              }).then(_ => {
                
                currentProgress++;
                return Promise.resolve();
              })
            }
            currentProgress++;
            return Promise.resolve();
          });
        });
      }
      
      promise = promise.then(_ => {
        progressMessage = `backup finished!`;
        console.info('backup applications finished');
        return Promise.resolve();
      });
    }

    if(contactsChk.checked && device.isRooted) {
      let contactsDir = "/data/data/com.android.providers.contacts/databases";
      let dest = `${dir + sep + "contacts"}`;
      promise = promise.then(_ => {
        currentProgress = 0;
        maxProgress = 1;
        progressMessage = "backing up contacts...";
        console.info(progressMessage);
        // stella.rmdir(dest);
        stella.mkdir(dir + sep + "contacts");
        
        return Promise.resolve();
      }).then(_ => {
        return Utils.pullFile(device, contactsDir, dest, true).then(_ => {
          console.info("backup contacts finished");
          currentProgress = 1;
          return Promise.resolve();
        })
      });

    } else if(contactsChk.checked && !device.isRooted) {
      console.info("cannot backup contacts without root permission");
    }

    if(wifiChk.checked && device.isRooted) {
      
      promise = promise.then(_ => {
        stella.mkdir(dir + sep + "misc");
        stella.mkdir(dir + sep + "misc" + sep + "wifi");
        currentProgress = 0;
        maxProgress = 2;  
        console.info("backing up wifi configuration and saved network...");
        return Promise.resolve();
      }).then(_ => {
        let wifiConfig = '/data/misc/wifi/softap.conf';
        let dest = dir + sep + "misc" + sep + "wifi" + sep + "softap.conf";
        console.info("backing up wifi configuration");
        return Utils.pullFile(device, wifiConfig, dest, true);
      }).then(_ => {
        currentProgress++;
        let wifiStoreFile, dest;

        if(device.version >= Vars.VERSION.LOLLIPOP && device.version < Vars.VERSION.PIE) {
          wifiStoreFile = "/data/misc/wifi/wpa_supplicant.conf";
          dest = dir + sep + "misc" + sep + "wifi" + sep + "wpa_supplicant.conf";  
        } else if(device.version === Vars.VERSION.PIE) {
          wifiStoreFile = "/data/misc/wifi/WifiConfigStore.xml";
          dest = dir + sep + "misc" + sep + "wifi" + sep + "WifiConfigStore.xml";
        }

        console.info("backing up saved wifi network");
        return Utils.pullFile(device, wifiStoreFile, dest, true);
      }).then(_ => {
        currentProgress++;
        console.info("backup wifi configuration and saved network finished!");
        return Promise.resolve();
      });
    } else if(wifiChk.checked && !device.isRooted) {
      console.info("cannot backup wifi configuration without root permission");
    }

    if(messagesChk.checked && device.isRooted) {
     
      promise = promise.then(_ => {
        currentProgress = 0;
        maxProgress = 1;
        stella.mkdir(dir + sep + "messages");
        progressMessage = "backing up messages...";
        console.info(progressMessage);
        return Promise.resolve();
      }).then(_ => {
        let messagesDir = "/data/data/com.android.providers.telephony/databases";
        let dest = dest = dir + sep + "messages";

        return Utils.pullFile(device, messagesDir, dest);
      }).then(_ => {
        currentProgress++;
        console.info("backup messages finished!");
        return Promise.resolve();
      });
    } else if(messagesChk.checked && !device.isRooted) {
      console.info("cannot backup messages without root permission");
    }

    promise.then(_ => {
      backupIsRunning = false;
      updateButtonState();
    })
  }
  GroupBox {
    id: mode
    contentHeight: 45
    anchors.right: parent.right
    anchors.rightMargin: 10
    title: ""
    anchors.top: parent.top
    anchors.topMargin: 10
    anchors.left: parent.left
    anchors.leftMargin: 10
    RadioButton {
      id: backupRb
      text: qsTr("Backup")
      anchors.left: parent.left
      anchors.leftMargin: 10
      anchors.top: parent.top
      anchors.topMargin: 8
      checked: true
      onCheckedChanged: {
        refreshPackage();
        if (backupRb.checked) {
          backupBtn.visible = true;
          restoreBtn.visible = false;
        } else {
          backupBtn.visible = false;
          restoreBtn.visible = true;
        }
      }
    }

    RadioButton {
      id: restoreRb
      text: qsTr("Restore")
      anchors.left: backupRb.right
      anchors.leftMargin: 10
      anchors.top: parent.top
      anchors.topMargin: 8
      onCheckedChanged: {
        // if (fastbootRb.checked) {
        //   keepDataChk.visible = true
        //   lockCrcChk.visible = true
        // } else {
        //   keepDataChk.visible = false
        //   lockCrcChk.visible = false
        // }
      }
    }
  }
  CheckBox {
    id: dataChk
    anchors.top: mode.bottom
    anchors.topMargin: 10
    anchors.left: parent.left
    anchors.leftMargin: 10
    text: qsTr("Data")
  }

  CheckBox {
    id: appChk
    anchors.top: mode.bottom
    anchors.topMargin: 10
    anchors.left: dataChk.right
    anchors.leftMargin: 10
    text: qsTr("Application")
    onCheckedChanged: {
      if(appChk.checked){
        apkTable.visible = true;
      } else {
        apkTable.visible = false;
      }
    }
  }

  CheckBox {
    id: messagesChk
    anchors.top: mode.bottom
    anchors.topMargin: 10
    anchors.left: appChk.right
    anchors.leftMargin: 10
    text: qsTr("Messages")
  }

  CheckBox {
    id: contactsChk
    anchors.top: dataChk.bottom
    anchors.topMargin: 10
    anchors.left: parent.left
    anchors.leftMargin: 10
    text: qsTr("Contacts")
  }

  CheckBox {
    id: wifiChk
    anchors.top: appChk.bottom
    anchors.topMargin: 10
    anchors.left: contactsChk.right
    anchors.leftMargin: 10
    text: qsTr("Wifi")
  }

  Component {
    id: tableHeaderDelegate
    Rectangle {
      height: 20
      width: 40
      anchors.leftMargin: (styleData.column === 0) ? 0 : 10
      QQC1.CheckBox {
        id: allChk
        anchors.centerIn: parent
        property bool pressed: styleData.pressed
        // property bool isChecked: allChecked
        width: styleData.width
        checked: allChecked
        visible: styleData.column === 0 // Show only in the 4th column
        activeFocusOnPress: true
        text: ""
        onPressedChanged: {
          if(pressed && styleData.column === 0 ) {
            checked = !checked;

            for(let i = 0; i < listModel.count; i++) {
              listModel.setProperty(i, "checked", checked);
            }
          }
        }
      }
      Text {
        anchors.leftMargin: 10
        text: styleData.value
        width: (styleData.column === 0) ? 0 : styleData.width
        visible: styleData.column > 0
        font.bold: true
      }
    }
  }

  
  QQC1.TableView {
    id: apkTable
    anchors.right: parent.right
    anchors.rightMargin: 10
    anchors.top: wifiChk.bottom
    anchors.topMargin: 10
    anchors.left: parent.left
    anchors.leftMargin: 10
    anchors.bottom: progress.top
    anchors.bottomMargin: 10
    visible: false
    headerDelegate: tableHeaderDelegate
    QQC1.TableViewColumn {
      role: "checked"
      title: ""
      width: 40
      resizable: false
      delegate: Rectangle {
        anchors.fill: parent
        color: (styleData.row % 2 === 0) ? "#acacac" : "#fff"
        QQC1.CheckBox {
          anchors.centerIn: parent
          checked: (model) ? model.checked : false// read from the model when created or recycled
          onCheckedChanged: { 
            if(model)
              model.checked = checked
          }
        }
      }
    }
    QQC1.TableViewColumn {
      role: "pkg"
      title: "Package"
      width: apkTable.width - 40
      delegate: Rectangle {
        color: (styleData.row % 2 === 0) ? "#acacac" : "#fff"
        // radius: 4
        Text {
          anchors.leftMargin: 10
          text: (model) ? model.pkg : ""
          color: "#55557f"
          // width: (styleData.column === 0) ? 0 : styleData.width
          // visible: styleData.column > 0
          // font.bold: true
        }
      }
    }
    model: ListModel {
      id: listModel
    }
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
    anchors.bottom: restoreBtn.top
    anchors.bottomMargin: 5
    anchors.topMargin: 5
    style: QQCS1.ProgressBarStyle {
      background: Rectangle {
        radius: 4
        color: "#3C3F41"
      }
    }
    Text {
      text: progressMessage
      anchors.centerIn: parent
      color: "#ffffff"
    }
  }

  Button {
    id: restoreBtn
    text: qsTr("Start Restore")
    anchors.left: parent.left
    anchors.leftMargin: 10
    anchors.bottom: parent.bottom
    anchors.bottomMargin: 10
    enabled: false
    visible: false
    onClicked: {
      startRestore();
    }
  }

  Button {
    id: backupBtn
    text: qsTr("Start Backup")
    enabled: false
    anchors.left: parent.left
    anchors.leftMargin: 10
    anchors.bottom: parent.bottom
    anchors.bottomMargin: 10
    onClicked: {
      backupIsRunning = true;
      updateButtonState();
      let device = Vars.currentDevice;
      // devicesDialog.open();

      if(!device.isRooted && !device.isRootUser && dataChk.checked && length(Vars.devices) > 1) {
        dialog.icon = StandardIcon.Warning;
        dialog.text = "Your device is not rooted, some data may not be able to be restored.";
        function onAccepted() {
          console.log("accepted")
          startBackup();
          dialog.onAccepted.disconnect(onAccepted)
        }
        dialog.onAccepted.connect(onAccepted)
        dialog.open();
      } else {
        startBackup();
      }
      
      // if(device && device.isRooted){
      //   startBackup(partitions);
      // } else if(device && !device.isRooted) {
      //   if(device.state === 'device') {
      //     Utils.reboot(device.serial, 'bootloader').then(_ => {

      //     });
      //   }
      // }
    }
  }



  Component.onCompleted: {
    if(!Vars.initialized) {
      Vars.events.once('app-ready',() => {
        updateButtonState();
        refreshPackage();
      });
    } else {
      updateButtonState();
      refreshPackage();
    }

    Vars.events.on('device-added', () => {
      updateButtonState();
      refreshPackage();
    });
    Vars.events.on('device-removed', () => {
      updateButtonState();
    });
    
  }
}
