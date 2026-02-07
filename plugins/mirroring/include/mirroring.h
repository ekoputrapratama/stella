#ifndef MIRRORING_H
#define MIRRORING_H

#include "StellaPlugin.h"
#include "stella.h"
#include <QtCore/QObject>

class MirroringPlugin : public QObject, public StellaPluginInterface {
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "io.github.stella.StellaPluginInterface" FILE "meta.json")
  Q_INTERFACES(StellaPluginInterface)
private:
  /* data */
public:
  MirroringPlugin(/* args */);
  ~MirroringPlugin();
  void run(Stella *stella) override;
  QString name() const override;
  QString icon() const override;
};

#endif
