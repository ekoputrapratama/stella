#ifndef BNRD_H
#define BNRD_H

#include "BnrServer.h"
#include "bnr-common.h"
#include <QObject>

class Bnrd : public QObject {
  Q_OBJECT
private:
  QString m_backupDir;
  BnrServer *server;
  void initDirectories();

public:
  Bnrd(QObject *parent = nullptr);

public Q_SLOTS:
  Q_SCRIPTABLE void changeBackupDir(const QString &backupDir);
};

#endif
