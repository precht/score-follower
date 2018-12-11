#include "include/recorder.h"
#include <QThread>
#include <QUrl>

Recorder::Recorder(QObject *parent)
  : QObject(parent)
{ }

void Recorder::run()
{
  m_audioRecorders.append(new QAudioRecorder(this));
  m_audioRecorders.append(new QAudioRecorder(this));

  m_audioSettings.setCodec("audio/AMR");
  m_audioSettings.setQuality(QMultimedia::VeryHighQuality);

  m_audioRecorders[0]->setEncodingSettings(m_audioSettings);
  m_audioRecorders[1]->setEncodingSettings(m_audioSettings);

  m_audioRecorders[0]->setOutputLocation(QUrl::fromLocalFile("out/test_" + QVariant(m_id).toString() + ".amr"));
  m_audioRecorders[0]->record();
  m_recorderIndex = 0;


  timer = new QTimer();
  connect(timer, SIGNAL(timeout()), this, SLOT(frame()));
  timer->start(mc_interval);
}

void Recorder::frame() {
  m_audioRecorders[m_recorderIndex]->stop();
  m_recorderIndex = (m_recorderIndex + 1) % 2;

  if (++m_id < mc_maxId) {
    m_audioRecorders[m_recorderIndex]->setOutputLocation(QUrl::fromLocalFile("out/test_" + QString::number(m_id) + ".amr"));
    m_audioRecorders[m_recorderIndex]->record();
    timer->start(mc_interval);
  } else {
    QThread::msleep(200);
    emit finished();
  }
}

void Recorder::finish() {
  emit finished();
}
