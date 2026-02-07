#include "QtPlugin.h"

QtPlugin::QtPlugin(QString name, QString icon, StellaPluginInterface *iface) :
    m_name(name), m_icon(icon), m_iface(iface) {
}
QtPlugin::QtPlugin(StellaPluginInterface *iface) :
    m_name(iface->name()), m_icon(iface->icon()), m_iface(iface) {
}
QtPlugin::~QtPlugin() {
}
QString QtPlugin::name() const {
  return m_name;
}

QString QtPlugin::icon() const {
  return m_icon;
}

StellaPluginInterface *QtPlugin::iface() const {
  return m_iface;
}
