#include "unlocker.h"

UnlockerPlugin::UnlockerPlugin() {
  Q_INIT_RESOURCE(unlocker);
}
UnlockerPlugin::~UnlockerPlugin() {
  Q_CLEANUP_RESOURCE(unlocker);
}

void UnlockerPlugin::run(Stella *stella) {
  qDebug() << "plugin Unlocker run()";
  // Bnr *bnr = new Bnr(stella);
  // stella->registerObject("bnr", bnr);
  stella->createWindow(QUrl(QStringLiteral("qrc:/unlocker/unlocker.qml")));
}

QString UnlockerPlugin::name() const {
  return "Unlocker";
}

QString UnlockerPlugin::icon() const {
  return "unlocker/unlocker.svg";
}
