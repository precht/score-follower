#ifndef RECORDER_H
#define RECORDER_H

#include <QObject>
#include <QTimer>
#include <QAudioRecorder>
#include <QAudioProbe>
#include <QScopedPointer>
#include <QAudioDeviceInfo>
#include <QFile>
#include <QAudioInput>

#include <map>
#include <essentia/algorithmfactory.h>
#include <essentia/essentiamath.h> // for the isSilent function
#include <essentia/pool.h>

class Recorder : public QObject
{
  Q_OBJECT

public:
  Recorder(QObject *parent = nullptr);
  void setScore(const QVector<int> &scoreNotes);
  void resetDtw();

public slots:
//  bool setDevice(const QString &deviceName);
  void startRecording();
  void stopRecording();
  void processBuffer(const QAudioBuffer buffer);

signals:
  void positionChanged(int position);
  void levelChanged(float level);

private:
  void initializeDevices();
  void initializeNotes();
  void initializePitchDetector(int bufferSize, int bufferSampleRate);
  int findNoteFromPitch(float pitch);
  void calculatePosition();
  void calculateMaxAmplitude();

  void setMaxAmplitude(const QAudioFormat &format);

  // -----

  // recording
//  const uint _recordingDuration = 60000; // in ms

  bool _isRunning = false;
  QTimer *_timer = nullptr;
  QAudioProbe *_probe = nullptr;
  QAudioRecorder *_recorder = nullptr;
  QAudioInput *_audioInput = nullptr;
  QAudioFormat _defaultFormat;
  QAudioFormat _currentFormat;
  QAudioEncoderSettings _recorderSettings;

  QString _notesFrequencyFileName = ":/other/notes-frequency";
  QVector<float> _notesFrequency;
  QVector<QPair<float, float>> _notesBoundry;
  QMap<QString, QAudioDeviceInfo> _devices;

  float _level = 0;
  float _maxAmplitude = 1;


  // position
  int _position = 0;
  QVector<int> _scoreNotes;
  QVector<int64_t> _dtwRow;
  QVector<int64_t> _nextRow;


  // essentia
  const int _sampleRate = 44100;
  const float _minimalConfidence = 0.7;
  const int _essentiaToRecordingBufferSizeRatio = 4;
  const int64_t _infinity = std::numeric_limits<int64_t>::max();
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
