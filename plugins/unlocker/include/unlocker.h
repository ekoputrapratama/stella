#ifndef UNLOCKER_PLUGIN_H
#define UNLOCKER_PLUGIN_H

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

class UnlockerPlugin : public QObject, public StellaPluginInterface {
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "io.github.stella.StellaPluginInterface" FILE "meta.json")
  Q_INTERFACES(StellaPluginInterface)

public:
  UnlockerPlugin();
  ~UnlockerPlugin() override;
  void run(Stella *stella) override;
  QString name() const override;
  QString icon() const override;
};

#endif
