#include "DeviceServer.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QProcessEnvironment>
#include <QSettings>
#include <QThread>

Observer *DeviceServer::observer = new Observer();

DeviceServer::DeviceServer(QString adbPath) : QThread(0) {
  this->adbPath = adbPath;
  findApkPath();
  QObject::connect(observer, &Observer::onStarted, this, &DeviceServer::onStarted);
}

DeviceServer::~DeviceServer() {
  qDebug() << "server destroyed";
  // process->close();
  // process->terminate();
  // process->kill();
  // process->thread()->quit();
  process->deleteLater();
}

QString DeviceServer::execCmd(QString program, QStringList args) {
  QProcess *proc = new QProcess();
  QProcess::connect(proc, &QProcess::errorOccurred, proc, [this, proc](QProcess::ProcessError e) {
    errorFound(e, proc);
  });

  proc->start(program, args);
  proc->waitForFinished();

  QString output(proc->readAllStandardOutput());

  proc->close();
  proc->deleteLater();
  return output;
}

void DeviceServer::findApkPath() {
  QSettings settings("Stella", "Stella");
  QString packageName = settings.value("Mirroring/server/packageName").toString();

  QStringList args = { "-c" };

#ifdef Q_OS_LINUX
  args.append(QString("adb shell pm list packages -3 -f  | grep %1 | "
                      "cut -d: -f 2 | cut -d= -f 1")
                  .arg(packageName));
#endif

  QString output = execCmd("sh", args);
  QStringList lines = output.split(QRegExp("\n|\r|\r\n"), Qt::SkipEmptyParts);

  if (lines.size() > 0) {
    classPath = lines[0];
    qDebug() << "device apk path" << classPath;
  } else {
    // stella for android is not installed try to install and retry
  }
}

void DeviceServer::start(QString serial, int width, int height) {
  QMutexLocker locker(&mutex);

  this->serial = serial;
  this->windowWidth = width;
  this->windowHeight = height;

  if (!isRunning()) {
    Base::start(QThread::Priority::HighPriority);
  } else {
    restart = true;
    condition.wakeOne();
  }
}

void DeviceServer::stop() {

  if (isRunning()) {
    process->terminate();
    shouldTerminate = true;
    condition.wakeAll();
  }

  QObject::disconnect(this, SLOT(errorFound(QProcess::ProcessError, QProcess *)));
}

void DeviceServer::run() {

  if (!serial.isNull() && !serial.isEmpty()) {
    mutex.lock();
    QString adb = this->adbPath;
    QString serial = this->serial;
    QSettings settings("Stella", "Stella");
    QString clientVarsion = settings.value("version", "1.0").toString();
    QString componentName = settings.value(COMPONENT_KEY).toString();
    bool control = settings.value(CONTROL_KEY.arg(serial), true).toBool();
    QString width = settings.value(WIDTH_KEY.arg(serial), windowWidth).toString();
    QString height = settings.value(HEIGHT_KEY.arg(serial), windowHeight).toString();

    process = new QProcess();
    process->setProcessChannelMode(QProcess::ProcessChannelMode::ForwardedChannels);

    mutex.unlock();
    QStringList args = { "-s",
                         serial,
                         "shell",
                         "app_process",
                         "-Djava.class.path=" + classPath,
                         "/data/local/tmp",
                         componentName,
                         "--client-version",
                         clientVarsion,
                         "--virtual-width",
                         width,
                         "--virtual-height",
                         height,
                         control ? "--control" : "" };

    qDebug() << "starting minicap server" << adb << args;

    process->start(adb, args);

    // prevent this thread from stopping
    mutex.lock();
    condition.wait(&mutex);
    mutex.unlock();
  }
}

void DeviceServer::printStdout() {
  qDebug() << "printStdout";
  QString output = process->readAllStandardOutput();
  QStringList lines = output.split(QRegExp("\n|\r\n"), Qt::SkipEmptyParts);

  foreach (QString line, lines) { qDebug() << line; }
}

void DeviceServer::errorFound(QProcess::ProcessError e, QProcess *p) {
  qCritical() << "error when executing command" << e;
  // qCritical() << p->readAllStandardOutput();
  if (p != nullptr)
    qCritical() << p->readAllStandardError();
  else
    qCritical() << process->readAllStandardError();
}
