#!/usr/bin/env python

import sys
import signal
import struct
import threading
from os import path, makedirs

from PyQt5.QtCore import (QObject, QThread, QCoreApplication, QTimer, QSettings, pyqtSlot, 
                          Q_CLASSINFO, QByteArray, QMutex, QWaitCondition)
from PyQt5.QtDBus import QDBusConnection, QDBusMessage, QDBusAbstractAdaptor
from PyQt5.QtGui import QIcon, QPixmap
from PyQt5.QtWidgets import QSystemTrayIcon, QMenu, QApplication

from message import (SyncMessage, MessageList, NetworkList, ContactList,
                    SYNC_MSG_TYPE_CONTACT, SYNC_MSG_TYPE_MESSAGE, SYNC_MSG_TYPE_NETWORK,
                    SYNC_MSG_TYPE_FILE, MIN_MSG_FILE_LENGTH, MIN_MSG_FILE_DATA_LENGTH, 
                    MIN_MSG_FILE_APK_LENGTH,
                    SYNC_MSG_TYPE_REQUEST_RESTORE, SYNC_MSG_TYPE_REQUEST_BACKUP,
                    FILE_TYPE_APK, FILE_TYPE_DATA, FILE_TYPE_SYSTEM_DATA,
                    FILE_TYPE_OBB, FILE_TYPE_MEDIA)
from stella.network import NetworkManager
from stella.manager import DeviceManager
from stella.config import Configuration

SERVICE_NAME = "io.github.stella.Bnr"

class DeviceHandler(QThread):
  _messages = []
  _threads = []
  _buffer = bytearray(b"")
  _device = None
  _dataLen = 0
  _shouldTerminate = False
  _condition = QWaitCondition()
  _mutex = QMutex()
  messageList = None
  contactList = None
  networkList = None
  serial = None
  isRooted = None
  def __init__(self, device, parent=None):
    QObject.__init__(self, parent=parent)
    self._device = device
    device.onNewMessage.connect(self.onNewMessage)

    self.settings = Configuration("Stella")

    defaultDir = path.expanduser("~/Documents/Stella/backups")
    self._backupDir = self.settings.get("backupDir", defaultDir)

    if device.isRooted() is None:
      buffer = struct.pack(f"!b", 6)
      device.send(buffer)

    self.messageList = MessageList(device.serial())
    self.contactList = ContactList(device.serial())
    self.networkList = NetworkList(device.serial())

  def stop(self):
    self._device.onNewMessage.disconnect(self.onNewMessage)
    self._shouldTerminate = True
    self._condition.wakeAll()
    
    while(self.isRunning()):
      time.sleep(0.1)

  def handleDeviceMessage(self, type, jsonStr):
    msg = SyncMessage(type)
    msg.deserialize(jsonStr)

    if msg.type == SYNC_MSG_TYPE_CONTACT:
      print(f"adding new contact {msg.contact}")
      self.contactList.add(msg.contact)
    elif msg.type == SYNC_MSG_TYPE_MESSAGE:
      print(f"adding new message {msg.message}")
      self.messageList.add(msg.message)
    elif msg.type == SYNC_MSG_TYPE_NETWORK:
      self.networkList.add(msg.network)
    elif msg.type == SYNC_MSG_TYPE_REQUEST_RESTORE:
      # self.prepareRestore()
      ""

  @pyqtSlot(QByteArray)
  def onNewMessage(self, chunk):
    # instead of treating this message as data, we should treat it as a chunk of full data
    # but there is a possibility that the data we get is a full data or just a slice 
    # of it combined with another different data.
    # this is probably a chunk of data, so we need to collect it first in instance variable
    self._buffer += chunk.data()

    while len(self._buffer) > 0:
      type, = struct.unpack_from("!b", self._buffer[0:1], 0) # get the data type
      print(f"data type {type}")
      if type == SYNC_MSG_TYPE_CONTACT or type == SYNC_MSG_TYPE_MESSAGE:
        dataLen, = struct.unpack_from("!i", self._buffer[1:5], 0) # get the data length, we need 4 bytes for integer
        # save it for now cuz there is a probability that current buffer is not enough to get all the data
        
        # check if buffer is enought to get the whole data, if not then
        # just exit the loop and wait for another data
        if len(self._buffer) - 5 >= dataLen:
          jsonStr, = struct.unpack_from(f"!{dataLen}s", self._buffer[5:dataLen + 5], 0)
          assert len(jsonStr) == dataLen

          self.handleDeviceMessage(type, jsonStr)

          # we already get all the data we need, now remove it from the buffer
          del self._buffer[0:1] # remove data type
          del self._buffer[0:4] # remove data length
          del self._buffer[0:dataLen] # remove data
        else:
          break
      elif type == SYNC_MSG_TYPE_FILE and len(self._buffer) > MIN_MSG_FILE_LENGTH:
        fileType, = struct.unpack_from("!b", self._buffer[1:2], 0) # get the file type
        # print(f"file type {fileType}")
        if fileType == FILE_TYPE_APK and len(self._buffer) > MIN_MSG_FILE_APK_LENGTH:
        
          nameLen, = struct.unpack_from("!i", self._buffer[2:6], 0) # get the name length, we need 4 bytes for integer
          # print(f"name length {nameLen}")
          dataLen, = struct.unpack_from("!i", self._buffer[6:10], 0) # get the data length, we need 4 bytes for integer
          print(f"data length {dataLen}")

          if len(self._buffer) >= (dataLen + MIN_MSG_FILE_LENGTH + nameLen):
            name, = struct.unpack_from(f"!{nameLen}s", self._buffer[MIN_MSG_FILE_LENGTH:nameLen + MIN_MSG_FILE_LENGTH],0)
            name = name.decode("ascii")
            buff = self._buffer[MIN_MSG_FILE_LENGTH+nameLen:dataLen+MIN_MSG_FILE_LENGTH+nameLen]
            
            assert len(name) == nameLen
            assert len(buff) == dataLen
            
            destPath = ""
            if fileType == FILE_TYPE_APK:
              destPath = path.join(self._backupDir, self._device.serial(), "apk", name)
            elif filetype == FILE_TYPE_DATA:
              destPath = path.join(self._backupDir, self._device.serial(), "data", name)
            elif filetype == FILE_TYPE_SYSTEM_DATA:
              destPath = path.join(self._backupDir, self._device.serial(), "system-data", name)

            if not path.exists(path.dirname(destPath)):
              makedirs(path.dirname(destPath))

            with open(destPath, "wb") as f:
              print(f"writing file with size {dataLen}")
              f.write(buff)
              # we already get all the data we need, now remove it from the buffer
              del self._buffer[0:1] # remove data type
              del self._buffer[0:1] # remove file type
              del self._buffer[0:4] # remove data length
              del self._buffer[0:4] # remove name length
              del self._buffer[0:nameLen] # remove name
              del self._buffer[0:dataLen] # remove data
          else:
            break
        elif (fileType == FILE_TYPE_DATA or fileType == FILE_TYPE_SYSTEM_DATA) and len(self._buffer) > MIN_MSG_FILE_DATA_LENGTH:
          nameLen, = struct.unpack_from("!i", self._buffer[2:6], 0) # get the name length, we need 4 bytes for integer
          subdirectoriesLen, = struct.unpack_from("!i", self._buffer[6:10], 0)
          # print(f"name length {nameLen}")
          dataLen, = struct.unpack_from("!i", self._buffer[10:14], 0) # get the data length, we need 4 bytes for integer

          if len(self._buffer) >= (MIN_MSG_FILE_DATA_LENGTH + nameLen + subdirectoriesLen + dataLen):
            name, = struct.unpack_from(f"!{nameLen}s", self._buffer[MIN_MSG_FILE_DATA_LENGTH:nameLen + MIN_MSG_FILE_DATA_LENGTH],0)
            name = name.decode("ascii")
            subdirectories, = struct.unpack_from(f"!{subdirectoriesLen}s", 
              self._buffer[MIN_MSG_FILE_DATA_LENGTH+nameLen:subdirectoriesLen + nameLen + MIN_MSG_FILE_DATA_LENGTH], 
              0
            )
            # print("subdirectories")
            buff = self._buffer[MIN_MSG_FILE_DATA_LENGTH+nameLen+subdirectoriesLen:dataLen+MIN_MSG_FILE_DATA_LENGTH+nameLen+subdirectoriesLen]

            subdirectories = str(subdirectories.decode("ascii"))
            if subdirectories.startswith("/"):
              subdirectories = subdirectories[1:]
            
            sub = subdirectories.split("/")
            if fileType == FILE_TYPE_DATA:
              destPath = path.join(self._backupDir, self._device.serial(), "data", *sub, name)
            elif fileType == FILE_TYPE_SYSTEM_DATA:
              destPath = path.join(self._backupDir, self._device.serial(), "system-data", *sub, name)
            elif fileType == FILE_TYPE_OBB:
              destPath = path.join(self._backupDir, self._device.serial(), "obb", *sub, name)
            elif fileType == FILE_TYPE_MEDIA:
              destPath = path.join(self._backupDir, self._device.serial(), "media", *sub, name)

            if not path.exists(path.dirname(destPath)):
              makedirs(path.dirname(destPath))

            print(f"destination path {destPath}")
            with open(destPath, "wb") as f:
              print(f"writing file with size {dataLen}")
              f.write(buff)
              # we already get all the data we need, now remove it from the buffer
              del self._buffer[0:1] # remove data type
              del self._buffer[0:1] # remove file type
              del self._buffer[0:4] # remove name length
              del self._buffer[0:4] # remove subdirectories length
              del self._buffer[0:4] # remove data length
              del self._buffer[0:nameLen] # remove name
              del self._buffer[0:subdirectoriesLen] # remove subdirectories
              del self._buffer[0:dataLen] # remove data
          else:
            break
        else:
          break
      else:
        print(f"unknown data type {type} {self._buffer}")
        break

  def run(self):
    while True:
      while len(self._messages) > 0:
        self._mutex.lock()
        msg = self._messages.pop(0)
        self._mutex.unlock()
        self._device.send(msg.serialize())

      
      self._mutex.lock()
      if not self._shouldRestart:
        self._condition.wait(self._mutex)
      self._shouldRestart = False
      self._mutex.unlock()

      if self._shouldTerminate:
        break


class Bnrd(QObject):
  _manager = None
  _handlers = {}

  def __init__(self, parent=None):
    QObject.__init__(self, parent=parent)
    self._manager = DeviceManager(self)
    self._manager.onDeviceAdded.connect(self.onDeviceAdded)
    self._manager.onDeviceRemoved.connect(self.onDeviceRemoved)

  def onDeviceAdded(self, device):
    # print("new device added ")
    self._handlers[device._path] = DeviceHandler(device, self)
    # self._handlers.append(DeviceHandler(device, self))
  
  def onDeviceRemoved(self, path):
    handler = self._handlers[path]
    handler.stop()
    del handler
    del self._handlers[path]

  def stop(self):
    self._manager.onDeviceAdded.disconnect(self.onDeviceAdded)
    self._manager.onDeviceRemoved.disconnect(self.onDeviceRemoved)

    for key in self._handlers.keys():
      handler = self._handlers[key]
      handler.stop()
      del handler
    

def sigint_handler(*args):
    """Handler for the SIGINT signal."""
    # if QMessageBox.question(None, '', "Are you sure you want to quit?",
    #                         QMessageBox.Yes | QMessageBox.No,
    #                         QMessageBox.No) == QMessageBox.Yes:
    QCoreApplication.quit()


iconBase64 = b"PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSI2NCIgaGVpZ2h0PSI2NCIgdmVyc2lvbj0iMS4xIj4KICA8cmVjdCBzdHlsZT0ib3BhY2l0eTouMiIgd2lkdGg9IjUwIiBoZWlnaHQ9IjMyIiB4PSI3IiB5PSIyNCIgcng9IjIuNSIgcnk9IjIuNSIgLz4KICA8cGF0aCBzdHlsZT0iZmlsbDojNDg3N2IxIgogICAgZD0ibTcgNDQuNWMwIDEuMzg1IDEuMTE1IDIuNSAyLjUgMi41aDQ1YzEuMzg1IDAgMi41LTEuMTE1IDIuNS0yLjV2LTI3YzAtMS4zODUtMS4xMTUtMi41LTIuNS0yLjVoLTI1LjV2LTIuNWMwLTEuMzg1LTEuMTE1LTIuNS0yLjUtMi41aC0xN2MtMS4zODUgMC0yLjUgMS4xMTUtMi41IDIuNSIgLz4KICA8cmVjdCBzdHlsZT0ib3BhY2l0eTouMiIgd2lkdGg9IjUwIiBoZWlnaHQ9IjMyIiB4PSI3IiB5PSIyMiIgcng9IjIuNSIgcnk9IjIuNSIgLz4KICA8cmVjdCBzdHlsZT0iZmlsbDojZTRlNGU0IiB3aWR0aD0iNDQiIGhlaWdodD0iMjAiIHg9IjEwIiB5PSIxOCIgcng9IjIuNSIgcnk9IjIuNSIgLz4KICA8cmVjdCBzdHlsZT0iZmlsbDojNTI5NGUyIiB3aWR0aD0iNTAiIGhlaWdodD0iMzIiIHg9IjciIHk9IjIzIiByeD0iMi41IiByeT0iMi41IiAvPgogIDxwYXRoIHN0eWxlPSJvcGFjaXR5Oi4xO2ZpbGw6I2ZmZmZmZiIKICAgIGQ9Im05LjUgMTBjLTEuMzg1IDAtMi41IDEuMTE1LTIuNSAyLjV2MWMwLTEuMzg1IDEuMTE1LTIuNSAyLjUtMi41aDE3YzEuMzg1IDAgMi41IDEuMTE1IDIuNSAyLjV2LTFjMC0xLjM4NS0xLjExNS0yLjUtMi41LTIuNXptMTkuNSA1djFoMjUuNWMxLjM5IDAgMi41IDEuMTE1IDIuNSAyLjV2LTFjMC0xLjM4NS0xLjExLTIuNS0yLjUtMi41eiIgLz4KICA8cGF0aCBzdHlsZT0iZmlsbDojMWQzNDRmIgogICAgZD0ibTMyLjQxNDA2MiAyOS4wMDc4MTJjLTAuODQ3Nzk2LTAuMDM1ODctMS43MTA4OTQgMC4wMzY4NC0yLjU3MDMxMiAwLjIyNjU2My00LjU3NzE2MSAxLjAxMDQ2NC03Ljg0Mzk0OCA1LjA3OTk4Mi03Ljg0Mzc1IDkuNzY1NjI1aC0ybDMgMyAzLTNoLTJjLTAuMDAwMjE1LTMuNzU4Nzc0IDIuNjAzNTgyLTcuMDAxOTA0IDYuMjc1MzkxLTcuODEyNSAzLjY3NjkzNC0wLjgxMTcyOCA3LjQwOTggMS4wNDAzMzggOC45ODgyODEgNC40NTg5ODQgMS41Nzg0ODEgMy40MTg2NDcgMC41NjY5NzYgNy40NjAwNjYtMi40MzU1NDcgOS43MzI0MjItMy4wMDI1MjMgMi4yNzIzNTctNy4xNjc5NzggMi4xNDY5OTQtMTAuMDI5Mjk3LTAuMzAwNzgxLTEuMDE0MzIyLTAuODY1ODg1LTIuMzEzMTUgMC42NTU1OTktMS4yOTg4MjggMS41MjE0ODQgMy41NjY4MzYgMy4wNTEzMjQgOC43OTIzIDMuMjA3NjUzIDEyLjUzNTE1NiAwLjM3NSAzLjc0Mjg1Ny0yLjgzMjY1MiA1LjAxMDY1Ny03LjkwNDQzMSAzLjA0Mjk2OS0xMi4xNjYwMTUtMS40NzU3NjYtMy4xOTYxODktNC40NzkxMzItNS4zMDIyNDUtNy44MjIyNjYtNS43Mjg1MTYtMC4yNzg1OTQtMC4wMzU1Mi0wLjU1OTE5Ny0wLjA2MDMtMC44NDE3OTctMC4wNzIyN3ptLTAuNDI5Njg3IDMuOTc4NTE2YTEuMDAwMSAxLjAwMDEgMCAwIDAgLTAuOTg0Mzc1IDEuMDEzNjcydjQuODYzMjgxYTEuMDAwMSAxLjAwMDEgMCAwIDAgMC4wNDEwMiAwLjQzMzU5NCAxLjAwMDEgMS4wMDAxIDAgMCAwIDAuMDI3MzQgMC4wNzQyMiAxLjAwMDEgMS4wMDAxIDAgMCAwIDAuMjA4OTg1IDAuMzMyMDMxbDIuOTIxODc1IDMuODk2NDg0YTEuMDAwMzkwNSAxLjAwMDM5MDUgMCAwIDAgMS42MDE1NjIgLTEuMTk5MjE4bC0yLjgwMDc4Mi0zLjczNDM3NnYtNC42NjYwMTZhMS4wMDAxIDEuMDAwMSAwIDAgMCAtMS4wMTU2MjUgLTEuMDEzNjcyeiIgLz4KPC9zdmc+Cg=="
if __name__ == "__main__":
  print("starting bnrd")
  
  app = QApplication(sys.argv)
  app.setOrganizationName("Stella");
  app.setOrganizationDomain("io.github.stella");
  app.setApplicationName("Stella");

  bus = QDBusConnection.sessionBus()
  if not bus.isConnected():
    print(f"Cannot connect to the D-Bus session bus.")
  
  if not bus.registerService(SERVICE_NAME): 
    print(bus.lastError().message())

  bnrd = Bnrd()
  # bnrd.changeBackupDirectory("/")
  QDBusConnection.sessionBus().registerObject("/", bnrd)
  
  signal.signal(signal.SIGINT, sigint_handler)
  
  # Python cannot handle signals while the Qt event loop is running.
  # so we need to use QTimer to let the interpreter run from time to time.
  # https://stackoverflow.com/questions/4938723/what-is-the-correct-way-to-make-my-pyqt-application-quit-when-killed-from-the-co
  timer = QTimer()
  timer.start(500)  # You may change this if you wish.
  timer.timeout.connect(lambda: None)  # Let the interpreter run each 500 ms.
  
  pixmap = QPixmap()
  pixmap.loadFromData(QByteArray.fromBase64(iconBase64))
  icon = QIcon(pixmap)

  trayIcon = QSystemTrayIcon(app);

  trayIcon.setIcon(icon)
  trayIcon.show()
  def appQuitted():
    bnrd.stop()
    
  app.aboutToQuit.connect(appQuitted)
  app.exec()
