#include "bnr.h"

// Bnr::Bnr(Stella *s) : stella(s) {
//   adb = stella->adbInstance();
//   m_backupDir = QDir(stella->appDir()).filePath("backups");
// }

// Bnr::~Bnr() {
// }

// QList<QVariant> Bnr::getDevices() {

//   AndroidDeviceList *adbDevices = adb->getDevices();

//   QList<QVariant> devices = {};
//   this->devices.clear();
//   for (int i = 0; i < adbDevices->length(); i++) {
//     AndroidDevice *d = adbDevices->get(i);
//     this->devices.push(d);

//     QVariant device = QVariant::fromValue(d);
//     devices.append(device);
//   }

//   return devices;
// }

// void Bnr::setBackupDir(QString backupDir) {
//   m_backupDir = backupDir;
// }

// QString Bnr::backupDir() const {
//   return m_backupDir;
// }

BackupAndRestore::BackupAndRestore() {
  Q_INIT_RESOURCE(bnr);
}
BackupAndRestore::~BackupAndRestore() {
  Q_CLEANUP_RESOURCE(bnr);
}

void BackupAndRestore::run(Stella *stella) {
  qDebug() << "plugin BackupAndRestore run()";
  // Bnr *bnr = new Bnr(stella);
  // stella->registerObject("bnr", bnr);
  stella->createWindow(QUrl(QStringLiteral("qrc:/bnr/bnr.qml")));
}

QString BackupAndRestore::name() const {
  return "Backup & Restore";
}

QString BackupAndRestore::icon() const {
  return "bnr/icon.svg";
}
