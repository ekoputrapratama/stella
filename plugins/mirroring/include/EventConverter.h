#ifndef EVENT_CONVERTER_H
#define EVENT_CONVERTER_H

#include "ControlMessage.h"
#include <QDebug>
#include <QEvent>
#include <QObject>

class EventConverter : public QObject {
  Q_OBJECT
private:
  /* data */
public:
  EventConverter(QObject *parent = nullptr);
  ~EventConverter();

  bool convertMouseAction(QEvent::Type type, enum AndroidMotioneventAction *action);
  bool convertKeycode(
      int from, enum AndroidKeycode *to, Qt::KeyboardModifiers mod, bool preferText);
  bool convertKeycodeAction(QEvent::Type type, enum AndroidKeyeventAction *action);
  bool isCapsLockOn();
  bool isNumLockOn();

  enum AndroidMetastate convertMetaState(Qt::KeyboardModifiers mod);
  enum AndroidMetastate autocompleteMetastate(enum AndroidMetastate metastate);
  enum AndroidMotioneventButtons convertMouseButtons(Qt::MouseButtons btn);
};

#endif
