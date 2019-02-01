// Author:  Jakub Precht

#ifndef LILYPOND_H
#define LILYPOND_H

#include <QObject>
#include <QVector>

class Settings;

class Lilypond : public QObject
{
  Q_OBJECT

public:
  explicit Lilypond(QObject *parent = nullptr);
  void setScore(const QVector<int> &scoreNotes);
  void setSettings(const Settings *settings);

public slots:
  void generateScore();

signals:
  void finishedGeneratingScore(int pagesNumber, QVector<QVector<int>> indicatorYs);

private:
  int countPages() const;
  QVector<QVector<int>> calculateIndicatorYs(int pagesNumber) const;

  // ----------

  const Settings *_settings;
  QVector<int> _scoreNotes;
};


#endif // LILYPOND_H
