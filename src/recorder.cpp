#include "include/recorder.h"
#include <QThread>
#include <QUrl>
#include <QIODevice>
#include <qendian.h>

#include <essentia/algorithmfactory.h>
#include <essentia/pool.h>
#include <limits>

using namespace essentia;
using namespace standard;

Recorder::Recorder(QObject *parent)
  : QObject(parent), _infinity(std::numeric_limits<int64_t>::max())
{
  essentia::init();
  initializeNotes();

  _recorder = new QAudioRecorder(this);

  _recorderSettings.setCodec("audio/AMR");
  _recorderSettings.setQuality(QMultimedia::VeryHighQuality);

  _recorder->setEncodingSettings(_recorderSettings);
  _recorder->setOutputLocation(QString("/tmp/score-follower/recording.amr"));

  _probe = new QAudioProbe(this);
  connect(_probe, &QAudioProbe::audioBufferProbed, this, &Recorder::processBuffer);
  _probe->setSource(_recorder);
}

void Recorder::initializeNotes()
{
  _notesFrequency.clear();
  _notesBoundry.clear();

  QFile notesFile(_notesFrequencyFileName);
  if (notesFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QTextStream in(&notesFile);
    int noteNumber;
    float noteFrequency;
    while (!in.atEnd()) {
      in >> noteNumber >> noteFrequency;
      if (noteNumber == 0 && noteFrequency == 0)
        continue;
      if (noteNumber != _notesFrequency.size())
        qDebug() << "Wrong note number in input file: " << _notesFrequencyFileName;
      _notesFrequency.push_back(noteFrequency);
    }
    notesFile.close();
  } else {
    qDebug() << "Failed to open: " << _notesFrequencyFileName;
  }

  float lastBoundry = 0;
  for (int i = 1; i < _notesFrequency.size(); i++) {
    float boundry = (_notesFrequency[i - 1] + _notesFrequency[i]) / 2;
    _notesBoundry.push_back({ lastBoundry, boundry });
    lastBoundry = boundry;
  }
  _notesBoundry.push_back({ lastBoundry, std::numeric_limits<float>::max() });
}

void Recorder::initializePitchDetector(int bufferFrameSize, int bufferSampleRate)
{
  _bufferSize = bufferFrameSize;
  _bufferSampleRate = bufferSampleRate;

  AlgorithmFactory& factory = AlgorithmFactory::instance();

  _windowCalculator = factory.create("Windowing", "type", "hann", "zeroPadding", 0);
  _spectrumCalculator = factory.create("Spectrum", "size", bufferFrameSize);
  _pitchDetector = factory.create("PitchYinFFT", "frameSize", bufferFrameSize, "sampleRate", bufferSampleRate);

  _windowCalculator->input("frame").set(_audio);
  _windowCalculator->output("frame").set(_windowedframe);

  _spectrumCalculator->input("frame").set(_windowedframe);
  _spectrumCalculator->output("spectrum").set(_spectrum);

  _pitchDetector->input("spectrum").set(_spectrum);
  _pitchDetector->output("pitch").set(_currentPitch);
  _pitchDetector->output("pitchConfidence").set(_currentConfidence);
}

void Recorder::setScore(const QVector<int> &scoreNotes)
{
  _scoreNotes = scoreNotes;
  _dtwRow.fill(_infinity, scoreNotes.size());
  _position = 0;
  _rowNumber = 0;
}

int Recorder::findNoteFromPitch(float pitch)
{
  uint beg = 0, end = _notesFrequency.size() - 1;
  while (beg < end) {
    uint m = (beg + end) / 2;
    if (_notesBoundry[m].second < pitch)
      beg = m + 1;
    else if (_notesBoundry[m].first > pitch)
      end = m - 1;
    else
      beg = end = m;
  }
  return beg;
}

void Recorder::startRecording()
{
  _recorder->record();
  _position = 0;
  _rowNumber = 0;
  emit positionChanged(-1);
}

void Recorder::stopRecording()
{
  _recorder->stop();
  emit finished();
}

void Recorder::processBuffer(const QAudioBuffer buffer)
{
  auto format = buffer.format();
  if (_essentiaToRecordingBufferSizeRatio * buffer.frameCount() != _bufferSize || format.sampleRate() != _bufferSampleRate)
    initializePitchDetector(_essentiaToRecordingBufferSizeRatio * buffer.frameCount(), buffer.format().sampleRate());

  const int channelBytes = buffer.format().sampleSize() / 8;
  const unsigned char *ptr = reinterpret_cast<const unsigned char*>(buffer.data());

  if ((int)_audio.size() > _bufferSize)
    _audio.erase(_audio.begin(), _audio.begin() + buffer.frameCount());

  float value = 0;
  for (int i = 0; i < buffer.frameCount(); i++) {

    if (format.sampleSize() == 8) {
      value = *reinterpret_cast<const qint8*>(ptr);
    } else if (format.sampleSize() == 16 ) {
      if (format.byteOrder() == QAudioFormat::LittleEndian)
        value = qFromLittleEndian<qint16>(ptr);
      else
        value = qFromBigEndian<qint16>(ptr);
    } else if (format.sampleSize() == 32 && (format.sampleType() == QAudioFormat::UnSignedInt
                                             || format.sampleType() == QAudioFormat::SignedInt)) {
      if (format.byteOrder() == QAudioFormat::LittleEndian)
        value = qFromLittleEndian<quint32>(ptr);
      else
        value = qFromBigEndian<quint32>(ptr);
    } else if (format.sampleSize() == 32 && format.sampleType() == QAudioFormat::Float) {
      value = *reinterpret_cast<const float*>(ptr);
    } else {
      qDebug() << "Unsupported audio format.";
    }

    _audio.push_back(value);
    ptr += channelBytes;
  }

  _windowCalculator->compute();
  _spectrumCalculator->compute();
  _pitchDetector->compute();

  if (_currentPitch < _notesBoundry[_noteNumber].first || _currentPitch > _notesBoundry[_noteNumber].second)
    _noteNumber = findNoteFromPitch(_currentPitch);

  if (_currentConfidence >= _minimalConfidence)
    calculatePosition();
}

void Recorder::calculatePosition()
{
  // dtw algorithm

  if (_rowNumber++ == 0) {
    _dtwRow[0] = qAbs(_noteNumber - _scoreNotes[0]);
    for (int i = 1; i < _dtwRow.size(); i++)
      _dtwRow[i] = qAbs(_noteNumber - _scoreNotes[i]) + _dtwRow[i - 1];
    return;
  }

  QVector<int64_t> nextRow(_dtwRow.size(), _infinity);
  nextRow[0] = qAbs(_noteNumber - _scoreNotes[0]) + _dtwRow[0];

  int position = 0;
  int minValue = nextRow[0];
  for (int i = 1; i < _dtwRow.size(); i++) {
    nextRow[i] = qAbs(_noteNumber - _scoreNotes[i]) + qMin(nextRow[i - 1], qMin(_dtwRow[i], _dtwRow[i-1]));
    if (nextRow[i] < minValue) {
      position = i;
      minValue = nextRow[i];
    }
  }

  _dtwRow.swap(nextRow);
  if (position != _position)
    emit positionChanged(position);
  _position = position;
}
