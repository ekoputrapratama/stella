#include "PluginManager.h"
#include "PluginModel.h"
#include "QtPlugin.h"
#include "PythonPlugin.h"
#include "Settings.h"
#include <qobject.h>
#undef slots
#include <Python.h>
#define slots Q_SLOTS
#include <QCoreApplication>
#include <QDebug>
#include <QDirIterator>
#include <QPluginLoader>
#include <QProcess>
#include <QSettings>

PluginManager::PluginManager(QObject *stella, Settings *s) : QObject(stella), settings(s) {

  QString path = QCoreApplication::applicationDirPath();
  qDebug() << "Loading Plugins...";
  
  m_pluginsModel = new PluginModel((Stella *) Stella::getInstance());
  qDebug() << "settings has key pluginsDir" << settings->has("pluginsDir");
  if (!settings->has("pluginsDir")) {
#ifdef Q_OS_WIN32
    QDir pluginDir = QDir(path);
    pluginDir.cd("plugins");
    pluginsDir.append(pluginDir.absolutePath());
#elif defined(Q_OS_LINUX)
    // user library path
    QDir userLibPath = QDir::home();
    userLibPath.cd(".local");
    userLibPath.cd("lib");
    // local path inside appimage
    QDir localPath = QDir(QCoreApplication::applicationDirPath());
    localPath.cdUp();
    localPath.cd("lib");

#if defined(STELLA_DEBUG)
    QDir subdirPath = QDir(QCoreApplication::applicationDirPath());
    subdirPath.cdUp();
#endif

    const QStringList paths = { "/usr/lib", userLibPath.absolutePath(), localPath.absolutePath(),
                                subdirPath.absolutePath() };
    const QStringList libPaths = QCoreApplication::libraryPaths();

    for (int i = 0; i < libPaths.size(); i++) {
      QDir pluginDir = QDir(libPaths[i]);
      if (pluginDir.cd("stella") && pluginDir.cd("plugins"))
        pluginsDir.append(pluginDir.absolutePath());
    }

    for (int i = 0; i < paths.size(); i++) {
      QDir pluginDir = QDir(paths[i]);
      if (pluginDir.cd("stella") && pluginDir.cd("plugins"))
        pluginsDir.append(pluginDir.absolutePath());
    }

#endif
  } else {
    QVariantList list = settings->get("pluginsDir").toList();
    foreach (QVariant val, list) { pluginsDir.append(val.toString()); }
  }
  loadPlugins();
}

PluginManager::~PluginManager() {
}

PluginModel *PluginManager::pluginsModel() const {
  return m_pluginsModel;
}

void PluginManager::loadPlugins() {
  qDebug() << "plugins directory" << pluginsDir;

  

  QStringList exts = { "*.plugin" };

  for (int i = 0; i < pluginsDir.size(); i++) {
    QDirIterator it(pluginsDir[i], exts, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
      const QString fileName = it.next();
      qDebug() << "checking plugin  identities " << fileName;
      QSettings *settings = new QSettings(fileName, QSettings::Format::IniFormat);
      QString type = settings->value("Plugin/Type").toString();
      QString daemon = settings->value("Plugin/Daemon").toString();

      QFileInfo info(fileName);
      QDir directory = info.absoluteDir();

      // some plugin may require daemon to run first,
      // so we run it first if it's in debug mode
#if defined(STELLA_DEBUG)
      // if (!daemon.isNull() & !daemon.isEmpty()) {
      //   runDaemon(directory, settings);
      // }
#endif
      if (type.toLower() == "qt") {
        // const auto staticInstances = QPluginLoader::staticInstances();
        // for (QObject *plugin : staticInstances) {
        //   auto iPlugin = qobject_cast<StellaPluginInterface *>(plugin);
        //   if (iPlugin) {
        //     plugins.append(iPlugin);
        //     m_pluginsModel->addPlugin(new QtPlugin(iPlugin));
        //   }
        // }

        qDebug() << "loadQtPlugin " << directory.absolutePath();
        loadQtPlugin(directory, settings);
      } else if (type.toLower() == "python") {
        qDebug() << "loadPythonPlugin " << directory.absolutePath();
        loadPythonPlugin(directory, settings);
      }
    }
  }
}

void PluginManager::loadQtPlugin(QDir pluginDir, QSettings *info) {
  QString module = info->value("Plugin/Module").toString();

  QPluginLoader loader(pluginDir.filePath(module));
  QObject *plugin = loader.instance();
  if (plugin != nullptr) {
    auto iPlugin = qobject_cast<StellaPluginInterface *>(plugin);

    if (iPlugin) {
      QString key = QString("plugins.").append(module);
      if (!settings->has(key)) {
        QObject obj = QObject();
        obj.setProperty("name", iPlugin->name());
        obj.setProperty("rootDir", pluginDir.absolutePath());
        settings->set(key, obj);
      }
      qDebug() << "adding plugin" << iPlugin->name();
      plugins.append(iPlugin);
      m_pluginsModel->addPlugin(new QtPlugin(iPlugin));
    }
  }
}
void PluginManager::loadPythonPlugin(QDir pluginDir, QSettings *info) {
  QString module = info->value("Plugin/Module").toString();
  QString name = info->value("Plugin/Name").toString();

  PyObject *pName, *pModule;

  pName = PyUnicode_FromString(name.toStdString().c_str());

  PyRun_SimpleString("import sys");
  PyRun_SimpleString(
      QString("sys.path.append(\"%1\")").arg(pluginDir.absolutePath()).toStdString().c_str());

  pModule = PyImport_Import(pName);
  Py_DECREF(pName);

  if (pModule != NULL) {
    qDebug() << "python module loaded";
    PythonPlugin *plugin = new PythonPlugin(pModule);
    plugins.append(plugin);
    m_pluginsModel->append(plugin);
  } else {
    qDebug() << "cannot import plugin" << pluginDir.absolutePath();
    PyErr_Print();
  }
}

#if defined(STELLA_DEBUG)
void PluginManager::runDaemon(QDir pluginDir, QSettings *info) {
  QString module = info->value("Plugin/Module").toString();
  QString daemon = info->value("Plugin/Daemon").toString();
  QString type = info->value("Plugin/DaemonType").toString();

  QString name = module;
  if (module.lastIndexOf(".") > 0) {
    name = module.mid(0, module.length() - (module.length() - module.lastIndexOf(".")));
  }

  QString program = pluginDir.absoluteFilePath(daemon);
  qDebug() << "daemon name" << name;
  qDebug() << "running daemon" << program;

  QString key = QString("plugins.%1.%2").arg(name);
  if (!settings->has(key.arg("rootDir"))) {
    settings->set(key.arg("rootDir"), pluginDir.absolutePath());
  }

  if (!settings->has(key.arg("daemon.path"))) {
    settings->set(key.arg("daemon.path"), program);
  }

  if (!settings->has(key.arg("daemon.enabled"))) {
    settings->set(key.arg("daemon.enabled"), true);
  }

  settings->save();

  QProcess *process = new QProcess();
  process->setProcessChannelMode(QProcess::ForwardedChannels);

  process->start(program, QStringList());
  daemons.append(process);
}
#endif
