#ifndef RECORDER_H
#define RECORDER_H

#include <QObject>
#include <QTimer>
#include <QAudioRecorder>
#include <QAudioProbe>
#include <QScopedPointer>
#include <QAudioDeviceInfo>

class Recorder : public QObject
{
  Q_OBJECT
  QVector<QAudioRecorder*> m_audioRecorders;
  QVector<QAudioProbe*> m_probes;
  QTimer *timer;
  QAudioEncoderSettings m_audioSettings;
  int m_id = 0;
  int m_recorderIndex = 0;
  const int mc_maxId = 20;
  const int mc_interval = 250;

public:
  Recorder(QObject *parent = 0);

public slots:
  void run();

  void frame();

  void finish();

signals:
  void finished();
};

#endif // RECORDER_H
