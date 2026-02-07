#include "EventConverter.h"
#include "common.h"

#ifdef Q_OS_WIN32
#include <windows.h>
#else
#include <X11/XKBlib.h>
#undef KeyPress
#undef KeyRelease
#undef FocusIn
#undef FocusOut
#endif
// clang-format off
#define MAP(FROM, TO) case FROM: *to = TO; return true
#define FAIL default: return false
#define MOD_LSHIFT 0x50000000
#define MOD_RSHIFT 0x62000000
#define MOD_LCTRL 37
#define MOD_RCTRL 105
#define MOD_NUM_LOCK 77
#define MOD_CAPS_LOCK 66
// clang-format on
EventConverter::EventConverter(QObject *parent) : QObject(parent) {
}

EventConverter::~EventConverter() {
}

bool EventConverter::convertMouseAction(QEvent::Type type, enum AndroidMotioneventAction *to) {
  // Qt::Key_Eisu_Shift
  switch (type) {
    MAP(QEvent::Type::MouseButtonPress, AMOTION_EVENT_ACTION_DOWN);
    MAP(QEvent::Type::MouseButtonRelease, AMOTION_EVENT_ACTION_UP);
    FAIL;
  }
}

enum AndroidMotioneventButtons EventConverter::convertMouseButtons(Qt::MouseButtons state) {
  enum AndroidMotioneventButtons buttons = AMOTION_EVENT_BUTTON_NONE;

  if (state & Qt::MouseButton::LeftButton) {
    buttons |= AMOTION_EVENT_BUTTON_PRIMARY;
  }
  if (state & Qt::MouseButton::RightButton) {
    buttons |= AMOTION_EVENT_BUTTON_SECONDARY;
  }
  if (state & Qt::MouseButton::MiddleButton) {
    buttons |= AMOTION_EVENT_BUTTON_TERTIARY;
  }
  if (state & Qt::MouseButton::XButton1) {
    buttons |= AMOTION_EVENT_BUTTON_BACK;
  }
  if (state & Qt::MouseButton::XButton2) {
    buttons |= AMOTION_EVENT_BUTTON_FORWARD;
  }

  return buttons;
}

enum AndroidMetastate EventConverter::autocompleteMetastate(enum AndroidMetastate metastate) {
  if (metastate & (AMETA_SHIFT_LEFT_ON | AMETA_SHIFT_RIGHT_ON)) {
    metastate |= AMETA_SHIFT_ON;
  }
  if (metastate & (AMETA_CTRL_LEFT_ON | AMETA_CTRL_RIGHT_ON)) {
    metastate |= AMETA_CTRL_ON;
  }
  if (metastate & (AMETA_ALT_LEFT_ON | AMETA_ALT_RIGHT_ON)) {
    metastate |= AMETA_ALT_ON;
  }
  if (metastate & (AMETA_META_LEFT_ON | AMETA_META_RIGHT_ON)) {
    metastate |= AMETA_META_ON;
  }

  return metastate;
}
enum AndroidMetastate EventConverter::convertMetaState(Qt::KeyboardModifiers mod) {
  enum AndroidMetastate metastate = (AndroidMetastate)0;

  if (mod & Qt::ShiftModifier) {
    qDebug() << "shift modifier present";
    metastate |= AMETA_SHIFT_ON;
  }

  if (mod & Qt::ControlModifier) {
    qDebug() << "control modifier present";
    metastate |= AMETA_CTRL_ON;
  }

  if (mod & Qt::MetaModifier) {
    qDebug() << "meta modifier present";
    metastate |= AMETA_META_ON;
  }

  if (mod & Qt::AltModifier) {
    qDebug() << "alt modifier present";
    metastate |= AMETA_ALT_ON;
  }

  if (isNumLockOn()) {
    qDebug() << "num lock is on";
    metastate |= AMETA_NUM_LOCK_ON;
  }

  if (isCapsLockOn()) {
    qDebug() << "caps lock is on";
    metastate |= AMETA_CAPS_LOCK_ON;
  }

  return autocompleteMetastate(metastate);
}

bool EventConverter::convertKeycode(
    int from, enum AndroidKeycode *to, Qt::KeyboardModifiers mod, bool preferText) {

  switch (from) {
    MAP(Qt::Key_Return, AKEYCODE_ENTER);
    MAP(Qt::Key_Escape, AKEYCODE_ESCAPE);
    MAP(Qt::Key_Backspace, AKEYCODE_DEL);
    MAP(Qt::Key_Tab, AKEYCODE_TAB);
    MAP(Qt::Key_PageUp, AKEYCODE_PAGE_UP);
    MAP(Qt::Key_PageDown, AKEYCODE_PAGE_DOWN);
    MAP(Qt::Key_Delete, AKEYCODE_FORWARD_DEL);
    MAP(Qt::Key_Home, AKEYCODE_MOVE_HOME);
    MAP(Qt::Key_End, AKEYCODE_MOVE_END);
    MAP(Qt::Key_Right, AKEYCODE_DPAD_RIGHT);
    MAP(Qt::Key_Left, AKEYCODE_DPAD_LEFT);
    MAP(Qt::Key_Down, AKEYCODE_DPAD_DOWN);
    MAP(Qt::Key_Up, AKEYCODE_DPAD_UP);
  }

  if ((mod & Qt::KeypadModifier) && !isNumLockOn()) {
    switch (from) {
      MAP(Qt::Key_0, AKEYCODE_INSERT);
      MAP(Qt::Key_1, AKEYCODE_MOVE_END);
      MAP(Qt::Key_2, AKEYCODE_DPAD_DOWN);
      MAP(Qt::Key_3, AKEYCODE_PAGE_DOWN);
      MAP(Qt::Key_4, AKEYCODE_DPAD_LEFT);
      MAP(Qt::Key_6, AKEYCODE_DPAD_RIGHT);
      MAP(Qt::Key_7, AKEYCODE_MOVE_HOME);
      MAP(Qt::Key_8, AKEYCODE_DPAD_UP);
      MAP(Qt::Key_9, AKEYCODE_PAGE_UP);
      MAP(Qt::Key_Period, AKEYCODE_FORWARD_DEL);
    }
  }

  if (isNumLockOn() && (mod | Qt::KeypadModifier)) {
    switch (from) {
      MAP(Qt::Key_0, AKEYCODE_0);
      MAP(Qt::Key_1, AKEYCODE_1);
      MAP(Qt::Key_2, AKEYCODE_2);
      MAP(Qt::Key_3, AKEYCODE_3);
      MAP(Qt::Key_4, AKEYCODE_4);
      MAP(Qt::Key_5, AKEYCODE_5);
      MAP(Qt::Key_6, AKEYCODE_6);
      MAP(Qt::Key_7, AKEYCODE_7);
      MAP(Qt::Key_8, AKEYCODE_8);
      MAP(Qt::Key_9, AKEYCODE_9);
      MAP(Qt::Key_Period, AKEYCODE_PERIOD);
    }
  }

  if (preferText && !(mod & Qt::ControlModifier)) {
    return false;
  }

  if (mod & (Qt::AltModifier | Qt::MetaModifier)) {
    return false;
  }

  switch (from) {
    MAP(Qt::Key_A, AKEYCODE_A);
    MAP(Qt::Key_B, AKEYCODE_B);
    MAP(Qt::Key_C, AKEYCODE_C);
    MAP(Qt::Key_D, AKEYCODE_D);
    MAP(Qt::Key_E, AKEYCODE_E);
    MAP(Qt::Key_F, AKEYCODE_F);
    MAP(Qt::Key_G, AKEYCODE_G);
    MAP(Qt::Key_H, AKEYCODE_H);
    MAP(Qt::Key_I, AKEYCODE_I);
    MAP(Qt::Key_J, AKEYCODE_J);
    MAP(Qt::Key_K, AKEYCODE_K);
    MAP(Qt::Key_L, AKEYCODE_L);
    MAP(Qt::Key_M, AKEYCODE_M);
    MAP(Qt::Key_N, AKEYCODE_N);
    MAP(Qt::Key_O, AKEYCODE_O);
    MAP(Qt::Key_P, AKEYCODE_P);
    MAP(Qt::Key_Q, AKEYCODE_Q);
    MAP(Qt::Key_R, AKEYCODE_R);
    MAP(Qt::Key_S, AKEYCODE_S);
    MAP(Qt::Key_T, AKEYCODE_T);
    MAP(Qt::Key_U, AKEYCODE_U);
    MAP(Qt::Key_V, AKEYCODE_V);
    MAP(Qt::Key_W, AKEYCODE_W);
    MAP(Qt::Key_X, AKEYCODE_X);
    MAP(Qt::Key_Y, AKEYCODE_Y);
    MAP(Qt::Key_Z, AKEYCODE_Z);
    MAP(Qt::Key_Space, AKEYCODE_SPACE);
    FAIL;
  }
}

bool EventConverter::convertKeycodeAction(QEvent::Type type, enum AndroidKeyeventAction *to) {
  switch (type) {
    MAP(QEvent::KeyPress, AKEY_EVENT_ACTION_DOWN);
    MAP(QEvent::KeyRelease, AKEY_EVENT_ACTION_UP);
    FAIL;
  }
}

bool EventConverter::isCapsLockOn() {
#ifdef Q_OS_WIN32
  return GetKeyState(VK_CAPITAL) == 1;
#else
  Display *d = XOpenDisplay((char *)0);
  bool caps_state = false;

  if (d) {
    unsigned n;
    XkbGetIndicatorState(d, XkbUseCoreKbd, &n);
    caps_state = (n & 0x01) == 1;
  }
  XCloseDisplay(d);
  return caps_state;
#endif
}

bool EventConverter::isNumLockOn() {
#ifdef Q_OS_WIN32
  return GetKeyState(VK_NUMLOCK) == 1;
#else
  Display *d = XOpenDisplay((char *)0);
  XKeyboardState x;
  XGetKeyboardControl(d, &x);
  XCloseDisplay(d);
  return x.led_mask & 2;
#endif
}
