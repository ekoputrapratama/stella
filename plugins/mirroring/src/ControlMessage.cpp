#include "ControlMessage.h"
#include "StringUtil.h"
#include <string.h>

static uint16_t toFixedPoint16(float f) {
  // assert(f >= 0.0f && f <= 1.0f);
  uint32_t u = f * 0x1p16f; // 2^16
  if (u >= 0xffff) {
    u = 0xffff;
  }
  return (uint16_t)u;
}

ControlMessage::ControlMessage() {
}
ControlMessage::~ControlMessage() {
}

void ControlMessage::write16be(uint8_t *buf, uint16_t value) {
  buf[0] = value >> 8;
  buf[1] = value;
}

void ControlMessage::write32be(uint8_t *buf, uint32_t value) {
  buf[0] = value >> 24;
  buf[1] = value >> 16;
  buf[2] = value >> 8;
  buf[3] = value;
}

void ControlMessage::write64be(uint8_t *buf, uint64_t value) {
  write32be(buf, value >> 32);
  write32be(&buf[4], (uint32_t)value);
}

void ControlMessage::writePosition(uint8_t *buf, const struct Position *position) {
  write32be(&buf[0], position->point.x);
  write32be(&buf[4], position->point.y);
  write16be(&buf[8], position->screenSize.width);
  write16be(&buf[10], position->screenSize.height);
}

size_t ControlMessage::writeString(const char *utf8, size_t max_len, unsigned char *buf) {
  size_t len = utf8_truncation_index(utf8, max_len);
  write32be(buf, len);
  memcpy(&buf[4], utf8, len);
  return 4 + len;
}

int ControlMessage::serialize(unsigned char *buff) {
  buff[0] = type;
  switch (type) {
    case CONTROL_MSG_TYPE_BACK_OR_SCREEN_ON:
      return 1;
    case CONTROL_MSG_TYPE_INJECT_TOUCH_EVENT: {
      buff[1] = injectTouchEvent.action;
      write64be(&buff[2], injectTouchEvent.pointerId);
      writePosition(&buff[10], &injectTouchEvent.position);
      uint16_t pressure = toFixedPoint16(injectTouchEvent.pressure);
      write16be(&buff[22], pressure);
      write32be(&buff[24], injectTouchEvent.buttons);
      return 28;
    }
    case CONTROL_MSG_TYPE_INJECT_SCROLL_EVENT:
      writePosition(&buff[1], &injectScrollEvent.position);
      write32be(&buff[13], (uint32_t)injectScrollEvent.hscroll);
      write32be(&buff[17], (uint32_t)injectScrollEvent.vscroll);
      return 21;
    case CONTROL_MSG_TYPE_INJECT_KEYCODE:
      buff[1] = injectKeycode.action;
      write32be(&buff[2], injectKeycode.keycode);
      write32be(&buff[6], injectKeycode.repeat);
      write32be(&buff[10], injectKeycode.metastate);
      return 14;
    case CONTROL_MSG_TYPE_INJECT_TEXT: {
      size_t len = writeString(injectText.text, CONTROL_MSG_INJECT_TEXT_MAX_LENGTH, &buff[1]);
      return 1 + len;
    }
    case CONTROL_MSG_TYPE_SET_CLIPBOARD: {
      buff[1] = !!setClipboard.paste;
      size_t len = writeString(setClipboard.text, CONTROL_MSG_CLIPBOARD_TEXT_MAX_LENGTH, &buff[2]);
      return 2 + len;
    }
    default:
      return 0;
  }
}
