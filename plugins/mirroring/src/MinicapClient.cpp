#include "MinicapClient.h"

#include <QDir>
#include <QQmlEngine>
#include <QQmlFileSelector>
#include <QtNetwork>

using namespace std;

QList<MinicapClient *> MinicapClient::mInstances = {};
MinicapClient::MinicapClient(QObject *parent) : QObject(parent), socket(new QTcpSocket(parent)) {
  qDebug("creating tcp socket");
  QObject::connect(socket, &QTcpSocket::readyRead, this, &MinicapClient::readable);
  QObject::connect(socket, &QTcpSocket::connected, this, &MinicapClient::connected);
  QObject::connect(socket, &QTcpSocket::errorOccurred, this, &MinicapClient::displayError);

  MinicapClient::mInstances.append(this);
}

MinicapClient::~MinicapClient() {

  if (socket->isOpen()) {
    socket->close();
    socket->disconnectFromHost();
  }
  socket = nullptr;
}

void MinicapClient::stop() {
  QObject::disconnect(this, SLOT(readable()));
  QObject::disconnect(this, SLOT(displayError(QTcpSocket::SocketError)));
  if (socket->isOpen()) {
    socket->close();
    socket->disconnectFromHost();
  }
}

void MinicapClient::readable() {
  // qDebug() << "on socket readable";
  QByteArray chunk = socket->readAll();
  // qDebug() << "chunk length" << chunk.size();
  int len = chunk.size();
  for (int cursor = 0; cursor < len;) {
    // qDebug() << "current cursor" << cursor;
    if (readBannerBytes < bannerLength) {
      switch (readBannerBytes) {
        case 0: {
          unsigned char v = (unsigned char)chunk.at(cursor);
          bannerVersion = v;

          break;
        }
        case 1: {
          // length
          unsigned char v = (unsigned char)chunk.at(cursor);
          bannerLength = v;
          break;
        }
        case 2:
        case 3:
        case 4:
        case 5: {
          // pid
          unsigned char v = (unsigned char)chunk.at(cursor);
          pid += (v << ((readBannerBytes - 2) * 8)) >> 0;
          break;
        }
        case 6:
        case 7:
        case 8:
        case 9: {
          // real width
          unsigned char v = (unsigned char)chunk.at(cursor);
          setDeviceWidth(mRealWidth + (v << ((readBannerBytes - 6) * 8)) >> 0);
          break;
        }
        case 10:
        case 11:
        case 12:
        case 13: {
          // real height
          unsigned char v = (unsigned char)chunk.at(cursor);
          setDeviceHeight(mRealHeight + (v << ((readBannerBytes - 6) * 8)) >> 0);
          break;
        }
        case 14:
        case 15:
        case 16:
        case 17: {
          // virtual width
          unsigned char v = (unsigned char)chunk.at(cursor);
          setVirtualWidth(mVirtualWidth + (v << ((readBannerBytes - 14) * 8)) >> 0);
          break;
        }
        case 18:
        case 19:
        case 20:
        case 21: {
          // virtual height
          unsigned char v = (unsigned char)chunk.at(cursor);
          setVirtualHeight(mVirtualHeight += (v << ((readBannerBytes - 18) * 8)) >> 0);
          break;
        }
        case 22: {
          // orientation
          unsigned char v = (unsigned char)chunk.at(cursor);
          orientation += v * 90;

          break;
        }
        case 23: {
          // quirks
          unsigned char v = (unsigned char)chunk.at(cursor);
          quirks = v;

          break;
        }
      }
      cursor += 1;
      readBannerBytes += 1;

      if (readBannerBytes == bannerLength) {
        onSizeChanged(deviceWidth(), deviceHeight(), virtualWidth(), virtualHeight());
        MinicapBanner *banner = new MinicapBanner(bannerVersion, bannerLength);
        banner->realHeight = mRealHeight;
        banner->realWidth = mRealWidth;
        banner->virtualHeight = mVirtualHeight;
        banner->virtualWidth = mVirtualWidth;
        banner->pid = pid;
        banner->orientation = orientation;
        banner->quirks = quirks;
        onBannerReceived(banner);
      }
    } else if (readFrameBytes < 4) {
      unsigned char v = chunk.at(cursor);
      frameBodyLength += (v << (readFrameBytes * 8)) >> 0;
      int t = (((unsigned char)chunk.at(cursor)) << (readFrameBytes * 8)) >> 0;
      cursor += 1;
      readFrameBytes += 1;

    } else {
      if (len - cursor >= frameBodyLength) {

        QByteArray bodyChunk = chunk.mid(cursor, cursor + frameBodyLength);
        vector<unsigned char> body(bodyChunk.begin(), bodyChunk.end());
        frameBody.insert(frameBody.end(), body.begin(), body.end());

        // Sanity check for JPG header, only here for debugging purposes.
        if (frameBody[0] != 0xFF || frameBody[1] != 0xD8) {
          qWarning("Frame body does not start with JPG header");
          break;
        }

        QByteArray data
            = QByteArray(reinterpret_cast<const char *>(frameBody.data()), frameBody.size());

        onDeviceImage(data);

        cursor += frameBodyLength;
        frameBodyLength = readFrameBytes = 0;
        frameBody.clear();
      } else {
        QByteArray bodyChunk = chunk.mid(cursor, len);
        vector<unsigned char> body(bodyChunk.begin(), bodyChunk.end());
        frameBody.insert(frameBody.end(), body.begin(), body.end());

        frameBodyLength -= len - cursor;
        readFrameBytes += len - cursor;
        cursor = len;
      }
    }
  }
}

void MinicapClient::connected() {
  isConnected = true;
}

void MinicapClient::displayError(QTcpSocket::SocketError socketError) {
  qCritical() << socketError;
  if (socketError == QTcpSocket::RemoteHostClosedError
      || socketError == QTcpSocket::ConnectionRefusedError) {

    int port = socket->localPort();
    socket->reset();

    if (connectionAttempt > 0) {
      qInfo() << "Reconnecting...";
      QTimer::singleShot(100, this, SLOT(reconnect()));
    }
  }

  // QMessageBox::information(
  //     nullptr, tr("Local Fortune Client"),
  //     tr("The following error occurred: %1.").arg(socket->errorString()));
}

void MinicapClient::reconnect() {
  socket->connectToHost(host, port);
  socket->waitForConnected();
  --connectionAttempt;
}

void MinicapClient::connect(QString host, int port, int maxAttempt) {
  this->connectionAttempt = maxAttempt;
  this->host = host;
  this->port = port;
  socket->connectToHost(host, port);
}

QList<MinicapClient *> MinicapClient::getInstances() {
  return MinicapClient::mInstances;
}

void MinicapClient::setDeviceWidth(int width) {
  mRealWidth = width;
}
int MinicapClient::deviceWidth() const {
  return mRealWidth;
};
void MinicapClient::setDeviceHeight(int n) {
  mRealHeight = n;
}
int MinicapClient::deviceHeight() const {
  return mRealHeight;
};

void MinicapClient::setVirtualHeight(int n) {
  mVirtualHeight = n;
}
int MinicapClient::virtualHeight() const {
  return mVirtualHeight;
};
void MinicapClient::setVirtualWidth(int n) {
  mVirtualWidth = n;
}
int MinicapClient::virtualWidth() const {
  return mVirtualWidth;
};
