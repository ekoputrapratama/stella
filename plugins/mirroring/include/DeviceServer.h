#ifndef MINICAP_SERVER_H
#define MINICAP_SERVER_H

#include <QMutex>
#include <QProcess>
#include <QThread>
#include <QWaitCondition>

class Observer : public QObject {
  Q_OBJECT
public:
  Observer() {};
Q_SIGNALS:
  void onStarted();
};

class DeviceServer : public QThread {
  Q_OBJECT
  typedef QThread Base;

private:
  /* data */
  QString adbPath;
  QString apkPath;
  QString classPath;
  QString serial;
  QMutex mutex;
  QProcess *process;
  static Observer *observer;

  QWaitCondition condition;
  bool restart = false;
  bool shouldTerminate = false;

  int windowWidth;
  int windowHeight;

  const QString CONTROL_KEY = "Mirroring/devices/%1/control";
  const QString WIDTH_KEY = "Mirroring/devices/%1/width";
  const QString HEIGHT_KEY = "Mirroring/devices/%1/height";
  const QString COMPONENT_KEY = "Mirroring/server/component";

  void findApkPath();
  QString execCmd(QString program, QStringList args);

public:
  DeviceServer(QString adbPath);
  ~DeviceServer();

  void stop();
  void start(QString serial, int width, int height);

public Q_SLOTS:
  void errorFound(QProcess::ProcessError e, QProcess *p = nullptr);
  void printStdout();

protected:
  void run() override;

Q_SIGNALS:
  void onStarted();
};

#endif
