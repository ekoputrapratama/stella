#include "PluginModel.h"

PluginModel::PluginModel(Stella *s) : QAbstractListModel((QObject *)s), stella(s) {
}
PluginModel::~PluginModel() {
}

void PluginModel::addPlugin(QtPlugin *plugin) {
  beginInsertRows(QModelIndex(), rowCount(), rowCount());
  m_plugins << plugin;
  endInsertRows();
}

void PluginModel::append(StellaPluginInterface *iface) {
  QtPlugin *plugin = new QtPlugin(iface);
  beginInsertRows(QModelIndex(), rowCount(), rowCount());
  m_plugins << plugin;
  endInsertRows();
}

int PluginModel::rowCount(const QModelIndex &parent) const {
  Q_UNUSED(parent);
  return m_plugins.count();
}

QVariant PluginModel::data(const QModelIndex &index, int role) const {
  if (index.row() < 0 || index.row() >= m_plugins.count())
    return QVariant();

  QtPlugin *plugin = m_plugins[index.row()];
  if (role == NameRole)
    return plugin->name();
  else if (role == IconRole)
    return plugin->icon();
  return QVariant();
}
bool PluginModel::setData(const QModelIndex &index, const QVariant &value, int role) {
  qDebug() << "item clicked";
  return true;
}

//![0]
QHash<int, QByteArray> PluginModel::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[NameRole] = "name";
  roles[IconRole] = "icon";
  return roles;
}

void PluginModel::launch(const int &index) {
  QtPlugin *plugin = m_plugins[index];
  plugin->iface()->run(stella);
}
