// Author:  Jakub Precht

#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "lilypond.h"
#include "recorder.h"
#include "settings.h"

#include <QObject>
#include <QTimer>
#include <QThread>

class Controller : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool follow READ follow WRITE setFollow NOTIFY followChanged)
    Q_PROPERTY(float level READ level WRITE setLevel NOTIFY levelChanged)
    Q_PROPERTY(int indicatorWidth READ indicatorWidth NOTIFY indicatorWidthChanged)
    Q_PROPERTY(int indicatorHeight READ indicatorHeight NOTIFY indicatorHeightChanged)
    Q_PROPERTY(int playedNotes READ playedNotes WRITE setPlayedNotes NOTIFY playedNotesChanged)
    Q_PROPERTY(int pagesNumber READ pagesNumber WRITE setPagesNumber NOTIFY pagesNumberChanged)
    Q_PROPERTY(double indicatorScale READ indicatorScale WRITE setIndicatorScale NOTIFY indicatorScaleChanged)
    Q_PROPERTY(int scoreLength READ scoreLength WRITE setScoreLength NOTIFY scoreLengthChanged)
    Q_PROPERTY(int currentPage READ currentPage NOTIFY currentPageChanged)

public:
    explicit Controller(bool verbose = false, QObject *parent = nullptr);
    ~Controller();

    bool createdSuccessfully() const;

    int notesPerPage() const;
    int pagesNumber() const;
    int scoreLength() const;
    int currentPage() const;

public slots:
    int indicatorX(int index);
    int indicatorY(int index);
    bool openScore();
    float level() const;
    void setLevel(float level);
    bool follow() const;
    void setFollow(bool follow);
    int indicatorWidth() const;
    int indicatorHeight() const;
    int playedNotes() const;
    void setPlayedNotes(int played_notes);
    double indicatorScale() const;
    void setIndicatorScale(double indicator_scale);
    void setPagesNumber(int pages_number);
    void setScoreLength(int score_length);

signals:
    void updateScore();
    void startRecording();
    void stopRecording();
    void generateScore();
    void levelChanged();
    void followChanged();
    void indicatorWidthChanged();
    void indicatorHeightChanged();
    void indicatorScaleChanged();
    void playedNotesChanged();
    void notesPerPageChanged();
    void pagesNumberChanged();
    void scoreLengthChanged();
    void currentPageChanged();
    void cancelledFileOpening();

private:
    void calculateIndicatorYs();
    void updateCurrentPage();
    void resetPageAndPosition();

    // ----------

    bool m_status = true;
    const Settings *m_settings = nullptr;
    Lilypond *m_lilypond = nullptr;
    Recorder *m_recorder = nullptr;
    QThread m_lilypond_thread;
    QThread m_recorder_thread;

    int m_played_notes = 0;
    int m_pages_number = 0;
    int m_score_length = 0;
    int m_current_page = 0;
    bool m_follow = 0;
    float m_level = 0;
    double m_indicator_scale = 1;

    QTimer m_timer;
    QString m_file_to_open;
    QVector<QVector<int>> m_indicator_ys; // for each page generated by lilypond
};



#endif // CONTROLLER_H