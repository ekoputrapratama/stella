import stella 

from PyQt5.QtCore import QObject
# import .resources.icons

def testMethod():
  print("testMethod called")


def run():
  stella.createWindow("qrc:/mirroring/mirroring.qml")
  test = dict(fun=testMethod)
  stella.registerObject("test",test)
  print("root plugin run()")
  
  
def name():
  return b"Root"

def icon():
  return b"qrc:/root/root.png"
