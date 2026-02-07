#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include <QtCore>

#include "ControlMessage.h"
#include "DeviceController.h"

QT_BEGIN_NAMESPACE
class QMouseEvent;
class QWheelEvent;
class QKeyEvent;
QT_END_NAMESPACE

class DeviceController;
class DeviceView;
class EventConverter;

class InputManager : public QObject {
  Q_OBJECT
private:
  /* data */
  DeviceController *controller;
  EventConverter *converter;
  DeviceView *view;
  bool vFingerDown = false;
  bool forwardKeyRepeat = false;
  bool preferText = false;
  unsigned repeat;

  void pressBackOrTurnScreenOn();
  void actionHome(int action);
  void actionBack(int action);
  void actionAppSwitch(int actions);
  void actionMenu(int actions);
  void actionPower(int actions);
  void actionVolumeDown(int actions);
  void actionVolumeUp(int actions);
  void actionCopy(int actions);
  void actionCut(int actions);

  void sendKeycode(enum AndroidKeycode keycode, int actions, const char *name);
  void setDeviceClipboard(bool paste);
  void clipboardPaste();

  bool convertMouseButton(QMouseEvent *e, ControlMessage *msg);
  bool convertMouseMotion(QMouseEvent *e, ControlMessage *msg);
  bool convertMouseWheel(QWheelEvent *e, ControlMessage *msg);
  bool convertInputKey(QKeyEvent *e, ControlMessage *msg);

  struct Point inversePoint(struct Point point, struct Size size);
  bool simulateVirtualFinger(enum AndroidMotioneventAction action, struct Point point);
  bool isShortcutMod(quint32 mod);

public:
  InputManager(DeviceController *controller, DeviceView *view);
  ~InputManager();

  void processMouseButton(QMouseEvent *e);
  void processMouseMotion(QMouseEvent *e);
  void processMouseWheel(QWheelEvent *e);
  void processKey(QKeyEvent *e);
  void processTextInput(QKeyEvent *e);
};

#endif
