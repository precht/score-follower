// Author:  Jakub Precht

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QString>
#include <QVector>
#include <QJsonObject>

class Settings
{
public:
  bool readSettings();
  int sampleRate() const;
  int frameSize() const;
  int hopSize() const;
  float confidenceCoefficient() const;
  float confidenceShift() const;
  int indicatorWidth() const;
  int indicatorHeight() const;
  int staffIndent() const;
  int notesPerStaff() const;
  int staffsPerPage() const;
  int dpi() const;

  const QVector<float>& minimalConfidence() const;
  const QVector<QPair<float, float>>& notesFrequencyBoundry() const;
  const QVector<int>& indicatorXs() const;
  const QVector<QString>& lilypondNotesNotation() const;

  const QString& lilypondWorkingDirectory() const;
  const QString& lilypondHeader() const;
  const QString& lilypondFooter() const;

private:
  bool readSettings(const QString &filename);
  double readNumber(const QString &name);
  QString readString(const QString &name);
  bool readNotes();
  bool readIndicatorXPositions();

  // ----------

  QJsonObject _root;
  bool _status = false;
  const QString _defaultSettingsFilename = ":/other/settings.json";

  // essentia

  int _sampleRate = 0;
  int _frameSize = 0;
  int _hopSize = 0;
  float _confidenceCoefficient = 0;
  float _confidenceShift = 0;
  QVector<float> _minimalConfidence;
  QVector<QPair<float, float>> _notesFrequencyBoundry;

  // lilypond and gui

  int _indicatorWidth = 0;
  int _indicatorHeight = 0;
  int _staffIndent = 0;
  int _notesPerStaff = 0;
  int _staffsPerPage = 0;
  int _dpi = 0;
  QVector<int> _indicatorXs;
  QVector<QString> _lilypondNotesNotation;

  QString _lilypondWorkingDirectory;
  QString _lilypondHeader;
  QString _lilypondFooter;
};

#endif // SETTINGS_H
