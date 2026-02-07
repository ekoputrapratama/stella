#include "mirroring.h"
#include "DeviceView.h"

MirroringPlugin::MirroringPlugin() {
  Q_INIT_RESOURCE(mirroring);
}

MirroringPlugin::~MirroringPlugin() {
  Q_CLEANUP_RESOURCE(mirroring);
}

void MirroringPlugin::run(Stella *stella) {
  qDebug() << "plugin Mirroring run()";
  MirroringPlugin *mirroring = new MirroringPlugin();
  stella->registerObject("mirroring", mirroring);
  qmlRegisterType<DeviceView>("DeviceView", 1, 0, "DeviceView");
  stella->createWindow(QUrl(QStringLiteral("qrc:/mirroring/mirroring.qml")));
}

QString MirroringPlugin::name() const {
  return "Mirroring";
}

QString MirroringPlugin::icon() const {
  return "mirroring/icons/screen-cast.svg";
}
