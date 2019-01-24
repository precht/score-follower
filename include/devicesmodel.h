#ifndef DEVICESMODEL_H
#define DEVICESMODEL_H

#include <QVector>
#include <QStandardItemModel>
#include <QAbstractListModel>
#include <QDebug>

class DevicesModel : public QAbstractListModel
{
public:
  DevicesModel(QObject *parent = nullptr);

public slots:
  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:
  void initializeDeviceNames();
  QVector<QString> _deviceNames;
};

#endif // DEVICESMODEL_H
