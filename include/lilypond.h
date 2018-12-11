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

public slots:
  void generateScore(const QVector<int> &scoreNotes, int numberOfPlayedNotes);

signals:
  void finishedGeneratingScore();

private slots:
  /**
   * @brief Load header, footer and color changer files use to create lilypond input file.
   */
  void loadFiles();
//  void finishGeneratingScore(int exitCode, QProcess::ExitStatus exitStatus);

private:
  const QString m_headerFileName = ":/other/header.ly";
  const QString m_footerFileName = ":/other/footer.ly";
  const QString m_colorChangerFileName = ":/other/color-changer.ly";
  const QString m_notesFileName = ":/other/notes-lilypond";
  const QString m_configFileName = ":/other/lilypond.cfg";

  const int m_notesPerLine = 8;
  const int m_dpi = 150;
  const QString m_lilypondFileName = "/tmp/score.ly";
  const QString m_scoreFolderName = "/tmp/score-follower/";
  const QString m_scoreFileName = "score";

  QByteArray m_header;
  QByteArray m_footer;
  QByteArray m_colorChanger;
  QList<QString> m_config;
  QHash<int, QString> m_notes;
};


#endif // LILYPOND_H
