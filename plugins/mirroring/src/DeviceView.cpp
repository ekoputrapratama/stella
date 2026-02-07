#include "DeviceView.h"

#include "DeviceServer.h"
#include "InputManager.h"
#include "MinicapClient.h"
#include <QGuiApplication>

QList<DeviceView *> DeviceView::mInstances = {};
QList<DeviceView *> DeviceView::getInstances() {
  return mInstances;
}
DeviceView::DeviceView(QQuickItem *parent) {
  setFlags(flags() | ItemAcceptsDrops | ItemAcceptsInputMethod);
  setRenderTarget(RenderTarget::FramebufferObject);
  setObjectName("DeviceView");

  // setAcceptTouchEvents(true);
  // setAcceptedMouseButtons(Qt::MouseButton::AllButtons);

  QGuiApplication::instance()->installEventFilter(this);
  findAdbPath();

  mClient = new MinicapClient();
  controller = new DeviceController(this);
  inputManager = new InputManager(controller, this);
  server = new DeviceServer(adbPath);

  QObject::connect(mClient, &MinicapClient::onDeviceImage, this, &DeviceView::onImageReceived);
  QObject::connect(mClient, &MinicapClient::onBannerReceived, this, &DeviceView::onBannerReceived);

  mInstances.append(this);

  frameSize.height = height();
  frameSize.width = width();
  // QShortcut *shortcut = new QShortcut();
}

DeviceView::~DeviceView() {
  QObject::disconnect(this, SLOT(onImageReceived(QByteArray)));
  QObject::disconnect(this, SLOT(onBannerReceived(MinicapBanner *)));
  mClient->stop();
  mClient->deleteLater();
  server->stop();
  server->deleteLater();
  controller->stop();
  controller->deleteLater();
  mInstances.removeOne(this);
}

void DeviceView::findApkPath() {
  QSettings settings("Stella", "Stella");
  QString packageName = settings.value("Mirroring/server/packageName").toString();

  QStringList args = { "-c" };

#ifdef Q_OS_LINUX
  args.append(QString("adb shell pm list packages -3 -f  | grep %1 | "
                      "cut -d: -f 2 | cut -d= -f 1")
                  .arg(packageName));
#endif

  QString output = execCmd("sh", args);
  QStringList lines = output.split(QRegExp("\n|\r|\r\n"), Qt::SplitBehaviorFlags::SkipEmptyParts);

  if (lines.size() > 0) {
    classPath = lines[0];
    qDebug() << "device apk path" << classPath;
  } else {
    // stella for android is not installed try to install and retry
  }
}

void DeviceView::findAdbPath() {
  QStringList paths;

  QString appPath = QCoreApplication::applicationDirPath();
  paths.append(appPath);

#ifdef Q_OS_LINUX
  paths.append("/usr/bin");
  paths.append("/usr/local/bin");

  QDir userPath = QDir::home();
  if (userPath.cd(".local") && userPath.cd("bin"))
    paths.append(userPath.absolutePath());

  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  if (env.contains("ANDROID_SDK_HOME")) {
    QDir sdkPath = QDir(env.value("ANDROID_SDK_HOME"));

    if (sdkPath.cd("platform-tools"))
      paths.append(sdkPath.absolutePath());
  }
#endif

  foreach (QString p, paths) {
    QString path;
#ifdef Q_OS_LINUX
    path = QDir(p).filePath("adb");
#elif defined(Q_OS_WIN32)
    path = QDir(path).filePath("adb.exe");
#endif

    QFileInfo file(path);
    if (file.exists()) {
      adbPath = file.absoluteFilePath();
      qDebug() << "found adb path" << adbPath;
      break;
    }
  }
}

bool DeviceView::eventFilter(QObject *obj, QEvent *event) {
  // if (!controller || !controller->isConnected)
  //   return false;

  switch (event->type()) {
    case QEvent::MouseMove: {
      QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
      if (inputManager != nullptr) {
        inputManager->processMouseMotion(mouseEvent);
        event->accept();
      }
      return true;
    }
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease: {
      QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
      if (inputManager != nullptr) {
        inputManager->processMouseButton(mouseEvent);
        event->accept();
      }
      return true;
    }
    case QEvent::KeyPress:
    case QEvent::KeyRelease: {
      QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
      if (inputManager != nullptr) {
        inputManager->processKey(keyEvent);
        event->accept();
      }
      return true;
    }
    case QEvent::Wheel: {
      QWheelEvent *wheelEvent = static_cast<QWheelEvent *>(event);
      if (inputManager != nullptr) {
        inputManager->processMouseWheel(wheelEvent);
        event->accept();
      }
      return true;
    }

    default:
      break;
  }

  return false;
}

void DeviceView::componentComplete() {
  Base::componentComplete();
}

void DeviceView::paint(QPainter *painter) {
  painter->drawImage(this->boundingRect(), m_image);
}

void DeviceView::onBannerReceived(MinicapBanner *banner) {
  setWidth(banner->virtualWidth);
  setHeight(banner->virtualHeight);
  frameSize.height = banner->virtualHeight;
  frameSize.width = banner->virtualWidth;

  emit onDeviceConnected();
}

void DeviceView::onImageReceived(QByteArray buffer) {
  QImage img = QImage::fromData(buffer, "JPEG");
  m_image = img; // does shallow copy of image data
  update(); // triggers actual update
}

void DeviceView::connectToDevice(QString serial) {
  qDebug() << "connecting to device with serial" << serial;
  this->m_serial = serial;

  QSettings settings("Stella", "Stella");

  int width = settings.value(WIDTH_KEY.arg(serial), 0).toInt();
  int height = settings.value(HEIGHT_KEY.arg(serial), 0).toInt();
  bool control = settings.value(CONTROL_KEY.arg(serial), true).toBool();

  if (width == 0 || height == 0) {
    width = getDeviceWidth();
    height = getDeviceHeight();
  }

  removeAllForwardSocket();
  minicapPort = forward("localabstract:minicap", "tcp:0");

  server->start(serial, width, height);

  if (control) {
    controlPort = forward("tcp:7643", "tcp:0");
    QObject::connect(controller, &DeviceController::onConnected, this, [this]() {
      qDebug() << "starting minicap client";
      mClient->connect("localhost", minicapPort);
    });
    controller->connect("localhost", controlPort);
  } else {
    mClient->connect("localhost", minicapPort);
  }
}

void DeviceView::connectToControlServer() {
  controller->connect("localhost", controlPort);
}

void DeviceView::connectToMinicap() {
  mClient->connect("localhost", minicapPort);
}

void DeviceView::removeAllForwardSocket() {
  if (serial().isEmpty()) {
    qCritical("cannot remove forward socket without device serial");
    return;
  }

  QStringList args = { "-s", serial(), "forward", "--remove-all" };

  execCmd(adbPath, args);
}

int DeviceView::getDeviceWidth() {
  // clang-format off
  QStringList args = { 
    "-c",
    adbPath + " -s " + m_serial + " shell dumpsys window | grep -Eo 'init=[0-9]+x[0-9]+' | head -1 | cut -d= -f 2"
  };
  // clang-format on
  QString result = execCmd("/usr/bin/sh", args).replace(QRegExp("\n|\r|\r\n"), "");
  if (result.isEmpty() || result.isNull()) {
    // clang-format off
    QStringList newArgs = { 
      "-c", 
      adbPath + " -s " + m_serial + " shell dumpsys window | grep -Eo 'DisplayWidth=[0-9]+' | head -1 | cut -d= -f 2"
    };
    // clang-format on
    result = execCmd("sh", args).replace(QRegExp("\n|\r|\r\n"), "");
    return result.toInt();
  }

  int width = result.split("x")[0].toInt();

  return width;
}

int DeviceView::getDeviceHeight() {
  // clang-format off
  QStringList args = { 
    "-c",
    adbPath + " -s " + m_serial + " shell dumpsys window | grep -Eo 'init=[0-9]+x[0-9]+' | head -1 | cut -d= -f 2"
  };
  // clang-format on
  QString result = execCmd("/usr/bin/sh", args).replace(QRegExp("\n|\r|\r\n"), "");
  if (result.isEmpty() || result.isNull()) {
    // clang-format off
    QStringList newArgs = { 
      "-c", 
      adbPath + " -s " + m_serial + " shell dumpsys window | grep -Eo 'DisplayHeight=[0-9]+' | head -1 | cut -d= -f 2"
    };
    // clang-format on
    result = execCmd("sh", args).replace(QRegExp("\n|\r|\r\n"), "");
    return result.toInt();
  }

  int height = result.split("x")[1].toInt();

  return height;
}

// forward android device socket to local socket
int DeviceView::forward(QString from, QString to) {
  if (serial().isEmpty())
    qCritical("cannot do forwarding without device serial");

  qDebug() << "forward socket from" << from << "to" << to;
  QStringList args = { "-s", serial(), "forward", to, from };

  QString port = execCmd(adbPath, args).replace(QRegExp("\n|\r|\r\n"), "");
  qDebug() << "available port" << port;
  return port.toInt();
}

QString DeviceView::serial() const {
  return m_serial;
}

void DeviceView::setSerial(QString s) {
  m_serial = s;
}

QString DeviceView::execCmd(QString program, QStringList args) {
  QProcess *process = new QProcess();
  QProcess::connect(process, &QProcess::errorOccurred, process, [this](QProcess::ProcessError e) {
    qCritical() << e;
  });

  process->start(program, args);
  process->waitForFinished();

  QString output(process->readAllStandardOutput());

  process->close();
  process->deleteLater();
  return output;
}

Point DeviceView::convertWindowToFrameCoords(int x, int y) {

  struct Point result;
  switch (rotation) {
    case 0:
      result.x = x;
      result.y = y;
      break;
    case 1:
      result.x = height() - y;
      result.y = x;
      break;
    case 2:
      result.x = width() - x;
      result.y = height() - y;
      break;
    default:
      assert(rotation == 3);
      result.x = y;
      result.y = width() - x;
      break;
  }

  return result;
}
