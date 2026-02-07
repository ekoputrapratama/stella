#ifndef DEVICE_VIEW_H
#define DEVICE_VIEW_H

#include <QImage>
#include <QPainter>
#include <QtQuick/QQuickPaintedItem>

#include "DeviceController.h"

class MinicapClient;
class DeviceServer;
class MinicapBanner;
class InputManager;

class DeviceView : public QQuickPaintedItem {
  Q_OBJECT
  QML_NAMED_ELEMENT(DeviceView)
  typedef QQuickPaintedItem Base;

public:
  explicit DeviceView(QQuickItem *parent = nullptr);
  ~DeviceView();

  Q_PROPERTY(QString serial READ serial WRITE setSerial);

  void setSerial(QString s);
  QString serial() const;

  void paint(QPainter *painter);
  // void touchEvent(QTouchEvent *event);
  bool eventFilter(QObject *obj, QEvent *event);
  Point convertWindowToFrameCoords(int x, int y);

  static QList<DeviceView *> getInstances();
  DeviceController *controller;
  struct Size frameSize;

private:
  static QList<DeviceView *> mInstances;
  const QString WIDTH_KEY = "Mirroring/devices/%1/width";
  const QString HEIGHT_KEY = "Mirroring/devices/%1/height";
  const QString CONTROL_KEY = "Mirroring/devices/%1/control";

  DeviceServer *server = nullptr;
  MinicapClient *mClient = nullptr;
  InputManager *inputManager = nullptr;
  MinicapBanner *banner;

  QString m_serial;
  QString adbPath;
  QString classPath;
  int controlPort;
  int minicapPort;
  int rotation = 0;

  int forward(QString from, QString to);
  int getDeviceWidth();
  int getDeviceHeight();
  void removeAllForwardSocket();
  void findAdbPath();
  void findApkPath();

  QString execCmd(QString program, QStringList args);
Q_SIGNALS:
  void onDeviceConnected();
  void onDeviceSizeChanged();
public Q_SLOTS:
  // void onSizeChanged();
  void connectToDevice(QString serial);
  void onBannerReceived(MinicapBanner *banner);
  void onImageReceived(QByteArray buffer);
  void connectToMinicap();
  void connectToControlServer();

protected:
  void componentComplete();
  QImage blankImage = QImage::fromData(
      QByteArray::fromBase64("R0lGODlhAQABAAAAACH5BAEKAAEALAAAAAABAAEAAAICTAEAOw=="), "GIF");
  QImage m_image;
};

#endif
