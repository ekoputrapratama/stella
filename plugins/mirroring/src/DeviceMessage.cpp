#include "DeviceMessage.h"
#include <QDebug>

DeviceMessage::DeviceMessage(/* args */) {
}

DeviceMessage::~DeviceMessage() {
}

uint32_t DeviceMessage::read32be(const uint8_t *buf) {
  return (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
}

ssize_t DeviceMessage::deserialize(const unsigned char *buf, size_t len) {
  if (len < 5) {
    // at least type + empty string length
    return 0; // not available
  }
  type = (DeviceMessageType)buf[0];
  switch (type) {
    case DEVICE_MSG_TYPE_CLIPBOARD: {
      size_t clipboard_len = read32be(&buf[1]);
      if (clipboard_len > len - 5) {
        return 0; // not available
      }
      char *text = (char *)malloc(clipboard_len + 1);
      if (!text) {
        qWarning("Could not allocate text for clipboard");
        return -1;
      }
      if (clipboard_len) {
        memcpy(text, &buf[5], clipboard_len);
      }
      text[clipboard_len] = '\0';

      clipboard.text = text;
      return 5 + clipboard_len;
    }
    default:
      qWarning("Unknown device message type: %d", (int)type);
      return -1;
  }
}

void DeviceMessage::destroy() {
  if (type == DEVICE_MSG_TYPE_CLIPBOARD) {
    free(clipboard.text);
  }
}
