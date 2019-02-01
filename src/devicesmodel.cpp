// Author:  Jakub Precht

#include "devicesmodel.h"

#include <QDebug>
#include <QAudioDeviceInfo>
#include <QAudioRecorder>

// TODO make it work

DevicesModel::DevicesModel(QObject *parent)
  : QAbstractListModel(parent)
{
  initializeDeviceNames();
}

void DevicesModel::initializeDeviceNames()
{
  _deviceNames.clear();
  const QAudioDeviceInfo &defaultDevice = QAudioDeviceInfo::defaultInputDevice();
  _deviceNames.push_back(defaultDevice.deviceName());
  for (auto &deviceInfo: QAudioDeviceInfo::availableDevices(QAudio::AudioInput)) {
    if (deviceInfo != defaultDevice)
      _deviceNames.push_back(deviceInfo.deviceName());
  }
}

int DevicesModel::rowCount(const QModelIndex&) const
{
  return _deviceNames.size();
}

QVariant DevicesModel::data(const QModelIndex &index, int role) const
{
  if (index.row() < 0 || index.row() >= _deviceNames.size() || role != Qt::DisplayRole)
    return QVariant();

  return QVariant(_deviceNames[index.row()]);
}
