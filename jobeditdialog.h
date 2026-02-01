#ifndef JOBEDITDIALOG_H
#define JOBEDITDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include "cronjob.h"

class JobEditDialog : public QDialog
{
    Q_OBJECT

public:
    explicit JobEditDialog(QWidget *parent = nullptr);
    explicit JobEditDialog(const CronJob& job, QWidget *parent = nullptr);
    
    CronJob getJob() const;

private slots:
    void browseScript();
    void browseCustomCommand();
    void validateAndAccept();
    void onCustomCommandToggled(bool checked);

private:
    void setupUI();
    void populateFromJob(const CronJob& job);

    QLineEdit* m_nameEdit;
    QLineEdit* m_scriptPathEdit;
    QPushButton* m_browseButton;
    QLineEdit* m_argumentsEdit;
    QCheckBox* m_useCustomCommandCheck;
    QLineEdit* m_customCommandEdit;
    QPushButton* m_browseCommandButton;
    QSpinBox* m_intervalSpinBox;
    QComboBox* m_intervalUnitCombo;
    QCheckBox* m_enabledCheck;
    
    QString m_jobId;
    QDateTime m_lastRun;
    QDateTime m_nextRun;
};

#endif // JOBEDITDIALOG_H
