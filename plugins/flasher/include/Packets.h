#ifndef PACKETS_H
#define PACKETS_H

#include "InboundPacket.h"
#include "OutboundPacket.h"
#include "flasher.h"

class ControlPacket : public OutboundPacket {
public:
  enum {
    kControlTypeSession = 0x64,
    kControlTypePitFile = 0x65,
    kControlTypeFileTransfer = 0x66,
    kControlTypeEndSession = 0x67
  };

protected:
  enum { kDataSize = 4 };

private:
  unsigned int controlType;

public:
  ControlPacket(unsigned int controlType) : OutboundPacket(1024) {
    this->controlType = controlType;
  }

  unsigned int GetControlType(void) const {
    return (controlType);
  }

  virtual void Pack(void) {
    PackInteger(0, controlType);
  }
};

class FileTransferPacket : public ControlPacket {
public:
  enum { kRequestFlash = 0x00, kRequestDump = 0x01, kRequestPart = 0x02, kRequestEnd = 0x03 };

protected:
  enum { kDataSize = ControlPacket::kDataSize + 4 };

private:
  unsigned int request;

public:
  FileTransferPacket(unsigned int request) :
      ControlPacket(ControlPacket::kControlTypeFileTransfer) {
    this->request = request;
  }

  unsigned int GetRequest(void) const {
    return (request);
  }

  virtual void Pack(void) {
    ControlPacket::Pack();

    PackInteger(ControlPacket::kDataSize, request);
  }
};

class BeginDumpPacket : public FileTransferPacket {
public:
  enum { kChipTypeRam = 0, kChipTypeNand = 1 };

private:
  unsigned int chipType;
  unsigned int chipId;

public:
  BeginDumpPacket(unsigned int chipType, unsigned int chipId) :
      FileTransferPacket(FileTransferPacket::kRequestDump) {
    this->chipType = chipType;
    this->chipId = chipId;
  }

  unsigned int GetChipType(void) const {
    return (chipType);
  }

  unsigned int GetChipId(void) const {
    return (chipId);
  }

  virtual void Pack(void) {
    FileTransferPacket::Pack();

    PackInteger(FileTransferPacket::kDataSize, chipType);
    PackInteger(FileTransferPacket::kDataSize + 4, chipId);
  }
};

class SessionSetupPacket : public ControlPacket {
public:
  enum {
    kBeginSession = 0,
    kDeviceType = 1, // ?
    kTotalBytes = 2,
    // kEnableSomeSortOfFlag = 3,
    kFilePartSize = 5,
    kEnableTFlash = 8
  };

private:
  unsigned int request;

protected:
  enum { kDataSize = ControlPacket::kDataSize + 4 };

public:
  SessionSetupPacket(unsigned int request) : ControlPacket(ControlPacket::kControlTypeSession) {
    this->request = request;
  }

  unsigned int GetRequest(void) const {
    return (request);
  }

  void Pack(void) {
    ControlPacket::Pack();

    PackInteger(ControlPacket::kDataSize, request);
  }
};

class BeginSessionPacket : public SessionSetupPacket {
public:
  BeginSessionPacket() : SessionSetupPacket(SessionSetupPacket::kBeginSession) {
  }
};

class DeviceTypePacket : public SessionSetupPacket {
public:
  DeviceTypePacket() : SessionSetupPacket(SessionSetupPacket::kDeviceType) {
  }
};

class DumpPartFileTransferPacket : public FileTransferPacket {
private:
  unsigned int partIndex;

public:
  DumpPartFileTransferPacket(unsigned int partIndex) :
      FileTransferPacket(FileTransferPacket::kRequestPart) {
    this->partIndex = partIndex;
  }

  unsigned int GetPartIndex(void) const {
    return (partIndex);
  }

  virtual void Pack(void) {
    FileTransferPacket::Pack();

    PackInteger(FileTransferPacket::kDataSize, partIndex);
  }
};

class PitFilePacket : public ControlPacket {
public:
  enum {
    kRequestFlash = 0x00,
    kRequestDump = 0x01,
    kRequestPart = 0x02,
    kRequestEndTransfer = 0x03
  };

protected:
  enum { kDataSize = ControlPacket::kDataSize + 4 };

private:
  unsigned int request;

public:
  PitFilePacket(unsigned int request) : ControlPacket(ControlPacket::kControlTypePitFile) {
    this->request = request;
  }

  unsigned int GetRequest(void) const {
    return (request);
  }

  void Pack(void) {
    ControlPacket::Pack();

    PackInteger(ControlPacket::kDataSize, request);
  }
};

class DumpPartPitFilePacket : public PitFilePacket {
private:
  unsigned int partIndex;

public:
  DumpPartPitFilePacket(unsigned int partIndex) : PitFilePacket(PitFilePacket::kRequestPart) {
    this->partIndex = partIndex;
  }

  unsigned int GetPartIndex(void) const {
    return (partIndex);
  }

  void Pack(void) {
    PitFilePacket::Pack();

    PackInteger(PitFilePacket::kDataSize, partIndex);
  }
};

class ResponsePacket : public InboundPacket {
public:
  enum {
    kResponseTypeSendFilePart = 0x00,
    kResponseTypeSessionSetup = 0x64,
    kResponseTypePitFile = 0x65,
    kResponseTypeFileTransfer = 0x66,
    kResponseTypeEndSession = 0x67
  };

private:
  unsigned int responseType;

protected:
  enum { kDataSize = 4 };

public:
  ResponsePacket(int responseType) : InboundPacket(8) {
    this->responseType = responseType;
  }

  unsigned int GetResponseType(void) const {
    return (responseType);
  }

  virtual bool Unpack(void) {
    unsigned int receivedResponseType = UnpackInteger(0);
    if (receivedResponseType != responseType) {
      responseType = receivedResponseType;
      return (false);
    }

    return (true);
  }
};

class EndFileTransferPacket : public FileTransferPacket {
public:
  enum { kDestinationPhone = 0x00, kDestinationModem = 0x01 };

protected:
  enum { kDataSize = FileTransferPacket::kDataSize + 16 };

private:
  unsigned int destination; // PDA / Modem
  unsigned int sequenceByteCount;
  unsigned int unknown1; // EFS?
  unsigned int deviceType;

protected:
  EndFileTransferPacket(unsigned int destination, unsigned int sequenceByteCount,
                        unsigned int unknown1, unsigned int deviceType) :
      FileTransferPacket(FileTransferPacket::kRequestEnd) {
    this->destination = destination;
    this->sequenceByteCount = sequenceByteCount;
    this->unknown1 = unknown1;
    this->deviceType = deviceType;
  }

public:
  unsigned int GetDestination(void) const {
    return (destination);
  }

  unsigned int GetSequenceByteCount(void) const {
    return (sequenceByteCount);
  }

  unsigned int GetUnknown1(void) const {
    return (unknown1);
  }

  unsigned int GetDeviceType(void) const {
    return (deviceType);
  }

  virtual void Pack(void) {
    FileTransferPacket::Pack();

    PackInteger(FileTransferPacket::kDataSize, destination);
    PackInteger(FileTransferPacket::kDataSize + 4, sequenceByteCount);
    PackInteger(FileTransferPacket::kDataSize + 8, unknown1);
    PackInteger(FileTransferPacket::kDataSize + 12, deviceType);
  }
};

class EndModemFileTransferPacket : public EndFileTransferPacket {
private:
  unsigned int endOfFile;

public:
  EndModemFileTransferPacket(unsigned int sequenceByteCount, unsigned int unknown1,
                             unsigned int chipIdentifier, bool endOfFile) :
      EndFileTransferPacket(EndFileTransferPacket::kDestinationModem, sequenceByteCount, unknown1,
                            chipIdentifier) {
    this->endOfFile = (endOfFile) ? 1 : 0;
  }

  bool IsEndOfFile(void) const {
    return (endOfFile == 1);
  }

  void Pack(void) {
    EndFileTransferPacket::Pack();

    PackInteger(EndFileTransferPacket::kDataSize, endOfFile);
  }
};

class EndPhoneFileTransferPacket : public EndFileTransferPacket {
public:
  /*enum
                        {
                                kFilePrimaryBootloader			= 0x00,
                                kFilePit						= 0x01, //
     New 1.1 - Don't flash the pit this way! kFileSecondaryBootloader		= 0x03,
                                kFileSecondaryBootloaderBackup	= 0x04,	// New 1.1
                                kFileKernel						= 0x06,
                                kFileRecovery					= 0x07,	// New 1.1

                                kFileTabletModem				= 0x08, // New 1.2

                                kFileEfs						= 0x14, //
     New 1.1 kFileParamLfs					= 0x15, kFileFactoryFilesystem
     = 0x16, kFileDatabaseData				= 0x17, kFileCache
     = 0x18,

                                kFileModem						= 0x0B //
     New 1.1 - Kies flashes the modem this way rather than using the EndModemFileTransferPacket.
                        };*/

private:
  unsigned int fileIdentifier;
  unsigned int endOfFile;

public:
  EndPhoneFileTransferPacket(unsigned int sequenceByteCount, unsigned int unknown1,
                             unsigned int chipIdentifier, unsigned int fileIdentifier,
                             bool endOfFile) :
      EndFileTransferPacket(EndFileTransferPacket::kDestinationPhone, sequenceByteCount, unknown1,
                            chipIdentifier) {
    this->fileIdentifier = fileIdentifier;
    this->endOfFile = (endOfFile) ? 1 : 0;
  }

  unsigned int GetFileIdentifier(void) {
    return (fileIdentifier);
  }

  bool IsEndOfFile(void) const {
    return (endOfFile == 1);
  }

  void Pack(void) {
    EndFileTransferPacket::Pack();

    PackInteger(EndFileTransferPacket::kDataSize, fileIdentifier);
    PackInteger(EndFileTransferPacket::kDataSize + 4, endOfFile);
  }
};

class EndPitFileTransferPacket : public PitFilePacket {
private:
  unsigned int fileSize;

public:
  EndPitFileTransferPacket(unsigned int fileSize) :
      PitFilePacket(PitFilePacket::kRequestEndTransfer) {
    this->fileSize = fileSize;
  }

  unsigned int GetFileSize(void) const {
    return (fileSize);
  }

  void Pack(void) {
    PitFilePacket::Pack();

    PackInteger(PitFilePacket::kDataSize, fileSize);
  }
};

class EndSessionPacket : public ControlPacket {
public:
  enum { kRequestEndSession = 0, kRequestRebootDevice = 1 };

private:
  unsigned int request;

public:
  EndSessionPacket(unsigned int request) : ControlPacket(ControlPacket::kControlTypeEndSession) {
    this->request = request;
  }

  unsigned int GetRequest(void) const {
    return (request);
  }

  void Pack(void) {
    ControlPacket::Pack();

    PackInteger(ControlPacket::kDataSize, request);
  }
};

class FilePartSizePacket : public SessionSetupPacket {
private:
  unsigned int filePartSize;

public:
  FilePartSizePacket(unsigned int filePartSize) :
      SessionSetupPacket(SessionSetupPacket::kFilePartSize) {
    this->filePartSize = filePartSize;
  }

  unsigned int GetFilePartSize(void) const {
    return filePartSize;
  }

  void Pack(void) {
    SessionSetupPacket::Pack();

    PackInteger(SessionSetupPacket::kDataSize, filePartSize);
  }
};

class FlashPartFileTransferPacket : public FileTransferPacket {
private:
  unsigned int sequenceByteCount;

public:
  FlashPartFileTransferPacket(unsigned int sequenceByteCount) :
      FileTransferPacket(FileTransferPacket::kRequestPart) {
    this->sequenceByteCount = sequenceByteCount;
  }

  unsigned int GetSequenceByteCount(void) const {
    return (sequenceByteCount);
  }

  void Pack(void) {
    FileTransferPacket::Pack();

    PackInteger(FileTransferPacket::kDataSize, sequenceByteCount);
  }
};

class FlashPartPitFilePacket : public PitFilePacket {
private:
  unsigned int partSize;

public:
  FlashPartPitFilePacket(unsigned int partSize) : PitFilePacket(PitFilePacket::kRequestPart) {
    this->partSize = partSize;
  }

  unsigned int GetPartSize(void) const {
    return (partSize);
  }

  void Pack(void) {
    PitFilePacket::Pack();

    PackInteger(PitFilePacket::kDataSize, partSize);
  }
};

class PitFileResponse : public ResponsePacket {
private:
  unsigned int fileSize;

public:
  PitFileResponse() : ResponsePacket(ResponsePacket::kResponseTypePitFile) {
  }

  unsigned int GetFileSize(void) const {
    return (fileSize);
  }

  bool Unpack(void) {
    if (!ResponsePacket::Unpack())
      return (false);

    fileSize = UnpackInteger(ResponsePacket::kDataSize);

    return (true);
  }
};

class ReceiveFilePartPacket : public InboundPacket {
public:
  enum { kDataSize = 500 };

  ReceiveFilePartPacket() : InboundPacket(kDataSize, true) {
  }

  bool Unpack(void) {
    return (true);
  }
};

class SendFilePartPacket : public OutboundPacket {
public:
  SendFilePartPacket(FILE *file, unsigned int size) : OutboundPacket(size) {
    memset(data, 0, size);

    unsigned int position = (unsigned int)FileTell(file);

    FileSeek(file, 0, SEEK_END);
    unsigned int fileSize = (unsigned int)FileTell(file);
    FileSeek(file, position, SEEK_SET);

    // min(fileSize, size)
    unsigned int bytesToRead = (fileSize < size) ? fileSize - position : size;
    (void)fread(data, 1, bytesToRead, file);
  }

  SendFilePartPacket(unsigned char *buffer, unsigned int size) : OutboundPacket(size) {
    memcpy(data, buffer, size);
  }

  void Pack(void) {
  }
};

class SendFilePartResponse : public ResponsePacket {
private:
  unsigned int partIndex;

public:
  SendFilePartResponse() : ResponsePacket(ResponsePacket::kResponseTypeSendFilePart) {
  }

  unsigned int GetPartIndex(void) const {
    return (partIndex);
  }

  bool Unpack(void) {
    if (!ResponsePacket::Unpack())
      return (false);

    partIndex = UnpackInteger(ResponsePacket::kDataSize);

    return (true);
  }
};

class SessionSetupResponse : public ResponsePacket {
private:
  unsigned int result;

public:
  SessionSetupResponse() : ResponsePacket(ResponsePacket::kResponseTypeSessionSetup) {
  }

  unsigned int GetResult(void) const {
    return (result);
  }

  bool Unpack(void) {
    if (!ResponsePacket::Unpack())
      return (false);

    result = UnpackInteger(ResponsePacket::kDataSize);

    return (true);
  }
};

class TotalBytesPacket : public SessionSetupPacket {
private:
  unsigned int totalBytes;

public:
  TotalBytesPacket(unsigned int totalBytes) : SessionSetupPacket(SessionSetupPacket::kTotalBytes) {
    this->totalBytes = totalBytes;
  }

  unsigned int GetTotalBytes(void) const {
    return (totalBytes);
  }

  void Pack(void) {
    SessionSetupPacket::Pack();

    PackInteger(SessionSetupPacket::kDataSize, totalBytes);
  }
};
#endif
