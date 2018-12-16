#ifndef LILYPOND_H
#define LILYPOND_H

#include <QObject>
#include <QVector>
#include <QHash>
#include <QProcess>

class Lilypond : public QObject
{
  Q_OBJECT

public:
  explicit Lilypond(QObject *parent = nullptr);
  void setScore(const QVector<int> &scoreNotes);

public slots:
  void setPosition(int position);
  void generateScore();
  void setPositionGenerateScore(int position);

signals:
  void finishedGeneratingScore();

private:
  /**
   * @brief Load header, footer and color changer files use to create lilypond input file.
   */
  void loadFiles();

  // -----

  const QString _headerFileName = ":/other/header.ly";
  const QString _footerFileName = ":/other/footer.ly";
  const QString _colorChangerFileName = ":/other/color-changer.ly";
  const QString _notesFileName = ":/other/notes-lilypond";
  const QString _configFileName = ":/other/lilypond.cfg";

  const int _dpi = 150;
  const int _notesPerLine = 8;
  const QString _lilypondFileName = "/tmp/score-follower/score.ly";
  const QString _scoreFolderName = "/tmp/score-follower/";
  const QString _scoreFileName = "score";

  QByteArray _header;
  QByteArray _footer;
  QByteArray _colorChanger;
  QList<QString> _config;
  QHash<int, QString> _notes;

  QVector<int> _scoreNotes;
  int _numberOfPlayedNotes = -1;
  int _previousNumberOfPlayedNotes = -1;
};


#endif // LILYPOND_H
