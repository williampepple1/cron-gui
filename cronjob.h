#ifndef CRONJOB_H
#define CRONJOB_H

#include <QString>
#include <QDateTime>
#include <QJsonObject>
#include <QUuid>

struct CronJob {
    QString id;
    QString name;
    QString scriptPath;
    QString arguments;
    int intervalMinutes;  // Run every X minutes
    bool enabled;
    QDateTime lastRun;
    QDateTime nextRun;

    CronJob() : intervalMinutes(60), enabled(true) {
        id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    }

    QJsonObject toJson() const {
        QJsonObject obj;
        obj["id"] = id;
        obj["name"] = name;
        obj["scriptPath"] = scriptPath;
        obj["arguments"] = arguments;
        obj["intervalMinutes"] = intervalMinutes;
        obj["enabled"] = enabled;
        obj["lastRun"] = lastRun.toString(Qt::ISODate);
        obj["nextRun"] = nextRun.toString(Qt::ISODate);
        return obj;
    }

    static CronJob fromJson(const QJsonObject& obj) {
        CronJob job;
        job.id = obj["id"].toString();
        job.name = obj["name"].toString();
        job.scriptPath = obj["scriptPath"].toString();
        job.arguments = obj["arguments"].toString();
        job.intervalMinutes = obj["intervalMinutes"].toInt(60);
        job.enabled = obj["enabled"].toBool(true);
        job.lastRun = QDateTime::fromString(obj["lastRun"].toString(), Qt::ISODate);
        job.nextRun = QDateTime::fromString(obj["nextRun"].toString(), Qt::ISODate);
        return job;
    }

    void calculateNextRun() {
        if (lastRun.isValid()) {
            nextRun = lastRun.addSecs(intervalMinutes * 60);
        } else {
            nextRun = QDateTime::currentDateTime();
        }
    }

    bool isDue() const {
        return enabled && nextRun.isValid() && QDateTime::currentDateTime() >= nextRun;
    }
};

#endif // CRONJOB_H
