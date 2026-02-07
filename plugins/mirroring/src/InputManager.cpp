#include "InputManager.h"
#include "DeviceView.h"
#include "EventConverter.h"
#include <QClipboard>
#include <QCursor>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>
static const int ACTION_DOWN = 1;
static const int ACTION_UP = 1 << 1;

InputManager::InputManager(DeviceController *parent, DeviceView *view) :
    QObject(parent), converter(new EventConverter(this)) {

  this->view = view;
  this->controller = parent;
}

InputManager::~InputManager() {
  converter->deleteLater();
}

struct Point InputManager::inversePoint(struct Point point, struct Size size) {
  point.x = size.width - point.x;
  point.y = size.height - point.y;
  return point;
}

void InputManager::actionHome(int actions) {
  sendKeycode(AKEYCODE_HOME, actions, "HOME");
}

void InputManager::actionBack(int actions) {
  sendKeycode(AKEYCODE_BACK, actions, "BACK");
}

void InputManager::actionAppSwitch(int actions) {
  sendKeycode(AKEYCODE_APP_SWITCH, actions, "APP_SWITCH");
}

void InputManager::actionPower(int actions) {
  sendKeycode(AKEYCODE_POWER, actions, "POWER");
}

void InputManager::actionVolumeUp(int actions) {
  sendKeycode(AKEYCODE_VOLUME_UP, actions, "VOLUME_UP");
}

void InputManager::actionVolumeDown(int actions) {
  sendKeycode(AKEYCODE_VOLUME_DOWN, actions, "VOLUME_DOWN");
}

void InputManager::actionMenu(int actions) {
  sendKeycode(AKEYCODE_MENU, actions, "MENU");
}

void InputManager::actionCopy(int actions) {
  sendKeycode(AKEYCODE_COPY, actions, "COPY");
}

void InputManager::actionCut(int actions) {
  sendKeycode(AKEYCODE_CUT, actions, "CUT");
}

void InputManager::pressBackOrTurnScreenOn() {
  ControlMessage *msg = new ControlMessage();
  msg->type = CONTROL_MSG_TYPE_BACK_OR_SCREEN_ON;
  controller->pushMessage(msg);
}

void InputManager::setDeviceClipboard(bool paste) {
  QClipboard *clipboard = QGuiApplication::clipboard();
  char *text = clipboard->text().toLocal8Bit().data();
  if (!text) {
    qWarning() << "Could not get clipboard text";
    return;
  }
  qDebug() << "set device clipboard" << text;

  if (!*text) {
    // empty text;
    return;
  }

  ControlMessage *msg = new ControlMessage();
  msg->type = CONTROL_MSG_TYPE_SET_CLIPBOARD;
  msg->setClipboard.text = text;
  msg->setClipboard.paste = paste;

  if (!controller->pushMessage(msg)) {
    qWarning() << "Could not request 'set device clipboard'";
  }
}

void InputManager::clipboardPaste() {
  QClipboard *clipboard = QGuiApplication::clipboard();
  char *text = clipboard->text().toLocal8Bit().data();
  if (!text) {
    qWarning() << "Could not get clipboard text";
    return;
  }
  qDebug() << "paste clipboard" << text;

  if (!*text) {
    // empty text;
    return;
  }

  ControlMessage *msg = new ControlMessage();
  msg->type = CONTROL_MSG_TYPE_INJECT_TEXT;
  msg->injectText.text = text;
  if (!controller->pushMessage(msg)) {
    qWarning() << "Could not request 'paste clipboard'";
  }
}

void InputManager::processMouseButton(QMouseEvent *e) {
  QEvent::Type type = e->type();

  bool down = type == QEvent::MouseButtonPress;
  if (down) {
    if (e->button() == Qt::MouseButton::RightButton) {
      pressBackOrTurnScreenOn();
      return;
    }

    if (e->button() == Qt::MouseButton::MiddleButton) {
      actionHome(ACTION_DOWN | ACTION_UP);
      return;
    }
  }

  ControlMessage *msg = new ControlMessage();
  if (!convertMouseButton(e, msg)) {
    qDebug() << "cannot convert mouse button";
    return;
  }

  if (!controller->pushMessage(msg)) {
    return;
  }

  // Pinch-to-zoom simulation.
  //
  // If Ctrl is hold when the left-click button is pressed, then
  // pinch-to-zoom mode is enabled: on every mouse event until the left-click
  // button is released, an additional "virtual finger" event is generated,
  // having a position inverted through the center of the screen.
  //
  // In other words, the center of the rotation/scaling is the center of the
  // screen.
  bool ctrlIsPressed = QGuiApplication::keyboardModifiers().testFlag(Qt::ControlModifier);
  if ((down && !vFingerDown && ctrlIsPressed) || (!down && vFingerDown)) {
    struct Point mouse = msg->injectTouchEvent.position.point;
    struct Point vFinger = inversePoint(mouse, view->frameSize);
    enum AndroidMotioneventAction action
        = down ? AMOTION_EVENT_ACTION_DOWN : AMOTION_EVENT_ACTION_UP;
    if (!simulateVirtualFinger(action, vFinger)) {
      return;
    }
    vFingerDown = down;
  }
}

bool InputManager::simulateVirtualFinger(enum AndroidMotioneventAction action, struct Point point) {
  bool up = action == AMOTION_EVENT_ACTION_UP;

  ControlMessage *msg = new ControlMessage();
  msg->type = CONTROL_MSG_TYPE_INJECT_TOUCH_EVENT;
  msg->injectTouchEvent.action = action;
  msg->injectTouchEvent.position.screenSize = view->frameSize;
  msg->injectTouchEvent.position.point = point;
  msg->injectTouchEvent.pointerId = POINTER_ID_VIRTUAL_FINGER;
  msg->injectTouchEvent.pressure = up ? 0.0f : 1.0f;
  msg->injectTouchEvent.buttons = AMOTION_EVENT_BUTTON_NONE;

  if (!controller->pushMessage(msg)) {
    return false;
  }

  return true;
}

bool InputManager::isShortcutMod(quint32 mod) {
  // mod &= Qt::KeyboardModifierMask;

  return mod & Qt::KeyboardModifierMask;
}

void InputManager::processMouseWheel(QWheelEvent *e) {
  ControlMessage *msg = new ControlMessage();
  if (convertMouseWheel(e, msg)) {
    if (!controller->pushMessage(msg)) {
      qWarning() << "Could not request 'inject mouse wheel event'";
    }
  }
}

void InputManager::processKey(QKeyEvent *e) {

  bool smod = isShortcutMod(e->modifiers());

  int keycode = e->key();
  bool down = e->type() == QEvent::KeyPress;
  bool ctrl = e->modifiers() & Qt::KeyboardModifier::ControlModifier;
  bool shift = e->modifiers() & Qt::KeyboardModifier::ShiftModifier;
  bool repeat = e->isAutoRepeat();

  if (smod) {
    int action = down ? ACTION_DOWN : ACTION_UP;
    qDebug() << "is shortcut mod";
    switch (keycode) {
      case Qt::Key_H:
        if (ctrl && !shift && !repeat) {
          actionHome(action);
        }
        return;
      case Qt::Key_B:
      case Qt::Key_Backspace:
        if (ctrl && !shift && !repeat) {
          actionBack(action);
        }
        return;
      case Qt::Key_S:
        if (ctrl && !shift && !repeat) {
          actionAppSwitch(action);
        }
        return;
      case Qt::Key_M:
        if (ctrl && !shift && !repeat) {
          actionMenu(action);
        }
        return;
      case Qt::Key_P:
        if (ctrl && !shift && !repeat) {
          actionPower(action);
        }
        return;
      case Qt::Key_O:
        // if (ctrl && !shift && !repeat) {
        //   actionBack(action);
        // }
        return;
      case Qt::Key_Down:
        if (ctrl && !shift) {
          actionVolumeDown(action);
        }
        return;
      case Qt::Key_Up:
        if (ctrl && !shift) {
          actionVolumeUp(action);
        }
        return;
      case Qt::Key_C:
        if (ctrl && !shift && !repeat) {
          actionCopy(action);
        }
        return;
      case Qt::Key_X:
        if (ctrl && !shift && !repeat) {
          actionCut(action);
        }
        return;
      case Qt::Key_V:
        if (ctrl && !repeat && down) {
          if (shift) {
            clipboardPaste();
          } else {
            setDeviceClipboard(true);
          }
        }
        return;
    }
    return;
  }

  if (e->isAutoRepeat()) {
    if (!forwardKeyRepeat) {
      return;
    }
    ++this->repeat;
  } else {
    this->repeat = 0;
  }

  if (ctrl && !shift && keycode == Qt::Key_V && down && !repeat) {
    // Synchronize the computer clipboard to the device clipboard before
    // sending Ctrl+v, to allow seamless copy-paste.
    setDeviceClipboard(false);
  }

  ControlMessage *msg = new ControlMessage();
  // const char *c = e->text().toStdString().c_str();
  // if (!isalpha(*c) && *c != ' ' && e->text() != "" && !(e->modifiers() & Qt::KeypadModifier)) {
  //   qDebug() << "not letters or space and keypad button" << c;
  //   // return;
  // }
  if (convertInputKey(e, msg)) {
    if (!controller->pushMessage(msg)) {
      qWarning() << "Could not request 'inject keycode'";
    }
  }
}
void InputManager::processMouseMotion(QMouseEvent *e) {
  if (!e->buttons()) {
    return;
  }

  ControlMessage *msg = new ControlMessage();
  if (!convertMouseMotion(e, msg)) {
    return;
  }

  if (!controller->pushMessage(msg)) {
    qWarning() << "Could not request 'inject mouse motion event'";
  }

  if (vFingerDown) {
    struct Point mouse = msg->injectTouchEvent.position.point;
    struct Point vfinger = inversePoint(mouse, view->frameSize);
    simulateVirtualFinger(AMOTION_EVENT_ACTION_MOVE, vfinger);
  }
}

void InputManager::processTextInput(QKeyEvent *e) {
  if (e->type() == QEvent::KeyRelease)
    return;
  if (isShortcutMod(e->modifiers())) {
    return;
  }

  if (!preferText) {
    const char *c = e->text().toStdString().c_str();
    if (isalpha(*c) || *c == ' ') {
      return;
    }
  }

  ControlMessage *msg = new ControlMessage();
  msg->type = CONTROL_MSG_TYPE_INJECT_TEXT;
  msg->injectText.text = e->text().toLocal8Bit().data();
  if (!msg->injectText.text) {
    return;
  }

  if (!controller->pushMessage(msg)) {
    qWarning() << "Could not request 'inject text'";
  }
}

bool InputManager::convertInputKey(QKeyEvent *e, ControlMessage *msg) {
  msg->type = CONTROL_MSG_TYPE_INJECT_KEYCODE;

  if (!converter->convertKeycodeAction(e->type(), &msg->injectKeycode.action)) {
    return false;
  }

  Qt::KeyboardModifiers mod = e->modifiers();
  if (!converter->convertKeycode(e->key(), &msg->injectKeycode.keycode, mod, preferText)) {
    // try to inject as text
    processTextInput(e);
    return false;
  }

  msg->injectKeycode.repeat = repeat;
  msg->injectKeycode.metastate = converter->convertMetaState(mod);

  return true;
}

bool InputManager::convertMouseWheel(QWheelEvent *e, ControlMessage *msg) {
  QPointF point = view->mapFromGlobal(e->globalPosition());

  // clang-format off
  struct Position position = { 
    .screenSize = view->frameSize,
    .point = view->convertWindowToFrameCoords(point.x(), point.y()) 
  };
  // clang-format on

  QPoint numDegrees = e->angleDelta() / 8;
  QPoint numSteps = numDegrees / 15;

  msg->type = CONTROL_MSG_TYPE_INJECT_SCROLL_EVENT;
  msg->injectScrollEvent.position = position;
  msg->injectScrollEvent.hscroll = numSteps.x();
  msg->injectScrollEvent.vscroll = numSteps.y();

  return true;
}

bool InputManager::convertMouseMotion(QMouseEvent *e, ControlMessage *msg) {
  msg->type = CONTROL_MSG_TYPE_INJECT_TOUCH_EVENT;
  msg->injectTouchEvent.action = AMOTION_EVENT_ACTION_MOVE;
  msg->injectTouchEvent.pointerId = POINTER_ID_MOUSE;
  msg->injectTouchEvent.position.screenSize = view->frameSize;
  msg->injectTouchEvent.position.point = view->convertWindowToFrameCoords(e->x(), e->y());
  msg->injectTouchEvent.pressure = 1.f;
  msg->injectTouchEvent.buttons = converter->convertMouseButtons(e->buttons());

  return true;
}

bool InputManager::convertMouseButton(QMouseEvent *e, ControlMessage *msg) {

  msg->type = CONTROL_MSG_TYPE_INJECT_TOUCH_EVENT;

  if (!converter->convertMouseAction(e->type(), &msg->injectTouchEvent.action)) {
    return false;
  }

  msg->injectTouchEvent.pointerId = POINTER_ID_MOUSE;
  msg->injectTouchEvent.position.screenSize = view->frameSize;
  msg->injectTouchEvent.pressure = e->type() == QMouseEvent::MouseButtonPress ? 1.f : 0.f;
  msg->injectTouchEvent.buttons = converter->convertMouseButtons(e->button());
  msg->injectTouchEvent.position.point = view->convertWindowToFrameCoords(e->x(), e->y());

  return true;
}

void InputManager::sendKeycode(AndroidKeycode keycode, int actions, const char *name) {
  ControlMessage *msg = new ControlMessage();
  msg->type = CONTROL_MSG_TYPE_INJECT_KEYCODE;
  msg->injectKeycode.keycode = keycode;
  msg->injectKeycode.repeat = 0;
  msg->injectKeycode.metastate = AMETA_NONE;

  if (actions & ACTION_DOWN) {
    msg->injectKeycode.action = AKEY_EVENT_ACTION_DOWN;
    if (!controller->pushMessage(msg)) {
      qWarning("Couldn't request 'inject %s (DOWN)'", name);
      return;
    }
  }

  if (actions & ACTION_UP) {
    msg->injectKeycode.action = AKEY_EVENT_ACTION_UP;
    if (!controller->pushMessage(msg)) {
      qWarning("Couldn't request 'inject %s (UP)'", name);
    }
  }
}
