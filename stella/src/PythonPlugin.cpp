#include "PythonPlugin.h"
#include <python3.14/object.h>

PythonPlugin::PythonPlugin() : QObject(nullptr){
}

PythonPlugin::PythonPlugin(PyObject *m) : QObject(nullptr), module(m) {
}

PythonPlugin::~PythonPlugin() {
}

void PythonPlugin::setModule(PyObject *m) {
  module = m;
}

void PythonPlugin::run(Stella *stella) {
  PyObject *pFunc = PyObject_GetAttrString(module, "run");

  if (pFunc && PyCallable_Check(pFunc)) {
    qDebug() << "python function defined";
    PyObject *pValue = PyObject_CallObject(pFunc, nullptr);
    Py_DECREF(pValue);
  }
  Py_DECREF(pFunc);
}

QString PythonPlugin::name() const {
  PyObject *pFunc = PyObject_GetAttrString(module, "name");
  if (pFunc && PyCallable_Check(pFunc)) {

    PyObject *pValue = PyObject_CallObject(pFunc, nullptr);
    char *name = PyBytes_AsString(pValue);

    Py_DECREF(pValue);
    return QString(name);
  }
  return QString();
}

QString PythonPlugin::icon() const {
  PyObject *pFunc = PyObject_GetAttrString(module, "icon");
  if (pFunc && PyCallable_Check(pFunc)) {
    PyObject *pValue = PyObject_CallObject(pFunc, nullptr);

    char *name = PyBytes_AsString(pValue);

    Py_DECREF(pValue);
    return QString(name);
  }
  return QString();
}
