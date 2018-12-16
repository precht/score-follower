#ifndef RECORDER_H
#define RECORDER_H

#include <QObject>
#include <QTimer>
#include <QAudioRecorder>
#include <QAudioProbe>
#include <QScopedPointer>
#include <QAudioDeviceInfo>
#include <QFile>

#include <map>
#include <essentia/algorithmfactory.h>
#include <essentia/essentiamath.h> // for the isSilent function
#include <essentia/pool.h>

class Recorder : public QObject
{
  Q_OBJECT

public:
  Recorder(QObject *parent = 0);
  void setScore(const QVector<int> &scoreNotes);
  void reset();

public slots:
  void startRecording();
  void stopRecording();
  void processBuffer(const QAudioBuffer buffer);

signals:
  void positionChanged(int position);
  void finished();

private:
  void initializeNotes();
  void initializePitchDetector(int bufferSize, int bufferSampleRate);
  int findNoteFromPitch(float pitch);
  void calculatePosition();

  // -----

  // recording
  const uint _recordingDuration = 60000; // in ms

  QTimer *_timer;
  QAudioProbe *_probe;
  QAudioRecorder *_recorder;
  QAudioEncoderSettings _recorderSettings;

  QString _notesFrequencyFileName = ":/other/notes-frequency";
  QVector<float> _notesFrequency;
  QVector<QPair<float, float>> _notesBoundry;

  // position
  const int64_t _infinity;
  const float _minimalConfidence = 0.4;

  int _position = 0;
  int _rowNumber = 0;
  QVector<int> _scoreNotes;
  QVector<int64_t> _dtwRow;

  // essentia
  const int _essentiaToRecordingBufferSizeRatio = 2;
  int _bufferSize = 0;
  int _bufferSampleRate = 0;
  float _noteNumber = 0;
  float _currentPitch = 0;
  float _currentConfidence = 0;

  std::vector<float> _audio;
  std::vector<float> _spectrum;
  std::vector<float> _windowedframe;

  essentia::standard::Algorithm* _windowCalculator;
  essentia::standard::Algorithm* _spectrumCalculator;
  essentia::standard::Algorithm* _pitchDetector;

};

#endif // RECORDER_H
