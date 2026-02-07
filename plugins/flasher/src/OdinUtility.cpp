#include "OdinUtility.h"
#include "BridgeManager.h"
#include "Interface.h"
#include "Packets.h"
#include "Utility.h"
#include "flasher.h"

PitData *Odin::getPitData(BridgeManager *bridgeManager, FILE *pitFile, bool repartition) {
  PitData *pitData;
  PitData *localPitData = nullptr;

  // If a PIT file was passed as an argument then we must unpack it.

  if (pitFile) {
    // Load the local pit file into memory.

    FileSeek(pitFile, 0, SEEK_END);
    unsigned int localPitFileSize = (unsigned int)FileTell(pitFile);
    FileRewind(pitFile);

    unsigned char *pitFileBuffer = new unsigned char[localPitFileSize];
    memset(pitFileBuffer, 0, localPitFileSize);

    int dataRead = fread(pitFileBuffer, 1, localPitFileSize, pitFile);

    if (dataRead > 0) {
      FileRewind(pitFile);

      localPitData = new PitData();
      localPitData->Unpack(pitFileBuffer);

      delete[] pitFileBuffer;
    } else {
      Interface::PrintError("Failed to read PIT file.\n");

      delete[] pitFileBuffer;
      return (nullptr);
    }
  }

  if (repartition) {
    // Use the local PIT file data.
    pitData = localPitData;
  } else {
    // If we're not repartitioning then we need to retrieve the device's PIT
    // file and unpack it.
    unsigned char *pitFileBuffer;

    if (bridgeManager->DownloadPitFile(&pitFileBuffer) == 0)
      return (nullptr);

    pitData = new PitData();
    pitData->Unpack(pitFileBuffer);

    delete[] pitFileBuffer;

    if (localPitData != nullptr) {
      // The user has specified a PIT without repartitioning, we should verify
      // the local and device PIT data match!
      bool pitsMatch = pitData->Matches(localPitData);
      delete localPitData;

      if (!pitsMatch) {
        Interface::Print("Local and device PIT files don't match and "
                         "repartition wasn't specified!\n");
        Interface::PrintError("Flash aborted!\n");
        return (nullptr);
      }
    }
  }

  return (pitData);
}

bool Odin::flashPitData(BridgeManager *bridgeManager, const PitData *pitData) {
  Interface::Print("Uploading PIT\n");

  if (bridgeManager->SendPitData(pitData)) {
    Interface::Print("PIT upload successful\n\n");
    return (true);
  } else {
    Interface::PrintError("PIT upload failed!\n\n");
    return (false);
  }
}

bool Odin::flashFile(BridgeManager *bridgeManager, const PartitionFlashInfo &partitionFlashInfo) {
  if (partitionFlashInfo.pitEntry->GetBinaryType()
      == PitEntry::kBinaryTypeCommunicationProcessor) // Modem
  {
    Interface::Print("Uploading %s\n", partitionFlashInfo.pitEntry->GetPartitionName());

    if (bridgeManager->SendFile(partitionFlashInfo.file,
                                EndModemFileTransferPacket::kDestinationModem,
                                partitionFlashInfo.pitEntry->GetDeviceType())) {
      Interface::Print("%s upload successful\n\n", partitionFlashInfo.pitEntry->GetPartitionName());
      return (true);
    } else {
      Interface::PrintError("%s upload failed!\n\n",
                            partitionFlashInfo.pitEntry->GetPartitionName());
      return (false);
    }
  } else // partitionFlashInfo.pitEntry->GetBinaryType() ==
         // PitEntry::kBinaryTypeApplicationProcessor
  {
    Interface::Print("Uploading %s\n", partitionFlashInfo.pitEntry->GetPartitionName());

    if (bridgeManager->SendFile(partitionFlashInfo.file,
                                EndPhoneFileTransferPacket::kDestinationPhone,
                                partitionFlashInfo.pitEntry->GetDeviceType(),
                                partitionFlashInfo.pitEntry->GetIdentifier())) {
      Interface::Print("%s upload successful\n\n", partitionFlashInfo.pitEntry->GetPartitionName());
      return (true);
    } else {
      Interface::PrintError("%s upload failed!\n\n",
                            partitionFlashInfo.pitEntry->GetPartitionName());
      return (false);
    }
  }
}

bool Odin::setupPartitionFlashInfo(const vector<PartitionFile> &partitionFiles,
                                   const PitData *pitData,
                                   vector<PartitionFlashInfo> &partitionFlashInfos) {
  for (vector<PartitionFile>::const_iterator it = partitionFiles.begin();
       it != partitionFiles.end(); it++) {
    const PitEntry *pitEntry = nullptr;

    // Was the argument a partition identifier?
    unsigned int partitionIdentifier;

    if (Utility::ParseUnsignedInt(partitionIdentifier, it->argumentName)
        == kNumberParsingStatusSuccess) {
      pitEntry = pitData->FindEntry(partitionIdentifier);

      if (!pitEntry) {
        Interface::PrintError("No partition with identifier \"%s\" exists in "
                              "the specified PIT.\n",
                              it->argumentName);
        return (false);
      }
    } else {
      // The argument must be an partition name e.g. "ZIMAGE"
      pitEntry = pitData->FindEntry(it->argumentName);

      if (!pitEntry) {
        Interface::PrintError("Partition \"%s\" does not exist in the specified PIT.\n",
                              it->argumentName);
        return (false);
      }
    }

    partitionFlashInfos.push_back(PartitionFlashInfo(pitEntry, it->file));
  }

  return (true);
}

bool Odin::flashPartitions(BridgeManager *bridgeManager,
                           const vector<PartitionFile> &partitionFiles, const PitData *pitData,
                           bool repartition) {
  vector<PartitionFlashInfo> partitionFlashInfos;

  // Map the files being flashed to partitions stored in the PIT file.
  if (!setupPartitionFlashInfo(partitionFiles, pitData, partitionFlashInfos))
    return (false);

  // If we're repartitioning then we need to flash the PIT file first (if it is
  // listed in the PIT file).
  if (repartition) {
    if (!flashPitData(bridgeManager, pitData))
      return (false);
  }

  // Flash partitions in the same order that arguments were specified in.
  for (vector<PartitionFlashInfo>::const_iterator it = partitionFlashInfos.begin();
       it != partitionFlashInfos.end(); it++) {
    if (!flashFile(bridgeManager, *it))
      return (false);
  }
  return (true);
}

bool Odin::sendTotalTransferSize(BridgeManager *bridgeManager,
                                 const vector<PartitionFile> &partitionFiles, FILE *pitFile,
                                 bool repartition) {
  unsigned int totalBytes = 0;

  for (vector<PartitionFile>::const_iterator it = partitionFiles.begin();
       it != partitionFiles.end(); it++) {
    FileSeek(it->file, 0, SEEK_END);
    totalBytes += (unsigned int)FileTell(it->file);
    FileRewind(it->file);
  }

  if (repartition) {
    FileSeek(pitFile, 0, SEEK_END);
    totalBytes += (unsigned int)FileTell(pitFile);
    FileRewind(pitFile);
  }

  bool success;

  TotalBytesPacket *totalBytesPacket = new TotalBytesPacket(totalBytes);
  success = bridgeManager->SendPacket(totalBytesPacket);
  delete totalBytesPacket;

  if (!success) {
    Interface::PrintError("Failed to send total bytes packet!\n");
    return (false);
  }

  SessionSetupResponse *totalBytesResponse = new SessionSetupResponse();
  success = bridgeManager->ReceivePacket(totalBytesResponse);
  int totalBytesResult = totalBytesResponse->GetResult();
  delete totalBytesResponse;

  if (!success) {
    Interface::PrintError("Failed to receive session total bytes response!\n");
    return (false);
  }

  if (totalBytesResult != 0) {
    Interface::PrintError("Unexpected session total bytes response!\nExpected: 0\nReceived:%d\n",
                          totalBytesResult);
    return (false);
  }

  return (true);
}
