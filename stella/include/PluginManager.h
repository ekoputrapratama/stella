#ifndef STELLA_PLUGIN_MANAGER_H
#define STELLA_PLUGIN_MANAGER_H

#include "stella.h"
#include <QDir>
#include <QObject>

QT_BEGIN_NAMESPACE
class QSettings;
class QProcess;
QT_END_NAMESPACE

class PluginModel;
class StellaPluginInterface;
class Settings;
class PluginManager : public QObject {
private:
  /* data */
  PluginModel *m_pluginsModel;
  QList<StellaPluginInterface *> plugins;
  QStringList pluginsDir;
  Settings *settings;
  Stella *stella;
#ifdef STELLA_DEBUG
  QList<QProcess *> daemons;
  void runDaemon(QDir pluginDir, QSettings *info);
#endif
  void initPyModule();
  void loadPythonPlugin(QDir pluginDir, QSettings *info);
  void loadQtPlugin(QDir pluginDir, QSettings *info);
  void loadPlugins();

public:
  PluginManager(QObject *stella, Settings *s);
  ~PluginManager();

  PluginModel *pluginsModel() const;
};

#endif
