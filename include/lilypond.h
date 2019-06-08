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
    void setScore(const QVector<int> &score_notes);
    void setSettings(const Settings *settings);

public slots:
    void generateScore();

signals:
    void finishedGeneratingScore(int pages_number, QVector<QVector<int>> indicator_ys);

private slots:
    void finish(int exit_code, QProcess::ExitStatus exit_status);

private:
    int countPages() const;
    QVector<QVector<int>> calculateIndicatorYs(int pages_number) const;

    // ----------

    QProcess *m_process;
    const Settings *m_settings;
    QVector<int> m_score_notes;
};


#endif // LILYPOND_H
