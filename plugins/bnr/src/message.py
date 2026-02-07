
import json
import struct
from os import path, makedirs
from stella.config import Configuration
from PyQt5.QtCore import (QObject, QSettings)

SYNC_MSG_MAX_LENGTH = 1024

SYNC_MSG_TYPE_CONTACT = 1
SYNC_MSG_TYPE_MESSAGE = 2
SYNC_MSG_TYPE_NETWORK = 3
SYNC_MSG_TYPE_ADB_ENABLED = 4
SYNC_MSG_TYPE_ENABLE_ADB_WIRELESS = 5
SYNC_MSG_TYPE_HAS_ROOT_ACCESS = 6
SYNC_MSG_TYPE_HAS_BUSYBOX = 7
SYNC_MSG_TYPE_FILE = 8

SYNC_MSG_TYPE_REQUEST_BACKUP = 9
SYNC_MSG_TYPE_REQUEST_RESTORE = 10

MESSAGE_TYPE_SENT = 0
MESSAGE_TYPE_INBOX = 1
MESSAGE_TYPE_DRAFT = 2

FILE_TYPE_APK = 1
FILE_TYPE_DATA = 2
FILE_TYPE_SYSTEM_DATA = 3
FILE_TYPE_OBB = 4
FILE_TYPE_MEDIA = 5
# minimum required data length to get information about the data for file data type
# including data type, file type, file name and data length
MIN_MSG_FILE_LENGTH = 2
MIN_MSG_FILE_APK_LENGTH = 10
MIN_MSG_FILE_DATA_LENGTH = 14

class ContactList:
  contacts = []
  def __init__(self, serial):
    self.settings = Configuration("Stella")
    
    defaultDir = path.expanduser("~/Documents/Stella/backups")
    if not self.settings.has("backupDir"):
      self.settings.set("backupDir", defaultDir, True)

    backupDir = self.settings.get("backupDir", defaultDir)
    self.backupDir = path.join(backupDir, serial)

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
  def __init__(self, serial):
    self.settings = Configuration("Stella")
    
    defaultDir = path.expanduser("~/Documents/Stella/backups")
    if not self.settings.has("backupDir"):
      self.settings.set("backupDir", defaultDir, True)

    backupDir = self.settings.get("backupDir", defaultDir)
    self.backupDir = path.join(backupDir, serial)

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
  def __init__(self, serial):
    self.settings = Configuration("Stella")
    
    defaultDir = path.expanduser("~/Documents/Stella/backups")
    if not self.settings.has("backupDir"):
      self.settings.set("backupDir", defaultDir, True)

    self.backupDir = self.settings.get("backupDir", defaultDir)

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
