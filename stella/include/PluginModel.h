#ifndef PLUGIN_MODEL_H
#define PLUGIN_MODEL_H
#include <QObject>
#include "stella.h"
#include <QDir>


class PluginModel : public QAbstractListModel {
  Q_OBJECT
  
public:
  PluginModel(Stella *parent);
  ~PluginModel();

  enum PluginRoles { NameRole = Qt::UserRole + 1, IconRole };
  void addPlugin(QtPlugin *plugin);
  void append(StellaPluginInterface *plugin);

  int rowCount(const QModelIndex &parent = QModelIndex()) const;

  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
  bool setData(const QModelIndex &index, const QVariant &value, int role);

public Q_SLOTS:
  void launch(const int &index);

protected:
  QHash<int, QByteArray> roleNames() const;

private:
  QList<QtPlugin *> m_plugins;
  Stella *stella;
};

// QT_BEGIN_NAMESPACE
// #define PluginModel_iid "io.github.stella.PluginModel/1.0"
// Q_DECLARE_INTERFACE(PluginModel, PluginModel_iid)
// QT_END_NAMESPACE

#endif
