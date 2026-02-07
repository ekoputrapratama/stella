#include "LocalSocket.h"
#include <QDebug>
#include <QTimer>
#include <iostream>

static socket_t connect_and_read_byte(uint16_t port) {
  socket_t socket = net_connect(IPV4_LOCALHOST, port);
  if (socket == INVALID_SOCKET) {
    return INVALID_SOCKET;
  }

  char byte;
  // the connection may succeed even if the server behind the "adb tunnel"
  // is not listening, so read one byte to detect a working connection
  if (net_recv(socket, &byte, 1) != 1) {
    // the server is not listening yet behind the adb tunnel
    net_close(socket);
    return INVALID_SOCKET;
  }
  return socket;
}

LocalSocket::LocalSocket(QObject *parent) : QThread(parent) {
  setObjectName("LocalSocket");
}

LocalSocket::~LocalSocket() {
}

void LocalSocket::run() {
  mutex.lock();
  buffer = new std::vector<char>(2048);
  size_t offset = 0;
  mutex.unlock();

  for (;;) {
    char buff[512];
    // size_t head = 0;
    mutex.lock();
    ssize_t r = net_recv(m_descriptor, buffer->data() + offset, buffer->size() - offset);

    if (r < 0) {
      qDebug() << "socket stopped";
      break;
    }

    if (r > 0) {
      buffer->push_back('\0');
      buffer->resize(offset + r);
      emit readyRead();
      // qDebug() << "clearing buffer";
      buffer = new std::vector<char>(1024);
    }

    if (r == 0) {
      qDebug() << "no data received" << sizeof(buff);
    }
    mutex.unlock();
    if (shouldTerminate) {
      break;
    }
  }
}

QByteArray LocalSocket::readAll() {
  // QMutexLocker locker(&mutex);
  return QByteArray::fromRawData((char *)buffer->data(), buffer->size());
}

void LocalSocket::connect(uint16_t port, uint32_t maxAttempts) {
  this->m_port = port;
  this->attempts = maxAttempts;

  uint32_t attempts = maxAttempts;
  uint32_t delay = 100;
  do {
    qDebug() << "Remaining connection attempts: " << attempts;
    socket_t socket = connect_and_read_byte(port);
    if (socket != INVALID_SOCKET) {
      m_descriptor = socket;

      // start(QThread::Priority::NormalPriority);
      QTimer::singleShot(100, this, [this]() {
        QMetaObject::invokeMethod(this, "connected");
      });
      break;
    }
    if (attempts) {
      msleep(delay);
    }
  } while (--attempts > 0);
}

void LocalSocket::connectToHost() {
}

void LocalSocket::abort() {
}

void LocalSocket::close() {
  shouldTerminate = true;
  net_close(m_descriptor);
}

bool LocalSocket::write(unsigned char *data, size_t length) {
  int w = net_send_all(m_descriptor, data, length);
  return w == length;
}

socket_t LocalSocket::socketDescriptor() const {
  return m_descriptor;
}

void LocalSocket::setSocketDescriptor(socket_t descriptor) {
  this->m_descriptor = descriptor;
}
