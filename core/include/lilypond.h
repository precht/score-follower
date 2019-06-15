// Author:  Jakub Precht

#ifndef LILYPOND_H
#define LILYPOND_H

#include <QObject>
#include <QVector>
#include <QProcess>

class Settings;

class Lilypond : public QObject
{
    Q_OBJECT

public:
    explicit Lilypond(QObject *parent = nullptr);
    virtual void setScore(const QVector<int> &score_notes);
    void setSettings(const Settings *settings);
    void detectStaffHorizontalPositionsInImage(QImage &image, QVector<int> &positions);
    virtual ~Lilypond();
    virtual int countPages();

public slots:
    void generateScore();
    void finish(int exit_code, QProcess::ExitStatus exit_status);

signals:
    void finishedGeneratingScore(int pages_number, QVector<QVector<int>> indicator_ys);

private:
    QVector<QVector<int>> calculateIndicatorYs(int pages_number);

    // ----------

    QProcess *m_process;
    const Settings *m_settings;
    QVector<int> m_score_notes;
};


#endif // LILYPOND_H
