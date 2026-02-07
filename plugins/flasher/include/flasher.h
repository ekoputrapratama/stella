#ifndef FLASHER_PLUGIN_H
#define FLASHER_PLUGIN_H

#ifdef _MSC_VER // Microsoft Visual C Standard Library

#include <Windows.h>
#undef GetBinaryType

#ifndef va_copy
#define va_copy(d, s) ((d) = (s))
#endif

#define FileOpen(FILE, MODE)           fopen(FILE, MODE)
#define FileClose(FILE)                fclose(FILE)
#define FileSeek(FILE, OFFSET, ORIGIN) _fseeki64(FILE, OFFSET, ORIGIN)
#define FileTell(FILE)                 _ftelli64(FILE)
#define FileRewind(FILE)               rewind(FILE)

#else // POSIX Standard Library

#ifdef AUTOCONF
#include "../config.h"
#endif

#include <unistd.h>

#define Sleep(t) usleep(1000 * t)

#define FileOpen(FILE, MODE)           fopen(FILE, MODE)
#define FileClose(FILE)                fclose(FILE)
#define FileSeek(FILE, OFFSET, ORIGIN) fseeko(FILE, OFFSET, ORIGIN)
#define FileTell(FILE)                 ftello(FILE)
#define FileRewind(FILE)               rewind(FILE)

#endif

#if (!(defined _MSC_VER) || (_MSC_VER < 1700))

// #ifndef nullptr
// #define nullptr 0
// #endif

#endif

#include "StellaPlugin.h"
#include "adb_wrapper.h"
#include "devicewatcher.h"
#include "fastboot_wrapper.h"
#include "libpit.h"
#include "stella.h"
#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QObject>
#include <QtCore/QResource>
#include <QtCore/QtPlugin>
#include <QtQml/QQmlApplicationEngine>
#include <QtQml/QQmlContext>
#include <map>

using namespace libpit;

class FlasherPlugin : public QObject, public StellaPluginInterface {
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "io.github.stella.StellaPluginInterface" FILE "meta.json")
  Q_INTERFACES(StellaPluginInterface)

public:
  FlasherPlugin();
  ~FlasherPlugin() override;
  void run(Stella *stella) override;
  QString name() const override;
  QString icon() const override;
};

// class Flasher : public QObject {
//   Q_OBJECT
// private:
//   /* data */
//   void *bridgeManager = nullptr;
//   bool sessionStarted = false;

// public:
//   Flasher(QObject *parent = nullptr);
//   ~Flasher();
// public Q_SLOTS:
//   void flash(const QString &mode, const QString &partition, const QString &filepath);
//   // void flashAll(const QString &mode, const std::map<QString, PartitionFile> partition,
//   //               const QString &filepath);
//   void getPIT(const QString &mode, QJSValueList list = {});
//   void downloadPIT(const QString &mode);
//   void detectDevice(const QString &mode);
//   void reboot(const QString &mode);
//   void loadPackage(const QString &mode, const QString &filepath);
//   void loadScatter(const QString &mode, const QString &filepath);
//   void loadDownloadAgent(const QString &mode, const QString &filepath);
//   void *getBridgeManager();
// };

#endif
