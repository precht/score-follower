#include "controller.h"
#include <QDebug>
#include <QVector>
#include <QTimer>
#include <QFile>

Controller::Controller(QObject *parent)
  : QObject(parent), _lilypond(new Lilypond()), _recorder(new Recorder())
{
  QVector<int> scoreNotes = readScore();
  readScore();
  _lilypond->setScore(scoreNotes);
  _recorder->setScore(scoreNotes);

  _lilypond->moveToThread(&_lilypondThread);
  _recorder->moveToThread(&_recorderThread);

  connect(this, &Controller::startRecording, _recorder, &Recorder::startRecording);
  connect(this, &Controller::stopRecording, _recorder, &Recorder::stopRecording);
  connect(this, &Controller::generateScore, _lilypond, &Lilypond::generateScore);
  connect(_recorder, &Recorder::positionChanged, _lilypond, &Lilypond::setPosition);
//  connect(_recorder, &Recorder::positionChanged, _lilypond, &Lilypond::setPositionGenerateScore);
  connect(_lilypond, &Lilypond::finishedGeneratingScore, [=](){ emit updateScore(); });

  connect(&_timer, &QTimer::timeout, [=](){ emit generateScore(); });

  _recorderThread.start();
  _lilypondThread.start();
}

Controller::~Controller()
{
  _lilypondThread.quit();
  _recorderThread.quit();
  QThread::msleep(100);
}

void Controller::startScoreFollowing() {
  emit startRecording();
  _timer.start(scoreGenerateInterval);
}

void Controller::stopScoreFollowing() {
  emit stopRecording();
  _timer.stop();
}

QVector<int> Controller::readScore()
{
  QVector<int> scoreNotes;
  scoreNotes.clear();
  QFile scoreFile(_scoreFileName);
  if (scoreFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QTextStream in(&scoreFile);
    QString noteNumber;
    while (!in.atEnd()) {
      in >> noteNumber;
      if (!noteNumber.isEmpty())
        scoreNotes.push_back(noteNumber.toInt());
    }
    scoreFile.close();
  } else {
    qDebug() << "Failed to open: " << _scoreFileName;
  }
  return scoreNotes;
}
