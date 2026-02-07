#ifndef CONTROL_MESSAGE_H
#define CONTROL_MESSAGE_H

#include <stddef.h>
#include <stdint.h>

#include "input.h"
#include "keycodes.h"

#define CONTROL_MSG_INJECT_TEXT_MAX_LENGTH    300
#define CONTROL_MSG_MAX_SIZE                  (1 << 18) // 256k
#define CONTROL_MSG_CLIPBOARD_TEXT_MAX_LENGTH (CONTROL_MSG_MAX_SIZE - 6)

#define POINTER_ID_MOUSE          UINT64_C(-1);
#define POINTER_ID_VIRTUAL_FINGER UINT64_C(-2);

struct Size {
  uint16_t width;
  uint16_t height;
};

struct Point {
  int32_t x;
  int32_t y;
};

struct Position {
  // The video screen size may be different from the real device screen size,
  // so store to which size the absolute position apply, to scale it
  // accordingly.
  struct Size screenSize;
  struct Point point;
};

enum ControlMessageType {
  CONTROL_MSG_TYPE_INJECT_KEYCODE,
  CONTROL_MSG_TYPE_INJECT_TEXT,
  CONTROL_MSG_TYPE_INJECT_TOUCH_EVENT,
  CONTROL_MSG_TYPE_INJECT_SCROLL_EVENT,
  CONTROL_MSG_TYPE_BACK_OR_SCREEN_ON,
  CONTROL_MSG_TYPE_EXPAND_NOTIFICATION_PANEL,
  CONTROL_MSG_TYPE_COLLAPSE_NOTIFICATION_PANEL,
  CONTROL_MSG_TYPE_GET_CLIPBOARD,
  CONTROL_MSG_TYPE_SET_CLIPBOARD,
  CONTROL_MSG_TYPE_SET_SCREEN_POWER_MODE,
  CONTROL_MSG_TYPE_ROTATE_DEVICE,
};
enum ScreenPowerMode {
  // see
  // <https://android.googlesource.com/platform/frameworks/base.git/+/pie-release-2/core/java/android/view/SurfaceControl.java#305>
  SCREEN_POWER_MODE_OFF = 0,
  SCREEN_POWER_MODE_NORMAL = 2,
};
class ControlMessage {
private:
  /* data */
  void write16be(uint8_t *buf, uint16_t value);
  void write32be(uint8_t *buf, uint32_t value);
  void write64be(uint8_t *buf, uint64_t value);
  void writePosition(uint8_t *buf, const struct Position *position);
  size_t writeString(const char *utf8, size_t max_len, unsigned char *buf);

public:
  ControlMessage(/* args */);
  ~ControlMessage();

  int serialize(unsigned char *buff);

  enum ControlMessageType type;
  struct {
    enum AndroidKeyeventAction action;
    enum AndroidKeycode keycode;
    uint32_t repeat;
    enum AndroidMetastate metastate;
  } injectKeycode;
  struct {
    char *text; // owned, to be freed by SDL_free()
  } injectText;
  struct {
    enum AndroidMotioneventAction action;
    enum AndroidMotioneventButtons buttons;
    uint64_t pointerId;
    struct Position position;
    float pressure;
  } injectTouchEvent;
  struct {
    struct Position position;
    int32_t hscroll;
    int32_t vscroll;
  } injectScrollEvent;
  struct {
    char *text; // owned, to be freed by SDL_free()
    bool paste;
  } setClipboard;
  struct {
    enum ScreenPowerMode mode;
  } setScreenPowerMode;
};

#endif
