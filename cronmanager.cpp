#include "cronmanager.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QFileInfo>

CronManager::CronManager(QObject *parent)
    : QObject(parent)
    , m_timer(new QTimer(this))
    , m_running(false)
{
    connect(m_timer, &QTimer::timeout, this, &CronManager::checkAndRunJobs);
    loadJobs();
}

CronManager::~CronManager()
{
    stop();
    saveJobs();
}

void CronManager::addJob(const CronJob& job)
{
    CronJob newJob = job;
    newJob.calculateNextRun();
    m_jobs.append(newJob);
    saveJobs();
    emit jobsChanged();
    emit logMessage(QString("Added job: %1").arg(newJob.name));
}

void CronManager::updateJob(const CronJob& job)
{
    for (int i = 0; i < m_jobs.size(); ++i) {
        if (m_jobs[i].id == job.id) {
            m_jobs[i] = job;
            m_jobs[i].calculateNextRun();
            saveJobs();
            emit jobsChanged();
            emit logMessage(QString("Updated job: %1").arg(job.name));
            return;
        }
    }
}

void CronManager::removeJob(const QString& id)
{
    for (int i = 0; i < m_jobs.size(); ++i) {
        if (m_jobs[i].id == id) {
            QString name = m_jobs[i].name;
            m_jobs.removeAt(i);
            saveJobs();
            emit jobsChanged();
            emit logMessage(QString("Removed job: %1").arg(name));
            return;
        }
    }
}

CronJob* CronManager::getJob(const QString& id)
{
    for (int i = 0; i < m_jobs.size(); ++i) {
        if (m_jobs[i].id == id) {
            return &m_jobs[i];
        }
    }
    return nullptr;
}

QVector<CronJob>& CronManager::getJobs()
{
    return m_jobs;
}

void CronManager::start()
{
    if (!m_running) {
        m_running = true;
        m_timer->start(30000); // Check every 30 seconds
        emit logMessage("Cron scheduler started");
        // Run initial check
        checkAndRunJobs();
    }
}

void CronManager::stop()
{
    if (m_running) {
        m_running = false;
        m_timer->stop();
        emit logMessage("Cron scheduler stopped");
    }
}

void CronManager::checkAndRunJobs()
{
    QDateTime now = QDateTime::currentDateTime();
    
    for (int i = 0; i < m_jobs.size(); ++i) {
        if (m_jobs[i].isDue()) {
            executeJob(m_jobs[i]);
        }
    }
}

void CronManager::executeJob(CronJob& job)
{
    emit logMessage(QString("Executing job: %1").arg(job.name));
    
    QProcess* process = new QProcess(this);
    QString program = job.scriptPath;
    QStringList args;
    
    if (!job.arguments.isEmpty()) {
        args = job.arguments.split(" ", Qt::SkipEmptyParts);
    }
    
    // Determine how to run the script based on extension
    QFileInfo fileInfo(job.scriptPath);
    QString ext = fileInfo.suffix().toLower();
    
    if (ext == "py") {
        args.prepend(job.scriptPath);
        program = "python";
    } else if (ext == "ps1") {
        args.prepend(job.scriptPath);
        args.prepend("-File");
        args.prepend("-ExecutionPolicy");
        args.prepend("Bypass");
        program = "powershell";
    } else if (ext == "bat" || ext == "cmd") {
        args.prepend(job.scriptPath);
        args.prepend("/c");
        program = "cmd";
    } else if (ext == "js") {
        args.prepend(job.scriptPath);
        program = "node";
    }
    
    QString jobId = job.id;
    
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this, process, jobId](int exitCode, QProcess::ExitStatus status) {
        QString output = process->readAllStandardOutput();
        QString error = process->readAllStandardError();
        bool success = (exitCode == 0 && status == QProcess::NormalExit);
        
        QString fullOutput = output;
        if (!error.isEmpty()) {
            fullOutput += "\nErrors:\n" + error;
        }
        
        emit jobExecuted(jobId, success, fullOutput);
        emit logMessage(QString("Job completed with exit code: %1").arg(exitCode));
        
        process->deleteLater();
    });
    
    connect(process, &QProcess::errorOccurred, this, [this, process, jobId](QProcess::ProcessError error) {
        QString errorMsg;
        switch(error) {
            case QProcess::FailedToStart: errorMsg = "Failed to start"; break;
            case QProcess::Crashed: errorMsg = "Process crashed"; break;
            case QProcess::Timedout: errorMsg = "Timed out"; break;
            default: errorMsg = "Unknown error"; break;
        }
        emit jobExecuted(jobId, false, errorMsg);
        emit logMessage(QString("Job error: %1").arg(errorMsg));
        process->deleteLater();
    });
    
    // Update job timing
    job.lastRun = QDateTime::currentDateTime();
    job.calculateNextRun();
    saveJobs();
    emit jobsChanged();
    
    process->start(program, args);
}

QString CronManager::getConfigPath()
{
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(configDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    return configDir + "/cronjobs.json";
}

void CronManager::saveJobs()
{
    QJsonArray jobsArray;
    for (const CronJob& job : m_jobs) {
        jobsArray.append(job.toJson());
    }
    
    QJsonDocument doc(jobsArray);
    QFile file(getConfigPath());
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
    }
}

void CronManager::loadJobs()
{
    QFile file(getConfigPath());
    if (!file.exists()) {
        return;
    }
    
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray data = file.readAll();
        file.close();
        
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isArray()) {
            QJsonArray jobsArray = doc.array();
            m_jobs.clear();
            for (const QJsonValue& val : jobsArray) {
                if (val.isObject()) {
                    m_jobs.append(CronJob::fromJson(val.toObject()));
                }
            }
        }
    }
    emit logMessage(QString("Loaded %1 jobs").arg(m_jobs.size()));
}

void CronManager::setAutoStart(bool enable)
{
    QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run",
                       QSettings::NativeFormat);
    
    if (enable) {
        QString appPath = QCoreApplication::applicationFilePath();
        appPath = QDir::toNativeSeparators(appPath);
        settings.setValue("CronGUI", QString("\"%1\" --hidden").arg(appPath));
    } else {
        settings.remove("CronGUI");
    }
}

bool CronManager::isAutoStartEnabled()
{
    QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run",
                       QSettings::NativeFormat);
    return settings.contains("CronGUI");
}
