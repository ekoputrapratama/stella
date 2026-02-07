#ifndef QT_PLUGIN_H
#define QT_PLUGIN_H

#include "StellaPlugin.h"
#include <QtCore/QAbstractListModel>
#include <QtCore/QObject>

class QtPlugin : public QObject {
  Q_OBJECT
  // Q_PLUGIN_METADATA(IID "io.github.stella.QtPlugin" FILE "meta.json")
  // Q_INTERFACES(QtPlugin)
private:
  /* data */
  QString m_name;
  QString m_icon;
  StellaPluginInterface *m_iface;

public:
  QtPlugin(QString name, QString icon, StellaPluginInterface *iface);
  QtPlugin(StellaPluginInterface *iface);
  ~QtPlugin();

  QString name() const ;
  QString icon() const ;
  // void run(Stella *stella);
  StellaPluginInterface *iface() const;
};

#endif
