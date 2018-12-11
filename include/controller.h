#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QObject>
#include <QTimer>
#include "lilypond.h"

class Controller : public QObject
{
  Q_OBJECT
  Lilypond m_lilypond;
  int m_index = 0;
  QTimer m_timer;
  QVector<int> m_notes;

public:
  explicit Controller(QObject *parent = nullptr);

signals:
  void updateScore();

public slots:
  void startScoreFollowing();
  void stopScoreFollowing();
};

#endif // CONTROLLER_H
