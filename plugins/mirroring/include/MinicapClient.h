
#ifndef CLIENT_H
#define CLIENT_H

#include <QDataStream>
#include <QQuickItem>
#include <QtNetwork/QTcpSocket>
#include <vector>

using namespace std;

class MinicapBanner : public QObject {
  Q_OBJECT
public:
  MinicapBanner(unsigned char version, unsigned char length) {
    this->version = version;
    this->length = length;
  };
  unsigned char version;
  unsigned char length = 2;
  pid_t pid = 0;
  int realWidth = 0;
  int realHeight = 0;
  int virtualWidth = 0;
  int virtualHeight = 0;
  int orientation = 0;
  int quirks = 0;
};

class MinicapClient : public QObject {
  Q_OBJECT

public:
  explicit MinicapClient(QObject *parent = nullptr);
  ~MinicapClient();
  Q_SIGNAL void onDeviceImage(QByteArray img);
  Q_SIGNAL void onSizeChanged(int width, int height, int virtualWidth, int virtualHeight);
  Q_SIGNAL void onBannerReceived(MinicapBanner *banner);

  Q_PROPERTY(int deviceWidth READ deviceWidth WRITE setDeviceWidth);
  Q_PROPERTY(int deviceHeight READ deviceHeight WRITE setDeviceHeight);
  Q_PROPERTY(int virtualWidth READ virtualWidth WRITE setVirtualWidth);
  Q_PROPERTY(int virtualHeight READ virtualHeight WRITE setVirtualHeight);

  void setDeviceWidth(int width);
  int deviceWidth() const;
  void setDeviceHeight(int height);
  int deviceHeight() const;
  void setVirtualHeight(int height);
  int virtualHeight() const;
  void setVirtualWidth(int width);
  int virtualWidth() const;

public Q_SLOTS:
  void connect(QString host, int port, int maxAttempt = 100);
  void stop();
  void connected();
  static QList<MinicapClient *> getInstances();

private Q_SLOTS:
  void readable();
  void reconnect();
  void displayError(QTcpSocket::SocketError socketError);

private:
  static QList<MinicapClient *> mInstances;
  QTcpSocket *socket;
  bool isConnected = false;
  int readBannerBytes = 0;
  unsigned char bannerLength = 2;
  int readFrameBytes = 0;
  int frameBodyLength = 0;
  vector<unsigned char> frameBody;
  int mRealWidth = 0;
  int mRealHeight = 0;
  int mVirtualWidth = 0;
  int mVirtualHeight = 0;
  int orientation = 0;
  int quirks = 0;
  unsigned char bannerVersion;
  pid_t pid = 0;
  QString host;
  int port;
  int connectionAttempt;
};

#endif
