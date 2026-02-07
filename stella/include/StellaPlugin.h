#ifndef STELLA_PLUGIN_H
#define STELLA_PLUGIN_H

#include <QtPlugin>

class Stella;
class StellaPluginInterface {
private:
  /* data */
public:
  virtual ~StellaPluginInterface() = default;
  virtual void run(Stella *stella) = 0;
  virtual QString name() const = 0;
  virtual QString icon() const = 0;
};

QT_BEGIN_NAMESPACE
#define StellaPluginInterface_iid "io.github.stella.StellaPluginInterface/1.0"
Q_DECLARE_INTERFACE(StellaPluginInterface, StellaPluginInterface_iid)
QT_END_NAMESPACE

#endif
