#ifndef STELLA_H
#define STELLA_H

#include "QtPlugin.h"
#include "Settings.h"
#include "StellaPlugin.h"
#include "adb_wrapper.h"
#include "devicewatcher.h"
#include "fastboot_wrapper.h"
#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QDirIterator>
#include <QtCore/QMetaProperty>
#include <QtCore/QObject>
#include <QtCore/QPluginLoader>
#include <QtQml/QQmlApplicationEngine>
#include <QtQml/QQmlContext>

class PluginManager;
class Settings;

static Stella *instance;

class Stella : public QObject {
  Q_OBJECT
private:
  /* data */
  QQmlApplicationEngine *engine;
  QString m_appDir;
  QString m_dataDir;
  QString m_tempDir;
  QString m_minicapDir;
  QList<StellaPluginInterface *> plugins;
  AdbWrapper *m_adb;
  FastbootWrapper *m_fastboot;
  DeviceWatcher *m_watcher;
  AndroidDeviceList devices;
  PluginManager *pluginManager;
  Settings *settings;
#ifdef STELLA_DEBUG
  QProcess *daemonProcess;
  void runDaemon();
#endif
  void initPyFunctions();

public:
  

  Stella(QQmlApplicationEngine *engine);
  ~Stella();

  static Stella *getInstance();

  Q_SIGNAL void onConsoleMessage(const QString message);

  Q_PROPERTY(QString appDir READ appDir WRITE setAppDir);
  Q_PROPERTY(QString dataDir READ dataDir CONSTANT);
  Q_PROPERTY(QString tempDir READ tempDir WRITE setTempDir);
  Q_PROPERTY(QVariant pluginsModel READ pluginsModel);
  Q_PROPERTY(QVariant adb READ adb CONSTANT);
  Q_PROPERTY(QVariant watcher READ watcher CONSTANT);
  Q_PROPERTY(QVariant minicapDir READ minicapDir CONSTANT);

  void setAppDir(QString appDir);
  QString appDir() const;
  QString dataDir() const;
  void setTempDir(QString tempDir);
  QString tempDir() const;
  QString minicapDir() const;
  QVariant pluginsModel() const;

  void loadPlugins();

  void createWindow(QUrl url);
  void registerObject(QString name, QObject *obj);
  // void *Py_registerObject(void *self, void *args);

  void killAllProcess();
  QVariant adb() const;
  QVariant fastboot() const;
  QVariant watcher() const;

  AdbWrapper *adbInstance() const;
  FastbootWrapper *fastbootInstance() const;
  DeviceWatcher *watcherInstance() const;

public Q_SLOTS:
  void mkdir(const QString &path, QJSValue cb = QJSValue::UndefinedValue);
  void rmdir(const QString &path, QJSValue cb = QJSValue::UndefinedValue);
  void rm(const QString &path, QJSValue cb = QJSValue::UndefinedValue);
  QString filename(const QString &path);
  QString basename(const QString &path);
  QJSValue isDirectory(const QString &path);
  QStringList listDirectory(const QString &path);
  QJSValue exists(const QString &path);

  QJSValue getProp(const QString &serial, const QString &key,
                   QJSValue cb = QJSValue::UndefinedValue);

  QList<QVariant> getDevices();
};

void *PyInit_stella(void *);

#endif
