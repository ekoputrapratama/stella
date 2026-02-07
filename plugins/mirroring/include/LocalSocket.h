#ifndef LOCAL_SOCKET_H
#define LOCAL_SOCKET_H

#include "net.h"
#include <QMutex>
#include <QThread>
#include <QWaitCondition>

#define IPV4_LOCALHOST 0x7F000001

class LocalSocket : public QThread {
  Q_OBJECT
private:
  /* data */
  uint16_t m_port;
  uint32_t attempts;
  socket_t m_descriptor;
  uint32_t delay = 100;
  QMutex mutex;
  bool restart;
  bool shouldTerminate = false;
  QWaitCondition condition;
  std::vector<char> *buffer;

public:
  LocalSocket(QObject *parent = nullptr);
  ~LocalSocket();

  Q_PROPERTY(socket_t descriptor READ socketDescriptor WRITE setSocketDescriptor)

  void setSocketDescriptor(socket_t descriptor);
  socket_t socketDescriptor() const;

  void connect(uint16_t port, uint32_t maxAttempts = 100);
  void close();
  void abort();
  bool write(unsigned char *data, size_t length);
  QByteArray readAll();

private Q_SLOTS:
  void connectToHost();

protected:
  void run() override;

Q_SIGNALS:
  void connected();
  void readyRead();
};

#endif
