// Author:  Jakub Precht

#include "include/settings.h"
#include <QDebug>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <algorithm>
#include <QApplication>

bool Settings::readSettings()
{
  QString _settingsFilename = QApplication::applicationDirPath() + "/settings.json";
  QFileInfo checkFile(_settingsFilename);
  if (checkFile.exists() && checkFile.isFile()) {
    _status = true;
    readSettings(_settingsFilename);
    if (_status)
      return true;
  }
  qInfo().nospace() << "\nFailed to read " << _settingsFilename << ". Using default settings.\n";

  _status = true;
  readSettings(_defaultSettingsFilename);
  if (!_status)
    qCritical() << "\nFailed to read default settings!\n";
  return _status;
}

bool Settings::readSettings(const QString &filename)
{
  QFile file(filename);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    return false;
  }
  QString content = file.readAll();
  file.close();
  QJsonDocument document = QJsonDocument::fromJson(content.toUtf8());
  _root = document.object();

  _status = true;
  _sampleRate = static_cast<int>(readNumber("sampleRate"));
  _frameSize = static_cast<int>(readNumber("frameSize"));
  _hopSize = static_cast<int>(readNumber("hopSize"));
  _indicatorWidth = static_cast<int>(readNumber("indicatorWidth"));
  _indicatorHeight = static_cast<int>(readNumber("indicatorHeight"));
  _staffIndent = static_cast<int>(readNumber("staffIndent"));
  _notesPerStaff = static_cast<int>(readNumber("notesPerStaff"));
  _staffsPerPage = static_cast<int>(readNumber("staffsPerPage"));
  _dpi = static_cast<int>(readNumber("dpi"));
  _confidenceCoefficient = static_cast<float>(readNumber("confidenceCoefficient"));
  _confidenceShift = static_cast<float>(readNumber("confidenceShift"));
  _lilypondWorkingDirectory = readString("lilypondWorkingDirectory");
  _lilypondHeader = readString("lilypondHeader");
  _lilypondFooter = readString("lilypondFooter");

  if (readNotes() == false) {
    qWarning() << "Failed to read \"notes\".";
    _status = false;
  }
  if (readIndicatorXPositions() == false) {
    qWarning() << "Failed to read \"indicatorXPositions\".";
    _status = false;
  }

  if (_frameSize % 2 == 1) {
    qWarning().nospace() << "Frame size cannot be odd. Read value: " << _frameSize << ".";
    _status = false;
  }

  _root = { };
  return _status;
}

double Settings::readNumber(const QString &name)
{
  QJsonValue value = _root.value(name);
  double result = 0;
  bool tmpStatus = false;

  if (value.isDouble()) {
    result = value.toDouble();
    tmpStatus = true;
  }
  else if (value.isString()) {
    QStringList pieces = value.toString().split(" ");
    pieces.removeAll("");

    if (pieces.size() == 1) {
      bool ok = true;
      result = pieces[0].toDouble(&ok);
      tmpStatus = ok;
    }
    else if (pieces.size() == 3 && pieces[1] == "*") {
      bool ok1 = true, ok2 = true;
      result = pieces[0].toInt(&ok1) * pieces[2].toInt(&ok2);
      tmpStatus = ok1 && ok2;
    }
  }

  if (tmpStatus == false)
    qWarning().nospace() << "Failed to read number " << name << '.';
  _status &= tmpStatus;
  return result;
}

QString Settings::readString(const QString &name)
{
  QJsonValue value = _root.value(name);
  if (!value.isString()) {
    qWarning().nospace() << "Failed to read string " << name << '.';
    _status = false;
    return "";
  }
  return value.toString();
}

bool Settings::readNotes()
{
  if (_root.value("notes").isArray() == false)
    return false;

  QJsonArray notes = _root.value("notes").toArray();
  QVector<int> frequency(notes.size());
  _minimalConfidence.fill(0, notes.size());
  _lilypondNotesNotation.fill("", notes.size());

  for (auto it : notes) {
    if (it.isArray() == false)
      return false;
    QJsonArray row = it.toArray();
    if (row[0].isDouble() && row[1].isDouble() && row[2].isDouble() && row[3].isString() == false)
      return false;
    int number = static_cast<int>(row[0].toDouble());
    frequency[number] = static_cast<int>(row[1].toDouble());
    _minimalConfidence[number] = static_cast<float>(row[2].toDouble());
    if (_minimalConfidence[number] == 0.f) {
      _minimalConfidence[number] = 1;
    } else {
      _minimalConfidence[number] =
          qMax(0.f, _minimalConfidence[number] * _confidenceCoefficient + _confidenceShift);
    }
    _lilypondNotesNotation[number] = row[3].toString();
  }

  // calc notes frequency boundries
  float lastBoundry = 0;
  _notesFrequencyBoundry.clear();
  for (int i = 1; i < frequency.size(); i++) {
    float boundry = (frequency[i - 1] + frequency[i]) / 2;
    _notesFrequencyBoundry.push_back({ lastBoundry, boundry });
    lastBoundry = boundry;
  }
  _notesFrequencyBoundry.push_back({ lastBoundry, std::numeric_limits<float>::max() });

  return true;
}

bool Settings::readIndicatorXPositions()
{
  if (_root.value("indicatorXPositions").isArray() == false)
    return false;

  QJsonArray positions = _root.value("indicatorXPositions").toArray();
  _indicatorXs.clear();
  for (auto it : positions) {
    if (it.isDouble() == false)
      return false;
    _indicatorXs.push_back(it.toInt());
  }
  std::sort(_indicatorXs.begin(), _indicatorXs.end());
  if (_indicatorXs.size() > 1) {
    int shift = (_indicatorXs[1] - _indicatorXs[0]) / 2;
    for (auto &x : _indicatorXs)
      x += shift;
  }

  return true;
}

int Settings::sampleRate() const
{
  return _sampleRate;
}

int Settings::frameSize() const
{
  return _frameSize;
}

int Settings::hopSize() const
{
  return _hopSize;
}

float Settings::confidenceCoefficient() const
{
  return _confidenceCoefficient;
}

float Settings::confidenceShift() const
{
  return _confidenceShift;
}

int Settings::indicatorWidth() const
{
  return _indicatorWidth;
}

int Settings::indicatorHeight() const
{
  return _indicatorHeight;
}

int Settings::staffIndent() const
{
  return _staffIndent;
}

int Settings::notesPerStaff() const
{
  return _notesPerStaff;
}

int Settings::staffsPerPage() const
{
  return _staffsPerPage;
}

int Settings::dpi() const
{
  return _dpi;
}

const QVector<float>& Settings::minimalConfidence() const
{
  return _minimalConfidence;
}

const QVector<QPair<float, float>>& Settings::notesFrequencyBoundry() const
{
  return _notesFrequencyBoundry;
}

const QString& Settings::lilypondWorkingDirectory() const
{
  return _lilypondWorkingDirectory;
}

const QString &Settings::lilypondHeader() const
{
  return _lilypondHeader;
}

const QString &Settings::lilypondFooter() const
{
  return _lilypondFooter;
}

const QVector<int>& Settings::indicatorXs() const
{
  return _indicatorXs;
}

const QVector<QString>& Settings::lilypondNotesNotation() const
{
  return _lilypondNotesNotation;
}
