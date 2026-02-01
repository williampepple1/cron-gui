#ifndef CRONMANAGER_H
#define CRONMANAGER_H

#include <QObject>
#include <QTimer>
#include <QVector>
#include <QProcess>
#include <QSettings>
#include "cronjob.h"

class CronManager : public QObject
{
    Q_OBJECT

public:
    explicit CronManager(QObject *parent = nullptr);
    ~CronManager();

    void addJob(const CronJob& job);
    void updateJob(const CronJob& job);
    void removeJob(const QString& id);
    void runJobNow(const QString& id);  // Run a job immediately
    CronJob* getJob(const QString& id);
    QVector<CronJob>& getJobs();

    void start();
    void stop();

    void saveJobs();
    void loadJobs();

    // Windows startup management
    static void setAutoStart(bool enable);
    static bool isAutoStartEnabled();

signals:
    void jobExecuted(const QString& jobId, bool success, const QString& output);
    void jobsChanged();
    void logMessage(const QString& message);

private slots:
    void checkAndRunJobs();

private:
    void executeJob(CronJob& job);
    QString getConfigPath();

    QVector<CronJob> m_jobs;
    QTimer* m_timer;
    bool m_running;
};

#endif // CRONMANAGER_H
