#include "StellaPlugin.h"
#include <QObject>

#undef slots
#include <Python.h>
#define slots Q_SLOTS

class PythonPlugin : public QObject, public StellaPluginInterface {
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "io.github.stella.StellaPluginInterface" FILE "meta.json")
  Q_INTERFACES(StellaPluginInterface)
private:
  PyObject *module;

public:
  PythonPlugin();
  PythonPlugin(PyObject *m);
  ~PythonPlugin();

  void setModule(PyObject *m);
  void run(Stella *stella) override;
  QString name() const override;
  QString icon() const override;
};
