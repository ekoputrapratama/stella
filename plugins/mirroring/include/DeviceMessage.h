#ifndef DEVICE_MESSAGE_H
#define DEVICE_MESSAGE_H

#include <stdint.h>
#include <unistd.h>
#define DEVICE_MSG_MAX_SIZE (1 << 18) // 256k
// type: 1 byte; length: 4 bytes
#define DEVICE_MSG_TEXT_MAX_LENGTH (DEVICE_MSG_MAX_SIZE - 5)

enum DeviceMessageType { DEVICE_MSG_TYPE_CLIPBOARD };
class DeviceMessage {
private:
  /* data */
  enum DeviceMessageType type;
  struct {
    char *text; // owned, to be freed by SDL_free()
  } clipboard;

  uint32_t read32be(const uint8_t *buffer);

public:
  DeviceMessage(/* args */);
  ~DeviceMessage();

  ssize_t deserialize(const unsigned char *buf, size_t len);
  void destroy();
};

#endif
