// Author:  Jakub Precht

#include "controller.h"
#include "scorereader.h"
#include "settings.h"

#include <QDebug>
#include <QVector>
#include <QTimer>
#include <QFile>
#include <QFileDialog>
#include <QStandardPaths>

Controller::Controller(bool verbose, QObject *parent)
    : QObject(parent), m_lilypond(new Lilypond()), m_recorder(new Recorder())
{
    Settings *settings = new Settings();
    settings->setVerbose(verbose);
    m_status = settings->readSettings();
    if (!m_status)
        return;
    m_settings = settings;
    m_lilypond->setSettings(settings);
    m_recorder->setSettings(settings);
    m_status &= m_recorder->initialize();

    m_lilypond->moveToThread(&m_lilypond_thread);
    m_recorder->moveToThread(&m_recorder_thread);

    connect(this, &Controller::startRecording, m_recorder, &Recorder::startFollowing);
    connect(this, &Controller::stopRecording, m_recorder, &Recorder::stopFollowing);
    connect(this, &Controller::generateScore, m_lilypond, &Lilypond::generateScore);
    connect(m_recorder, &Recorder::positionChanged, [=](int position){ setPlayedNotes(position); });
    connect(m_recorder, &Recorder::levelChanged, [=](float level){ setLevel(level); });

    connect(m_lilypond, &Lilypond::finishedGeneratingScore, [=](int pages_count, QVector<QVector<int>> indicator_ys){
        m_current_page = 1;
        m_indicator_ys = indicator_ys;
        setPagesNumber(pages_count);
        emit currentPageChanged();
        emit updateScore();
    });

    connect(&m_timer, &QTimer::timeout, [=](){ emit generateScore(); });

    m_recorder_thread.start();
    m_lilypond_thread.start();
}

Controller::~Controller()
{
    m_lilypond_thread.quit();
    m_recorder_thread.quit();
    QThread::msleep(100);
    delete m_settings;
}

bool Controller::createdSuccessfully() const
{
    return m_status;
}

bool Controller::openScore()
{
    m_file_to_open = QFileDialog::getOpenFileName(nullptr, "Open Score",
                                                  QStandardPaths::writableLocation(QStandardPaths::MusicLocation),
                                                  "Score Files (*.txt *.mid);; All Files (*.*)");
    if (m_file_to_open == "")
        return false;

    QVector<int> score_notes = ScoreReader::readScoreFile(m_file_to_open);
    m_lilypond->setScore(score_notes);
    m_recorder->setScore(score_notes);
    setScoreLength(score_notes.size());

    emit generateScore();
    return true;
}

int Controller::playedNotes() const
{
    return m_played_notes;
}

void Controller::setPlayedNotes(int played_notes)
{
    if (played_notes == m_played_notes)
        return;

    m_played_notes = played_notes;
    updateCurrentPage();
    emit playedNotesChanged();
}

int Controller::indicatorHeight() const
{
    return m_settings->indicatorHeight();
}

int Controller::indicatorWidth() const
{
    return m_settings->indicatorWidth();
}

double Controller::indicatorScale() const
{
    return m_indicator_scale;
}

void Controller::setIndicatorScale(double indicator_scale)
{
    m_indicator_scale = indicator_scale;
    emit indicatorScaleChanged();
}

bool Controller::follow() const
{
    return m_follow;
}

void Controller::setFollow(bool follow)
{
    if (m_follow == follow)
        return;

    m_follow = follow;
    if (follow == true) {
        emit startRecording();
    } else {
        emit stopRecording();
    }

    emit followChanged();
}

float Controller::level() const
{
    return m_level;
}

void Controller::setLevel(float level)
{
    m_level = level;
    emit levelChanged();
}

int Controller::indicatorX(int index)
{
    if (index < 0 || m_settings->indicatorXs().size() == 0) {
        qWarning() << "wrong indicator x index or empty x indicator vector";
        return 0;
    }
    return m_settings->indicatorXs()[index % m_settings->indicatorXs().size()];
}

int Controller::indicatorY(int index)
{
    if (m_settings->indicatorXs().size() == 0 || m_indicator_ys.size() == 0) {
        qWarning() << "Empty indicator vectors";
        return 0;
    }

    int page = 0;
    while (page < m_pages_number && index >= m_indicator_ys[page].size() * m_settings->indicatorXs().size()) {
        index -= m_indicator_ys[page].size() * m_settings->indicatorXs().size();
        page++;
    }

    int y = index / m_settings->indicatorXs().size();
    if (y < 0 || y >= m_indicator_ys[page].size()) {
        qWarning() << "wrong indicator y index:" << index;
        return 0;
    }
    return m_indicator_ys[page][y];
}

int Controller::pagesNumber() const
{
    return m_pages_number;
}

int Controller::scoreLength() const
{
    return m_score_length;
}

int Controller::currentPage() const
{
    return m_current_page;
}

void Controller::setPagesNumber(int pages_number)
{
    if (m_pages_number == pages_number)
        return;

    m_pages_number = pages_number;
    emit pagesNumberChanged();
}

void Controller::setScoreLength(int score_length)
{
    if (m_score_length == score_length)
        return;

    m_score_length = score_length;
    emit scoreLengthChanged();
}

void Controller::updateCurrentPage()
{
    int index = m_played_notes;
    int page = 0;
    while (page < m_pages_number && index > m_indicator_ys[page].size() * m_settings->indicatorXs().size()) {
        index -= m_indicator_ys[page].size() * m_settings->indicatorXs().size();
        page++;
    }
    if (page + 1!= m_current_page) {
        m_current_page = page + 1;
        emit currentPageChanged();
    }
}

void Controller::resetPageAndPosition()
{
    setPlayedNotes(0);
    m_current_page = 1;
    emit currentPageChanged();
}
