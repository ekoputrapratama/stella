#ifndef BNR_SERVER_H
#define BNR_SERVER_H

#include <QObject>
#include <QTcpServer>

class BnrServer : public QObject {
  Q_OBJECT
private:
  /* data */
  QTcpServer *server;
  void initServer();

public:
  BnrServer(QObject *parent = nullptr);
  ~BnrServer();

private Q_SLOTS:
  void onNewConnection();

protected:
  void incomingConnection(qintptr handle);
};

#endif
