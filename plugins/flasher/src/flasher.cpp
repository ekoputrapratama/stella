#include "flasher.h"
#include "BridgeManager.h"
#include "OdinUtility.h"

// using namespace libpit;

FlasherPlugin::FlasherPlugin() {
  Q_INIT_RESOURCE(flasher);
}
FlasherPlugin::~FlasherPlugin() {
  Q_CLEANUP_RESOURCE(flasher);
}

void FlasherPlugin::run(Stella *stella) {
  qDebug() << "plugin Flasher run()";
  stella->createWindow(QUrl(QStringLiteral("qrc:/flasher/flasher.qml")));
}

QString FlasherPlugin::name() const {
  return "Flasher";
}

QString FlasherPlugin::icon() const {
  return "flasher/flasher.svg";
}

/* *
 * Flasher
 * */
// Flasher::Flasher(QObject *parent) {
// }

// Flasher::~Flasher() {
// }

// void *Flasher::getBridgeManager() {
//   BridgeManager *bm;
//   if (bridgeManager) {
//     bm = (BridgeManager *)bridgeManager;
//   } else {
//     bm = new BridgeManager(true);

//     if ((bm->Initialise(false) != BridgeManager::kInitialiseSucceeded || !bm->BeginSession())) {
//       delete bm;
//       sessionStarted = false;
//       return nullptr;
//     }
//   }

//   if (!bridgeManager) {
//     sessionStarted = true;
//     bridgeManager = (void *)bm;
//   }

//   return bm;
// }

// void Flasher::loadPackage(const QString &mode, const QString &filepath) {
//   if (mode == "odin") {
// #if defined(Q_OS_WIN32)

// #elif defined(Q_OS_LINUX)

// #endif
//   }
// }

// void Flasher::flash(const QString &mode, const QString &partition, const QString &filepath) {
//   if (mode == "odin") {
//     BridgeManager *bm = (BridgeManager *)getBridgeManager();

//     QString tmpPath = QDir().home().filePath(".stella");
//     QDir().mkpath(tmpPath);

//     const char *outputFilename = QDir(tmpPath).filePath("table.pit").toLocal8Bit().data();
//     FILE *outputPitFile = fopen(outputFilename, "wb");
//     // download pit file
//     unsigned char *pitBuffer;
//     int fileSize = bm->DownloadPitFile(&pitBuffer);
//     bool success = true;

//     if (fileSize > 0) {
//       if (fwrite(pitBuffer, 1, fileSize, outputPitFile) != fileSize) {
//         Interface::PrintError("Failed to write PIT data to output file.\n");
//         success = false;
//       }
//     } else {
//       success = false;
//     }
//     fclose(outputPitFile);
//     FILE *pitFile = fopen(outputFilename, "rb");

//     std::vector<PartitionFile> partitionFiles;

//     FILE *recoveryFile = fopen(filepath.toLocal8Bit().data(), "rb");
//     // QString partition = "RECOVERY";
//     PartitionFile recovery = PartitionFile(partition.toLocal8Bit().data(), recoveryFile);

//     partitionFiles.push_back(recovery);

//     success = Odin::sendTotalTransferSize(bm, partitionFiles, pitFile, false);
//     if (success) {
//       PitData *pitData = Odin::getPitData(bm, pitFile, false);

//       if (pitData)
//         success = Odin::flashPartitions(bm, partitionFiles, pitData, false);
//       else
//         success = false;

//       delete pitData;
//     }
//     fclose(pitFile);
//     fclose(recoveryFile);

//     if (success) {
//       qDebug() << "flashing finished!";
//     } else {
//       qDebug() << "flashing failed!";
//     }
//   }
// }
