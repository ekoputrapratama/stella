#ifndef DEVICE_CONTROLLER_H
#define DEVICE_CONTROLLER_H

#include <QMutex>
#include <QObject>
#include <QTcpSocket>
#include <QThread>
#include <QWaitCondition>

#include "ControlMessage.h"
#include "DeviceMessage.h"

class LocalSocket;

class DeviceController : public QThread {
  Q_OBJECT
  typedef QThread Base;

private:
  /* data */
  QMutex mutex;
  bool restart;
  bool shouldTerminate = false;

  QWaitCondition condition;
  LocalSocket *socket = nullptr;
  qintptr socketDescriptor;
  QList<ControlMessage *> queue;
  QString host;
  int port;
  int connectionAttempt;

public:
  DeviceController(QObject *parent = nullptr);
  ~DeviceController();

  bool pushMessage(ControlMessage *msg);
  void stop();

  bool isConnected = false;

protected:
  void run() override;

Q_SIGNALS:
  void onConnected();

public Q_SLOTS:
  void connect(QString host, int port, int maxAttempt = 100);

private Q_SLOTS:
  void displayError(QAbstractSocket::SocketError socketError);
  void connected();
  void readable();
};

#endif
