#include "stella.h"
#include "PluginManager.h"
#include "PluginModel.h"
#include "PythonPlugin.h"
#include "StellaPlugin.h"
#undef slots
#include <Python.h>
#define slots Q_SLOTS
#include <iostream>


/**
 * Stella
 */

static PyObject *createWindow(PyObject *self, PyObject *args, PyObject *keywds) {
  std::cout << "createWindow called from python" << std::endl;

  char *url = strdup("url");
  static char *kwlist[] = { url, NULL };

  if (!PyArg_ParseTupleAndKeywords(args, keywds, "s", kwlist, &url))
    return NULL;

  qDebug() << "url" << url;
  Stella::getInstance()->createWindow(QUrl(QString::fromUtf8(url)));

  Py_RETURN_NONE;
}

static PyObject *registerObject(PyObject *self, PyObject *args, PyObject *keywds) {
  std::cout << "registerObject called from python" << std::endl;

  PyObject *dict = NULL;
  const char *name = "";
  static char *kwlist[] = { strdup("name"), strdup("object"), NULL };

  if (!PyArg_ParseTupleAndKeywords(args, keywds, "sO!", kwlist, &name, &PyDict_Type, &dict))
    return NULL;

  if (dict && PyDict_Check(dict)) {
    PyObject *pName = PyUnicode_FromString("fun");
    PyObject *func = PyDict_GetItem(dict, pName);

    if (func && PyCallable_Check(func)) {

      PyObject *pValue = PyObject_CallObject(func, nullptr);
      Py_DECREF(pValue);
    }
  }
  Py_RETURN_NONE;
}

static PyMethodDef stellaMethods[]
    = { { "createWindow", (PyCFunction)(void (*)(void))createWindow, METH_VARARGS | METH_KEYWORDS,
          "" },
        { "registerObject", (PyCFunction)(void (*)(void))registerObject,
          METH_VARARGS | METH_KEYWORDS, "" },
        { NULL, NULL, 0, NULL } };

Stella::Stella(QQmlApplicationEngine *e) : engine(e) {
  Q_INIT_RESOURCE(common);

  instance = this;
  QString path = QCoreApplication::applicationDirPath();

  m_appDir = path;
  QString stellaModuleDir;

#if defined(STELLA_DEBUG)
  stellaModuleDir = QDir(m_appDir).absolutePath().replace("build/stella", "python");
  m_dataDir = path;
  QDir dir = QDir(path);
  if (dir.cd("files") && dir.cd("minicap")) {
    m_minicapDir = dir.absolutePath();
  }
#else
#if defined(Q_OS_LINUX)
  m_dataDir = "/usr/share/stella";
#elif defined(Q_OS_WIN32)
  m_dataDir = path;
#endif
#endif

  PyRun_SimpleString("import sys");
  PyRun_SimpleString(QString("sys.path.append(\"%1\")").arg(stellaModuleDir).toStdString().c_str());
  initPyFunctions();

  settings = new Settings("Stella");
  settings->set("applicationDir", m_appDir);
  settings->save();

#if defined(STELLA_DEBUG)
  runDaemon();
#endif

  pluginManager = new PluginManager(this, settings);
  m_watcher = new DeviceWatcher();
  m_adb = new AdbWrapper();
  m_fastboot = new FastbootWrapper(m_watcher);
}

Stella::~Stella() {
  // Py_Finalize();
  delete m_adb;
  delete m_fastboot;
  Q_CLEANUP_RESOURCE(common);

  if (m_watcher->isRunning()) {
    m_watcher->stop();
    delete m_watcher;
  }
#ifdef STELLA_DEBUG
  daemonProcess->terminate();
#endif
}

void Stella::initPyFunctions() {
  // register required functions for our plugins
  // notice that we modify existing module here not
  // creating a new one, cuz it's already registered when
  // we append our folder path to python sys.path.
  // do not try to call these methods from daemon cuz
  // these functions only available for plugins not for daemon.
  PyObject *pName, *pModule;
  const char *n = "stella";
  pName = PyUnicode_FromString(n);
  if(!pName) {
    qDebug() << "Error creating module name string " << pName;
  }
  pModule = PyImport_Import(pName);

  Py_DECREF(pName);
  PyModule_AddFunctions(pModule, stellaMethods);
  Py_DECREF(pModule);
}

#ifdef STELLA_DEBUG
void Stella::runDaemon() {
  QString stellaModuleDir;

#if defined(STELLA_DEBUG)
  stellaModuleDir = QDir(m_appDir).absolutePath().replace("build/stella", "python");
#endif
  daemonProcess = new QProcess();
  QProcess::connect(daemonProcess, &QProcess::errorOccurred, daemonProcess,
                    [this](QProcess::ProcessError e) {
                      qCritical() << e;
                    });

  QString program
      = QDir(m_appDir).absolutePath().replace("build/stella", "python/").append("stellad.py");

  daemonProcess->setProcessChannelMode(QProcess::ForwardedChannels);
  QStringList env = QProcessEnvironment::systemEnvironment().toStringList();
  env.append("PYTHONPATH=" + stellaModuleDir);
  daemonProcess->setEnvironment(env);
  daemonProcess->start(program, QStringList());
  daemonProcess->waitForFinished(200);
}
#endif

Stella *Stella::getInstance(){
  return instance;
}

void Stella::registerObject(QString name, QObject *obj) {
  if (engine) {
    qDebug() << "registering object with name" << name;
    engine->rootContext()->setContextProperty(name, obj);
  }
}

void Stella::createWindow(QUrl url) {
  if (engine) {
    qDebug() << "creating new window";
    engine->load(url);
  }
}

QList<QVariant> Stella::getDevices() {

  AndroidDeviceList *usbDevices = m_watcher->deviceList();

  AndroidDeviceList *adbDevices = m_adb->getDevices();
  AndroidDeviceList *fastbootDevices = m_fastboot->getDevices();

  QList<QVariant> devices = {};
  this->devices.clear();
  for (int i = 0; i < adbDevices->length(); i++) {
    AndroidDevice *d = adbDevices->get(i);
    this->devices.push(d);

    AndroidDevice *usbDevice = usbDevices->getBySerial(d->serial());

    if (usbDevice != nullptr) {
      d->setManufacturer(usbDevice->manufacturer());
      d->setModel(usbDevice->model());
      d->setProduct(usbDevice->product());
      d->setVendorId(usbDevice->vendorId());
      d->setProductId(usbDevice->productId());
      usbDevices->removeBySerial(d->serial());
    }

    QVariant device = QVariant::fromValue(d);
    devices.append(device);
  }

  for (int i = 0; i < fastbootDevices->length(); i++) {
    AndroidDevice *d = fastbootDevices->get(i);
    this->devices.push(d);

    AndroidDevice *usbDevice = usbDevices->getBySerial(d->serial());

    if (usbDevice != nullptr) {
      d->setManufacturer(usbDevice->manufacturer());
      d->setModel(usbDevice->model());
      d->setProduct(usbDevice->product());
      d->setVendorId(usbDevice->vendorId());
      d->setProductId(usbDevice->productId());
      usbDevices->removeBySerial(d->serial());
    }

    QVariant device = QVariant::fromValue(d);
    devices.append(device);
  }
  /**
   * if there is a usb devices left, we assume that these devices is
   * in qualcomm EDL mode, mediatek download mode or samsung odin download mode
   */
  if (usbDevices->length() > 0) {
    for (int i = 0; i < usbDevices->length(); i++) {
      AndroidDevice *usbDevice = usbDevices->get(i);

      QVariant device = QVariant::fromValue(usbDevice);
      devices.append(device);
    }
  }

  return devices;
}

QJSValue Stella::getProp(const QString &serial, const QString &key, QJSValue cb) {
  // ShellResult *result = m_adb->shell(serial, { "getprop", key }, cb);
  QVariant out = m_adb->shell(serial, { "getprop", key }, cb);
  QObject *obj = qvariant_cast<QObject *>(out);
  ShellResult *result = qobject_cast<ShellResult *>(obj);
  return result->output();
}

QVariant Stella::pluginsModel() const {
  return QVariant::fromValue(pluginManager->pluginsModel());
}

void Stella::setAppDir(QString appDir) {
  m_appDir = appDir;
}

QString Stella::appDir() const {
  return m_appDir;
}

QString Stella::dataDir() const {
  return m_dataDir;
}

void Stella::setTempDir(QString tempDir) {
  m_tempDir = tempDir;
}

QString Stella::tempDir() const {
  return m_tempDir;
}

QString Stella::minicapDir() const {
  return m_minicapDir;
}

AdbWrapper *Stella::adbInstance() const {
  return m_adb;
}

FastbootWrapper *Stella::fastbootInstance() const {
  return m_fastboot;
}

DeviceWatcher *Stella::watcherInstance() const {
  return m_watcher;
}
QVariant Stella::adb() const {
  return QVariant::fromValue(m_adb);
}

QVariant Stella::fastboot() const {
  return QVariant::fromValue(m_fastboot);
}

QVariant Stella::watcher() const {
  return QVariant::fromValue(m_watcher);
}

void Stella::mkdir(const QString &dir, QJSValue cb) {

  if (!QDir(dir).exists()) {
    QDir().mkpath(dir);
  }

  if (!cb.isNull() && !cb.isUndefined() && cb.isCallable()) {
    cb.call({});
  }
}

void Stella::rm(const QString &path, QJSValue cb) {
  QFile file(path);
  if (file.exists()) {
    file.remove();
  }

  if (!cb.isNull() && !cb.isUndefined() && cb.isCallable()) {
    cb.call({});
  }
}

void Stella::rmdir(const QString &path, QJSValue cb) {
  if (QDir(path).exists()) {
    QDir(path).removeRecursively();
  }

  if (!cb.isNull() && !cb.isUndefined() && cb.isCallable()) {
    cb.call({});
  }
}

QString Stella::filename(const QString &path) {
  QFileInfo info(path);
  return info.fileName();
}

QString Stella::basename(const QString &path) {
  QFileInfo info(path);

  return info.completeBaseName();
}

QJSValue Stella::exists(const QString &path) {
  QFileInfo file(path);
  return QJSValue(file.exists());
}

QJSValue Stella::isDirectory(const QString &path) {
  QFileInfo file(path);
  return QJSValue(file.isDir());
}

QStringList Stella::listDirectory(const QString &path) {
  QStringList result = {};
  QStringList list = QDir(path).entryList();

  foreach (QString name, list) {
    if (name != "." && name != "..") {
      result.append(name);
    }
  }
  return result;
}

// QString Stella::getAdbPublicKey() {
//   QString homeDir;
//   QString adbPubKeyPath;
// #ifdef Q_OS_LINUX
//   homeDir = QDir::homePath();
//   QString androidPath = QDir(homeDir).filePath(".android");
//   adbPubKeyPath = QDir(androidPath).filePath("adbkey.pub");
//   if (!QDir(adbPubKeyPath).exists()) {
//     QString sshPath = QDir(homeDir).filePath(".ssh");
//     adbPubKeyPath = QDir(sshPath).filePath("adbkey.pub");
//   }
// #elif defined(Q_OS_WIN32)
//   homeDir = QDir::homePath();
// #endif
//   if (!QDir(adbPubKeyPath).exists()) {
//     return NULL;
//   }

//   QFile file(adbPubKeyPath);
//   file.open(QFile::ReadOnly | QFile::Text);
//   QTextStream in(&file);

//   // file.readAll().toS
//   return in.readAll();
// }
