#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QObject>
#include <QTimer>
#include <QThread>
#include "lilypond.h"
#include "recorder.h"

class Controller : public QObject
{
  Q_OBJECT

public:
  explicit Controller(QObject *parent = nullptr);
  ~Controller();

signals:
  void updateScore();
  void startRecording();
  void stopRecording();
  void generateScore();

public slots:
  void startScoreFollowing();
  void stopScoreFollowing();

private:
  QVector<int> readScore();

  // -----
  const int scoreGenerateInterval = 250; // in ms
  Lilypond *_lilypond;
  Recorder *_recorder;

  int _index = 0;
  QTimer _timer;
  QString _scoreFileName = ":/other/score";
  QThread _lilypondThread;
  QThread _recorderThread;
};

#endif // CONTROLLER_H
