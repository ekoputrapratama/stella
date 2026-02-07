#ifndef BNR_H
#define BNR_H

#include "StellaPlugin.h"
#include "adb_wrapper.h"
#include "devicewatcher.h"
#include "fastboot_wrapper.h"
#include "stella.h"
#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QObject>
#include <QtCore/QResource>
#include <QtCore/QtPlugin>
#include <QtQml/QQmlApplicationEngine>
#include <QtQml/QQmlContext>

class BackupAndRestore : public QObject, public StellaPluginInterface {
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "io.github.stella.StellaPluginInterface" FILE "meta.json")
  Q_INTERFACES(StellaPluginInterface)

public:
  BackupAndRestore();
  ~BackupAndRestore() override;
  void run(Stella *stella) override;
  QString name() const override;
  QString icon() const override;
};

// class Bnr : public QObject {
//   Q_OBJECT
// private:
//   /* data */
//   AdbWrapper *adb;
//   Stella *stella;
//   QString m_backupDir;
//   AndroidDeviceList devices;

// public:
//   Bnr(Stella *stella);
//   ~Bnr();

//   Q_PROPERTY(QString backupDir READ backupDir WRITE setBackupDir);
//   void setBackupDir(QString backupDir);
//   QString backupDir() const;
// public Q_SLOTS:
//   QList<QVariant> getDevices();
// };

#endif
