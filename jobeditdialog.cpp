#include "jobeditdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QGroupBox>

JobEditDialog::JobEditDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUI();
    setWindowTitle("Add New Job");
}

JobEditDialog::JobEditDialog(const CronJob& job, QWidget *parent)
    : QDialog(parent)
{
    setupUI();
    setWindowTitle("Edit Job");
    populateFromJob(job);
}

void JobEditDialog::setupUI()
{
    setMinimumWidth(500);
    
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // Job Details Group
    QGroupBox* detailsGroup = new QGroupBox("Job Details", this);
    QFormLayout* formLayout = new QFormLayout(detailsGroup);
    
    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setPlaceholderText("Enter a descriptive name for this job");
    formLayout->addRow("Name:", m_nameEdit);
    
    // Script path with browse button
    QHBoxLayout* scriptLayout = new QHBoxLayout();
    m_scriptPathEdit = new QLineEdit(this);
    m_scriptPathEdit->setPlaceholderText("Path to script or executable file (any type)");
    m_browseButton = new QPushButton("Browse...", this);
    scriptLayout->addWidget(m_scriptPathEdit);
    scriptLayout->addWidget(m_browseButton);
    formLayout->addRow("Script/File:", scriptLayout);
    
    m_argumentsEdit = new QLineEdit(this);
    m_argumentsEdit->setPlaceholderText("Optional command line arguments");
    formLayout->addRow("Arguments:", m_argumentsEdit);
    
    mainLayout->addWidget(detailsGroup);
    
    // Custom Command Group
    QGroupBox* commandGroup = new QGroupBox("Custom Command (Optional)", this);
    QFormLayout* commandLayout = new QFormLayout(commandGroup);
    
    m_useCustomCommandCheck = new QCheckBox("Use custom command/runtime", this);
    m_useCustomCommandCheck->setToolTip("Enable this to specify a custom interpreter or command to run your script");
    commandLayout->addRow("", m_useCustomCommandCheck);
    
    QHBoxLayout* customCmdLayout = new QHBoxLayout();
    m_customCommandEdit = new QLineEdit(this);
    m_customCommandEdit->setPlaceholderText("e.g., ruby, perl, php, C:\\path\\to\\runtime.exe");
    m_customCommandEdit->setEnabled(false);
    m_browseCommandButton = new QPushButton("Browse...", this);
    m_browseCommandButton->setEnabled(false);
    customCmdLayout->addWidget(m_customCommandEdit);
    customCmdLayout->addWidget(m_browseCommandButton);
    commandLayout->addRow("Command:", customCmdLayout);
    
    QLabel* helpLabel = new QLabel("Leave empty to auto-detect based on file extension.\n"
                                   "Common runtimes: python, node, ruby, perl, php, powershell", this);
    helpLabel->setStyleSheet("color: gray; font-size: 9pt;");
    commandLayout->addRow("", helpLabel);
    
    mainLayout->addWidget(commandGroup);
    
    connect(m_useCustomCommandCheck, &QCheckBox::toggled, this, &JobEditDialog::onCustomCommandToggled);
    connect(m_browseCommandButton, &QPushButton::clicked, this, &JobEditDialog::browseCustomCommand);
    
    // Schedule Group
    QGroupBox* scheduleGroup = new QGroupBox("Schedule", this);
    QFormLayout* scheduleLayout = new QFormLayout(scheduleGroup);
    
    QHBoxLayout* intervalLayout = new QHBoxLayout();
    m_intervalSpinBox = new QSpinBox(this);
    m_intervalSpinBox->setMinimum(1);
    m_intervalSpinBox->setMaximum(9999);
    m_intervalSpinBox->setValue(1);
    
    m_intervalUnitCombo = new QComboBox(this);
    m_intervalUnitCombo->addItem("Minutes", 1);
    m_intervalUnitCombo->addItem("Hours", 60);
    m_intervalUnitCombo->addItem("Days", 1440);
    m_intervalUnitCombo->setCurrentIndex(1); // Default to hours
    
    intervalLayout->addWidget(m_intervalSpinBox);
    intervalLayout->addWidget(m_intervalUnitCombo);
    intervalLayout->addStretch();
    scheduleLayout->addRow("Run every:", intervalLayout);
    
    m_enabledCheck = new QCheckBox("Job is enabled", this);
    m_enabledCheck->setChecked(true);
    scheduleLayout->addRow("", m_enabledCheck);
    
    mainLayout->addWidget(scheduleGroup);
    
    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    QPushButton* cancelButton = new QPushButton("Cancel", this);
    QPushButton* saveButton = new QPushButton("Save", this);
    saveButton->setDefault(true);
    
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(saveButton);
    
    mainLayout->addLayout(buttonLayout);
    
    // Connections
    connect(m_browseButton, &QPushButton::clicked, this, &JobEditDialog::browseScript);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(saveButton, &QPushButton::clicked, this, &JobEditDialog::validateAndAccept);
}

void JobEditDialog::populateFromJob(const CronJob& job)
{
    m_jobId = job.id;
    m_lastRun = job.lastRun;
    m_nextRun = job.nextRun;
    
    m_nameEdit->setText(job.name);
    m_scriptPathEdit->setText(job.scriptPath);
    m_argumentsEdit->setText(job.arguments);
    m_useCustomCommandCheck->setChecked(job.useCustomCommand);
    m_customCommandEdit->setText(job.customCommand);
    m_customCommandEdit->setEnabled(job.useCustomCommand);
    m_browseCommandButton->setEnabled(job.useCustomCommand);
    m_enabledCheck->setChecked(job.enabled);
    
    // Convert minutes to appropriate unit
    int minutes = job.intervalMinutes;
    if (minutes >= 1440 && minutes % 1440 == 0) {
        m_intervalSpinBox->setValue(minutes / 1440);
        m_intervalUnitCombo->setCurrentIndex(2); // Days
    } else if (minutes >= 60 && minutes % 60 == 0) {
        m_intervalSpinBox->setValue(minutes / 60);
        m_intervalUnitCombo->setCurrentIndex(1); // Hours
    } else {
        m_intervalSpinBox->setValue(minutes);
        m_intervalUnitCombo->setCurrentIndex(0); // Minutes
    }
}

void JobEditDialog::onCustomCommandToggled(bool checked)
{
    m_customCommandEdit->setEnabled(checked);
    m_browseCommandButton->setEnabled(checked);
    if (!checked) {
        m_customCommandEdit->clear();
    }
}

void JobEditDialog::browseCustomCommand()
{
    QString filter = "Executables (*.exe);;All Files (*.*)";
    QString fileName = QFileDialog::getOpenFileName(this, "Select Runtime/Command", "", filter);
    if (!fileName.isEmpty()) {
        m_customCommandEdit->setText(fileName);
    }
}

void JobEditDialog::browseScript()
{
    QString filter = "All Files (*.*);;Scripts (*.py *.ps1 *.bat *.cmd *.js *.rb *.pl *.php *.sh);;Executables (*.exe)";
    QString fileName = QFileDialog::getOpenFileName(this, "Select Script or File", "", filter);
    if (!fileName.isEmpty()) {
        m_scriptPathEdit->setText(fileName);
        
        // Auto-fill name if empty
        if (m_nameEdit->text().isEmpty()) {
            QFileInfo info(fileName);
            m_nameEdit->setText(info.baseName());
        }
    }
}

void JobEditDialog::validateAndAccept()
{
    if (m_nameEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Validation Error", "Please enter a name for the job.");
        m_nameEdit->setFocus();
        return;
    }
    
    if (m_scriptPathEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Validation Error", "Please select a script file.");
        m_scriptPathEdit->setFocus();
        return;
    }
    
    QFileInfo fileInfo(m_scriptPathEdit->text());
    if (!fileInfo.exists()) {
        QMessageBox::warning(this, "Validation Error", "The selected script file does not exist.");
        m_scriptPathEdit->setFocus();
        return;
    }
    
    accept();
}

CronJob JobEditDialog::getJob() const
{
    CronJob job;
    
    if (!m_jobId.isEmpty()) {
        job.id = m_jobId;
        job.lastRun = m_lastRun;
        job.nextRun = m_nextRun;
    }
    
    job.name = m_nameEdit->text().trimmed();
    job.scriptPath = m_scriptPathEdit->text().trimmed();
    job.arguments = m_argumentsEdit->text().trimmed();
    job.useCustomCommand = m_useCustomCommandCheck->isChecked();
    job.customCommand = m_customCommandEdit->text().trimmed();
    job.enabled = m_enabledCheck->isChecked();
    
    int multiplier = m_intervalUnitCombo->currentData().toInt();
    job.intervalMinutes = m_intervalSpinBox->value() * multiplier;
    
    return job;
}
