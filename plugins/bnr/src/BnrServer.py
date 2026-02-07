#!/usr/bin/env python
# this file is an experiment to use a TCP socket to communicate with Android client
# so Android client can send a backup to this server
import time
import json
import struct
import threading
from multiprocessing import Pool
from os import path, makedirs
from PyQt5.QtCore import (QObject, QSettings, QThread, QCoreApplication, pyqtSlot, QMutex, QWaitCondition)
from PyQt5.QtNetwork import QTcpServer, QTcpSocket, QHostAddress,QNetworkInterface, QHostAddress

SYNC_MSG_MAX_LENGTH = 1024

SYNC_MSG_TYPE_CONTACT = 1
SYNC_MSG_TYPE_MESSAGE = 2
SYNC_MSG_TYPE_NETWORK = 3
SYNC_MSG_TYPE_ADB_ENABLED = 4
SYNC_MSG_TYPE_SUPERUSER = 5

SYNC_MSG_TYPE_REQUEST_BACKUP = 9
SYNC_MSG_TYPE_REQUEST_RESTORE = 10

MESSAGE_TYPE_SENT = 0
MESSAGE_TYPE_INBOX = 1
MESSAGE_TYPE_DRAFT = 2

class ContactList:
  contacts = []
  def __init__(self):
    self.settings = QSettings("Stella", "Stella")
    
    defaultDir = path.expanduser("~/Documents/Stella/backups")
    self.backupDir = self.settings.value("Bnr/backupDir", defaultDir)

  def add(self, contact):
    if contact is None:
      return

    self.contacts.append(contact)
    self.save()

  def remove(self, contact):
    if contact is None:
      return

    self.contacts.remove(contact)
    self.save()

  def update(self, contact):
    if contact is None:
      return

    self.save()

  def save(self):
    contacts = [contact.__dict__ for contact in self.contacts]
    jsonStr = json.dumps(contacts, indent=2, sort_keys=True)
    p = path.join(self.backupDir, "contacts", "contact.json")
    if not path.exists(path.dirname(p)):
      makedirs(path.dirname(p))

    with open(p, "w+") as f:
      f.write(jsonStr)

class Contact:
  id = None
  name = None
  number = None
  def __init__(self, id, name, number):
    self.name = name
    self.number = number

class MessageList:
  messages = []
  def __init__(self):
    self.settings = QSettings("Stella", "Stella")
    
    defaultDir = path.expanduser("~/Documents/Stella/backups")
    self.backupDir = self.settings.value("Bnr/backupDir", defaultDir)

  def add(self, message):
    if message is None:
      return

    self.messages.append(message)
    self.save()

  def remove(self, message):
    if message is None:
      return

    self.messages.remove(message)
    self.save()

  def update(self, message):
    if message is None:
      return
      
    self.save()

  def save(self):
    messages = [message.__dict__ for message in self.messages]
    jsonStr = json.dumps(messages, indent=2, sort_keys=True)
    p = path.join(self.backupDir, "messages", "message.json")
    if not path.exists(path.dirname(p)):
      makedirs(path.dirname(p))

    with open(p, "w+") as f:
      f.write(jsonStr)

class Message:
  id = None
  threadId = None
  address = None
  body = None
  date = None
  dateSent = None
  type = None
  read = False
  def __init__(self, id, threadId, type, address, body, date, dateSent, read):
    self.id = id
    self.threadId = threadId
    self.type = type
    self.address = address
    self.body = body
    self.date = date
    self.dateSent = dateSent
    self.read = read

class NetworkList:
  networks = []
  def __init__(self):
    self.settings = QSettings("Stella", "Stella")
    
    defaultDir = path.expanduser("~/Documents/Stella/backups")
    self.backupDir = self.settings.value("Bnr/backupDir", defaultDir)

  def add(self, network):
    if network is None:
      return

    self.networks.append(message)
    self.save()

  def remove(self, network):
    if network is None:
      return

    self.networks.remove(network)
    self.save()

  def update(self, network):
    if network is None:
      return

    self.save()

  def save(self):
    jsonStr = json.dumps(self.networks)
    p = path.join(self.backupDir, "networks", "network.json")
    if not path.exists(p):
      makedirs(p)

    with open(p, "w+") as f:
      f.write(jsonStr)

class Network:
  ssid = None
  password = None
  def __init__(self, ssid, password):
    self.ssid = ssid
    self.password = password

class Superuser:
  isRooted = False
  hasBusybox = False
  def __init__(self, isRooted, hasBusybox):
    self.isRooted = isRooted
    self.hasBusybox = hasBusybox

class SyncMessage:
  type = None
  serial = None
  message = None
  contact = None
  network = None
  superuser = None
  def __init__(self, type):
    self.type = type

  def utf8TruncationIndex(self, value, maxLen):
    length = len(value)
    if length <= maxLen:
      return length
    
    length = maxLen
    while (value[length] & 0x80) != 0 and (value[length] & 0xc0) != 0xc0:
      length -= 1

    return length

  def serialize(self):
    type = self.type
    
    jsonStr = None
    if type == SYNC_MSG_TYPE_CONTACT:
      jsonStr = json.dumps(self.contact.__dict__)

    elif type == SYNC_MSG_TYPE_MESSAGE:
      jsonStr = json.dumps(self.message.__dict__)
      
    if jsonStr is None and not type is None:
      buffer = struct.pack(f"!b", type)
      return buffer 

    length = self.utf8TruncationIndex(jsonStr, SYNC_MSG_MAX_LENGTH)
    buffer = struct.pack(f"!bi{length}s", 
      type,
      length,
      bytes(jsonStr, encoding="ascii")
    )
    return buffer

  def deserialize(self, jsonStr):
    type = self.type
    # length, = struct.unpack_from("!i", buffer, 1)
    # print(f"json length {length}")
    """
    Integer value always take 4 bytes in size.
    """
    # jsonStr, = struct.unpack(f"!{length}s", buffer)

    obj = json.loads(jsonStr)
    if type == SYNC_MSG_TYPE_CONTACT:
      self.contact = Contact(obj['id'], obj['name'], obj['number'])
    elif type == SYNC_MSG_TYPE_MESSAGE:
      self.message = Message(
        obj['id'], 
        obj['threadId'],
        obj['type'], 
        obj['address'], 
        obj['body'], 
        obj['date'],
        obj['dateSent'],
        obj['read']
      )
    elif type == SYNC_MSG_TYPE_SUPERUSER:
      isRooted = obj['isRooted']
      hasBusybox = obj['hasBusybox']
      self.superuser = Superuser(isRooted, hasBusybox)
    else:
      print(f"Unknown data type {type}")


class BnrClientHandler(QThread):
  _descriptor = None
  socket = None
  shouldTerminate = False
  shouldRestart = False
  mutex = QMutex()
  condition = QWaitCondition()
  _messages = []
  messageList = MessageList()
  contactList = ContactList()
  networkList = NetworkList()
  deviceName = None
  
  dataLen = 0
  msgData = bytearray(b"")

  def __init__(self, parent, descriptor):
    QThread.__init__(self, parent)
    self._descriptor = descriptor
    self.socket = QTcpSocket(self)
    self.socket.setSocketDescriptor(descriptor)
    self.socket.readyRead.connect(self.readyRead)
    self.threads = []

  def stop(self):
    print("stopping client handler...")
    self.shouldTerminate = True
    self.condition.wakeAll()
    self.socket.readyRead.disconnect(self.readyRead)
    self.socket.close()
    self.socket.deleteLater()
    while(self.isRunning()):
      time.sleep(0.1)

  def start(self):
    if not self.isRunning():
      super().start(QThread.Priority.NormalPriority)
    else:
      shouldRestart = True
      self.condition.wakeAll()

    msg = SyncMessage(SYNC_MSG_TYPE_SUPERUSER)
    self._messages.append(msg)

  def handleClientMessage(self, type, jsonStr):
    msg = SyncMessage(type)
    msg.deserialize(jsonStr)

    if msg.type == SYNC_MSG_TYPE_CONTACT:
      self.contactList.add(msg.contact)
    elif msg.type == SYNC_MSG_TYPE_MESSAGE:
      self.messageList.add(msg.message)
    elif msg.type == SYNC_MSG_TYPE_NETWORK:
      self.networkList.add(msg.network)
    elif msg.type == SYNC_MSG_TYPE_REQUEST_RESTORE:
      self.prepareRestore()

  # def prepareRestore(self, serial):

  @pyqtSlot()
  def readyRead(self):
    chunk = self.socket.readAll()
    length = chunk.size()

    if self.deviceName is None:
        name = chunk.mid(0, 64).data().decode('ascii')
        self.deviceName = str(name)
        # cursor += 64
        return

    self.msgData += chunk.data()

    while len(self.msgData) > 0:
      type, = struct.unpack_from("!b", self.msgData[0:1], 0)
      dataLen, = struct.unpack_from("!i", self.msgData[1:5], 0)
      self.dataLen = dataLen
      
      if len(self.msgData) - 5 >= self.dataLen:
        jsonStr, = struct.unpack_from(f"!{self.dataLen}s", self.msgData[5:self.dataLen + 5], 0)

        thread = threading.Thread(target=self.handleClientMessage, args=(type, jsonStr))
        self.threads.append(thread)
        thread.start()

        # clean 
        del self.msgData[0:1]
        del self.msgData[0:4]
        del self.msgData[0:self.dataLen]
      else:
        break

  def run(self):
    self.mutex.lock()
    descriptor = self._descriptor
    socket = QTcpSocket(self)
    socket.setSocketDescriptor(descriptor)
    self.mutex.unlock()

    while(True):
      
      while(len(self._messages) > 0):
        self.mutex.lock()
        msg = self._messages.pop()
        self.mutex.unlock()
        
        socket.write(msg.serialize())
        socket.flush()
      

      self.mutex.lock();
      if not self.shouldRestart:
        self.condition.wait(self.mutex);
      self.shouldRestart = False;
      self.mutex.unlock();

      if self.shouldTerminate:
        print("stopping client handler thread")
        socket.close()
        socket.deleteLater()
        break



class BnrServer(QTcpServer):
  server = None
  _clients = []
  def __init__(self, parent=None):
    QObject.__init__(self, parent=parent)

  def start(self):
    print("starting server...")
    if self.isListening():
      return

    if not self.listen(QHostAddress.Any, 59560):
      print("Unable to start server")
      return

    ipAddress = None
    ipAddresses = QNetworkInterface.allAddresses()

    for address in ipAddresses:
      if address != QHostAddress.LocalHost and address.toIPv4Address() > 0:
        print(f"address {address.toString()}")
        ipAddress = address.toString()

    if ipAddress is None:
      ipAddress = QHostAddress(QHostAddress.LocalHost).toString()

    print(f"The server is running on\nIP: {ipAddress}\nport: {self.serverPort()}")

  def stop(self):
    print("stopping server...")
    if self.isListening():
      self.close()
    
    for client in self._clients:
      client.stop()
      client.deleteLater()

    self._clients = []

    while(self.isListening()):
      time.sleep(0.1)
    
    self.deleteLater()

  def restart(self):
    print("restarting server...")
    self.stop()
    self.start()

  def incomingConnection(self, descriptor):
    handler = BnrClientHandler(self, descriptor)
    handler.start()
    self._clients.append(handler)

