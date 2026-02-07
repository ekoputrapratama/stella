#ifndef ODIN_UTILITY_H
#define ODIN_UTILITY_H

#include "libpit.h"
#include <vector>

using namespace std;
using namespace libpit;

class BridgeManager;

struct PartitionFile {
  const char *argumentName;
  FILE *file;

  PartitionFile(const char *argumentName, FILE *file) {
    this->argumentName = argumentName;
    this->file = file;
  }
};
struct PartitionFlashInfo {
  const PitEntry *pitEntry;
  FILE *file;

  PartitionFlashInfo(const PitEntry *pitEntry, FILE *file) {
    this->pitEntry = pitEntry;
    this->file = file;
  }
};

namespace Odin {

  PitData *getPitData(BridgeManager *bridgeManager, FILE *pitFile, bool repartition);
  bool flashPitData(BridgeManager *bridgeManager, const PitData *pitData);
  bool flashFile(BridgeManager *bridgeManager, const PartitionFlashInfo &partitionFlashInfo);
  bool setupPartitionFlashInfo(const vector<PartitionFile> &partitionFiles, const PitData *pitData,
                               vector<PartitionFlashInfo> &partitionFlashInfos);
  bool flashPartitions(BridgeManager *bridgeManager, const vector<PartitionFile> &partitionFiles,
                       const PitData *pitData, bool repartition);

  bool sendTotalTransferSize(BridgeManager *bridgeManager,
                             const vector<PartitionFile> &partitionFiles, FILE *pitFile,
                             bool repartition);
}
#endif
