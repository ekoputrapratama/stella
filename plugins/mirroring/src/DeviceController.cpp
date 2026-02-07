#include "DeviceController.h"
#include "LocalSocket.h"

#include <QDataStream>
#include <QTcpSocket>
#include <QTimer>

DeviceController::DeviceController(QObject *parent) : QThread(parent), socket(new LocalSocket(0)) {
  QObject::connect(socket, &LocalSocket::readyRead, this, &DeviceController::readable);
  QObject::connect(socket, &LocalSocket::connected, this, &DeviceController::connected);
  // QObject::connect(socket, &QTcpSocket::errorOccurred, this, &DeviceController::displayError);
}

DeviceController::~DeviceController() {
  if (socket != nullptr) {
    socket->close();
  }

  socket = nullptr;
}

void DeviceController::run() {
  mutex.lock();
  qintptr descriptor = this->socketDescriptor;
  LocalSocket *sock = new LocalSocket();
  sock->setSocketDescriptor(descriptor);

  mutex.unlock();

  forever {
    while (queue.size() > 0) {
      mutex.lock();
      ControlMessage *msg = this->queue.takeFirst();
      unsigned char buffer[CONTROL_MSG_MAX_SIZE];
      int length = msg->serialize(buffer);

      qDebug() << "sending message with length" << length;

      sock->write(buffer, length);

      mutex.unlock();
    }

    mutex.lock();
    if (!restart)
      condition.wait(&mutex);
    restart = false;
    mutex.unlock();

    if (shouldTerminate) {
      sock->close();
      break;
    }
  }
}

void DeviceController::readable() {
  isConnected = true;
  qDebug() << "controller receive a message";
}

void DeviceController::connected() {

  isConnected = true;
  socketDescriptor = socket->socketDescriptor();

  QMutexLocker locker(&mutex);
  if (!isRunning()) {
    start(QThread::Priority::NormalPriority);
  } else {
    restart = true;
    condition.wakeOne();
  }

  emit onConnected();
}

bool DeviceController::pushMessage(ControlMessage *msg) {
  unsigned char buffer[CONTROL_MSG_MAX_SIZE];
  if (msg->serialize(buffer) == 0)
    return false;

  queue.append(msg);
  if (!isRunning()) {
    start(QThread::Priority::NormalPriority);
  } else {
    restart = true;
    condition.wakeAll();
  }

  return true;
}

void DeviceController::displayError(QTcpSocket::SocketError socketError) {
  qCritical() << "DeviceController error" << socketError;
  isConnected = false;
  socket->abort();
  socket->close();

  // QMessageBox::information(
  //     nullptr, tr("Local Fortune Client"),
  //     tr("The following error occurred: %1.").arg(socket->errorString()));
}

void DeviceController::connect(QString host, int port, int maxAttempt) {
  connectionAttempt = maxAttempt;
  this->host = host;
  this->port = port;
  socket->connect(port);
}

void DeviceController::stop() {
  qDebug() << "stopping controller";
  // QObject::disconnect(this, SLOT(readable()));
  QObject::disconnect(this, SLOT(connected()));
  shouldTerminate = true;
  condition.wakeAll();

  if (socket != nullptr) {
    socket->abort();
    socket->close();
    socket->deleteLater();
  }
}
