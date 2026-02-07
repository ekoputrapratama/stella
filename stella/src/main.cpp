#include "main.h"
#include "sqlite3.h"
#include "stella.h"
#undef slots
#include <Python.h>
#define slots Q_SLOTS
QQmlApplicationEngine *engine;
Stella *stella = nullptr;

void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &message) {
  QString mode;
  if (type == QtInfoMsg) {
    mode = "INFO";
  } else if (type == QtWarningMsg) {
    mode = "WARNING";
  } else if (type == QtCriticalMsg) {
    mode = "CRITICAL";
  } else if (type == QtFatalMsg) {
    mode = "FATAL";
  } else {
    mode = "DEBUG";
  }
  // std::cout << &mode << " : " << &message;
  // printf("", mode, message);
  qDebug("%s : %s", qPrintable(mode), qPrintable(message));
  if (!engine->rootObjects().isEmpty() && mode == "INFO") {
    QObject *mainForm = engine->rootObjects().first();
    QMetaObject::invokeMethod(mainForm, "onConsoleLog", Q_ARG(QVariant, message));
  } else if (mode == "INFO") {
    stella->onConsoleMessage(message);
  }
}

int main(int argc, char *argv[]) {
  QGuiApplication app(argc, argv);
  app.setOrganizationName("Stella");
  app.setOrganizationDomain("io.github.stella");
  app.setApplicationName("Stella");

  qInstallMessageHandler(messageHandler);

  Py_Initialize();

  engine = new QQmlApplicationEngine();

  stella = new Stella(engine);
  SQLite3 *sqlite = new SQLite3(engine);

  QQmlContext *rootContext = engine->rootContext();
  rootContext->setContextProperty("stella", stella);
  rootContext->setContextProperty("sqlite3", sqlite);
  // rootContext->setContextProperty("watcher", watcher);
  rootContext->setContextProperty("pluginsModel", stella->pluginsModel());

  engine->load(QUrl(QStringLiteral("qrc:/main.qml")));

  if (engine->rootObjects().isEmpty())
    return -1;

  app.connect(&app, &QGuiApplication::aboutToQuit, stella, []() {
    // if (watcher->isRunning()) {
    //   watcher->quit();
    // }
    // ::stella->killAllProcess();
    // ::stella->stopAdb();
    stella->deleteLater();
  });
  int ret = app.exec();
  // Py_FinalizeEx();
  return ret;
}
