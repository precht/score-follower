#include "controller.h"
#include <QDebug>
#include <QVector>
#include <QTimer>

Controller::Controller(QObject *parent)
  : QObject(parent)
{
  m_notes = { 60, 62, 64, 65, 67, 69, 71, 72,
              60, 62, 64, 65, 67, 69, 71, 72,
              60, 62, 64, 65, 67, 69, 71, 72,
              60, 62, 64, 65, 67, 69, 71, 72,
              60, 62, 64, 65, 67, 69, 71, 72 };

  connect(&m_lilypond, &Lilypond::finishedGeneratingScore, [=](){ emit updateScore(); });
  connect(&m_timer, &QTimer::timeout, [=](){
    if (m_index > 16)
      m_timer.stop();
    else
      m_lilypond.generateScore(m_notes, m_index++);
  });
}

void Controller::startScoreFollowing() {
  m_index = 0;
  m_timer.start(250);
}

void Controller::stopScoreFollowing() {
  m_timer.stop();
}
