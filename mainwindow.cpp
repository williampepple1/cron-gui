#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "jobeditdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QGroupBox>
#include <QHeaderView>
#include <QMessageBox>
#include <QApplication>
#include <QStyle>
#include <QScrollBar>

MainWindow::MainWindow(bool startHidden, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_cronManager(new CronManager(this))
    , m_startHidden(startHidden)
{
    ui->setupUi(this);
    
    setWindowTitle("Cron Job Manager");
    setMinimumSize(900, 600);
    
    setupUI();
    setupSystemTray();
    
    // Connect signals
    connect(m_cronManager, &CronManager::jobExecuted, this, &MainWindow::onJobExecuted);
    connect(m_cronManager, &CronManager::jobsChanged, this, &MainWindow::onJobsChanged);
    connect(m_cronManager, &CronManager::logMessage, this, &MainWindow::onLogMessage);
    
    // Initial refresh
    refreshJobTable();
    updateStatusBar();
    
    // Start the scheduler
    m_cronManager->start();
    
    // Log startup info
    onLogMessage("Application started");
    onLogMessage(QString("Loaded %1 job(s)").arg(m_cronManager->getJobs().size()));
    for (const CronJob& job : m_cronManager->getJobs()) {
        if (job.enabled) {
            onLogMessage(QString("  - %1: next run at %2").arg(job.name, job.nextRun.toString("yyyy-MM-dd hh:mm:ss")));
        }
    }
    
    // Handle hidden start
    if (m_startHidden) {
        hide();
        m_trayIcon->showMessage("Cron Job Manager", 
            "Application is running in the background.", 
            QSystemTrayIcon::Information, 3000);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupUI()
{
    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    
    // Toolbar buttons
    QHBoxLayout* toolbarLayout = new QHBoxLayout();
    
    m_addButton = new QPushButton("Add Job", this);
    m_editButton = new QPushButton("Edit", this);
    m_removeButton = new QPushButton("Remove", this);
    m_runNowButton = new QPushButton("Run Now", this);
    
    m_addButton->setIcon(QIcon::fromTheme("list-add"));
    m_editButton->setIcon(QIcon::fromTheme("document-edit"));
    m_removeButton->setIcon(QIcon::fromTheme("list-remove"));
    m_runNowButton->setIcon(QIcon::fromTheme("media-playback-start"));
    
    toolbarLayout->addWidget(m_addButton);
    toolbarLayout->addWidget(m_editButton);
    toolbarLayout->addWidget(m_removeButton);
    toolbarLayout->addWidget(m_runNowButton);
    toolbarLayout->addStretch();
    
    m_autoStartCheck = new QCheckBox("Start with Windows", this);
    m_autoStartCheck->setChecked(CronManager::isAutoStartEnabled());
    toolbarLayout->addWidget(m_autoStartCheck);
    
    mainLayout->addLayout(toolbarLayout);
    
    // Splitter for table and log
    QSplitter* splitter = new QSplitter(Qt::Vertical, this);
    
    // Jobs table
    QGroupBox* jobsGroup = new QGroupBox("Scheduled Jobs", this);
    QVBoxLayout* jobsLayout = new QVBoxLayout(jobsGroup);
    
    m_jobTable = new QTableWidget(this);
    m_jobTable->setColumnCount(6);
    m_jobTable->setHorizontalHeaderLabels({"Name", "Script", "Interval", "Last Run", "Next Run", "Status"});
    m_jobTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_jobTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_jobTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_jobTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_jobTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_jobTable->setAlternatingRowColors(true);
    
    jobsLayout->addWidget(m_jobTable);
    splitter->addWidget(jobsGroup);
    
    // Log view
    QGroupBox* logGroup = new QGroupBox("Activity Log", this);
    QVBoxLayout* logLayout = new QVBoxLayout(logGroup);
    
    m_logView = new QTextEdit(this);
    m_logView->setReadOnly(true);
    m_logView->setMinimumHeight(200);
    m_logView->setAcceptRichText(false);  // Use plain text for reliability
    m_logView->setStyleSheet("QTextEdit { font-family: Consolas, monospace; font-size: 10pt; background-color: #f5f5f5; color: #000000; }");
    
    logLayout->addWidget(m_logView);
    splitter->addWidget(logGroup);
    
    mainLayout->addWidget(splitter);
    
    // Status bar
    m_statusLabel = new QLabel(this);
    statusBar()->addWidget(m_statusLabel);
    
    setCentralWidget(centralWidget);
    
    // Connect button signals
    connect(m_addButton, &QPushButton::clicked, this, &MainWindow::addJob);
    connect(m_editButton, &QPushButton::clicked, this, &MainWindow::editJob);
    connect(m_removeButton, &QPushButton::clicked, this, &MainWindow::removeJob);
    connect(m_runNowButton, &QPushButton::clicked, this, &MainWindow::runJobNow);
    connect(m_autoStartCheck, &QCheckBox::toggled, this, &MainWindow::toggleAutoStart);
    connect(m_jobTable, &QTableWidget::doubleClicked, this, &MainWindow::editJob);
}

void MainWindow::setupSystemTray()
{
    m_trayIcon = new QSystemTrayIcon(this);
    
    // Use custom icon
    QIcon trayIcon(":/icons/tray_icon.png");
    if (!trayIcon.isNull()) {
        m_trayIcon->setIcon(trayIcon);
        setWindowIcon(trayIcon);
    } else {
        // Fallback to a standard icon
        QIcon fallback = style()->standardIcon(QStyle::SP_ComputerIcon);
        m_trayIcon->setIcon(fallback);
        setWindowIcon(fallback);
    }
    m_trayIcon->setToolTip("Cron Job Manager");
    
    m_trayMenu = new QMenu(this);
    
    m_showAction = new QAction("Show Window", this);
    connect(m_showAction, &QAction::triggered, this, &MainWindow::showWindow);
    
    QAction* addAction = new QAction("Add Job...", this);
    connect(addAction, &QAction::triggered, this, [this]() {
        showWindow();
        addJob();
    });
    
    m_trayMenu->addAction(m_showAction);
    m_trayMenu->addAction(addAction);
    m_trayMenu->addSeparator();
    
    // Note: No quit action - only killable via Task Manager
    QAction* aboutAction = new QAction("About", this);
    connect(aboutAction, &QAction::triggered, this, [this]() {
        QMessageBox::about(this, "About Cron Job Manager",
            "Cron Job Manager v1.0\n\n"
            "A background task scheduler for Windows.\n\n"
            "This application runs in the system tray and executes\n"
            "your scripts on schedule, even when minimized.\n\n"
            "To completely exit, use Task Manager.");
    });
    m_trayMenu->addAction(aboutAction);
    
    m_trayIcon->setContextMenu(m_trayMenu);
    
    connect(m_trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::trayIconActivated);
    
    m_trayIcon->show();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // Minimize to tray instead of closing
    hide();
    m_trayIcon->showMessage("Cron Job Manager", 
        "Application minimized to tray. Jobs will continue running.", 
        QSystemTrayIcon::Information, 2000);
    event->ignore();
}

void MainWindow::trayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::DoubleClick) {
        showWindow();
    }
}

void MainWindow::showWindow()
{
    show();
    raise();
    activateWindow();
}

void MainWindow::addJob()
{
    JobEditDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        CronJob job = dialog.getJob();
        m_cronManager->addJob(job);
    }
}

void MainWindow::editJob()
{
    int row = m_jobTable->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "Edit Job", "Please select a job to edit.");
        return;
    }
    
    QString jobId = m_jobTable->item(row, 0)->data(Qt::UserRole).toString();
    CronJob* job = m_cronManager->getJob(jobId);
    
    if (job) {
        JobEditDialog dialog(*job, this);
        if (dialog.exec() == QDialog::Accepted) {
            CronJob updatedJob = dialog.getJob();
            m_cronManager->updateJob(updatedJob);
        }
    }
}

void MainWindow::removeJob()
{
    int row = m_jobTable->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "Remove Job", "Please select a job to remove.");
        return;
    }
    
    QString jobName = m_jobTable->item(row, 0)->text();
    QString jobId = m_jobTable->item(row, 0)->data(Qt::UserRole).toString();
    
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Remove Job",
        QString("Are you sure you want to remove '%1'?").arg(jobName),
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        m_cronManager->removeJob(jobId);
    }
}

void MainWindow::runJobNow()
{
    int row = m_jobTable->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "Run Job", "Please select a job to run.");
        return;
    }
    
    QString jobId = m_jobTable->item(row, 0)->data(Qt::UserRole).toString();
    CronJob* job = m_cronManager->getJob(jobId);
    
    if (job) {
        // Reset next run to now to trigger immediate execution
        job->nextRun = QDateTime::currentDateTime().addSecs(-1);
        m_cronManager->updateJob(*job);
        onLogMessage(QString("Manually triggered: %1").arg(job->name));
    }
}

void MainWindow::toggleJobEnabled()
{
    int row = m_jobTable->currentRow();
    if (row < 0) return;
    
    QString jobId = m_jobTable->item(row, 0)->data(Qt::UserRole).toString();
    CronJob* job = m_cronManager->getJob(jobId);
    
    if (job) {
        job->enabled = !job->enabled;
        m_cronManager->updateJob(*job);
    }
}

void MainWindow::toggleAutoStart()
{
    CronManager::setAutoStart(m_autoStartCheck->isChecked());
}

void MainWindow::onJobExecuted(const QString& jobId, bool success, const QString& output)
{
    CronJob* job = m_cronManager->getJob(jobId);
    QString jobName = job ? job->name : jobId;
    
    QString statusIcon = success ? "[OK]" : "[FAIL]";
    
    // Log the job completion status
    onLogMessage(QString("%1 %2 completed").arg(statusIcon, jobName));
    
    // Log the full output if there is any
    if (!output.trimmed().isEmpty()) {
        m_logView->append(QString("--- Output from %1 ---").arg(jobName));
        m_logView->append(output.trimmed());
        m_logView->append("--- End of output ---");
        m_logView->append("");  // Empty line for spacing
    }
    
    // Scroll to bottom to show latest
    m_logView->verticalScrollBar()->setValue(m_logView->verticalScrollBar()->maximum());
    
    // Show tray notification (keep this short)
    QString notifyText = output.trimmed().isEmpty() ? "Completed" : output.left(100);
    m_trayIcon->showMessage(
        success ? "Job Completed" : "Job Failed",
        QString("%1: %2").arg(jobName, notifyText),
        success ? QSystemTrayIcon::Information : QSystemTrayIcon::Warning,
        3000
    );
    
    refreshJobTable();
}

void MainWindow::onJobsChanged()
{
    refreshJobTable();
    updateStatusBar();
}

void MainWindow::onLogMessage(const QString& message)
{
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    m_logView->append(QString("[%1] %2").arg(timestamp, message));
}

void MainWindow::refreshJobTable()
{
    m_jobTable->setRowCount(0);
    
    const QVector<CronJob>& jobs = m_cronManager->getJobs();
    
    for (const CronJob& job : jobs) {
        int row = m_jobTable->rowCount();
        m_jobTable->insertRow(row);
        
        QTableWidgetItem* nameItem = new QTableWidgetItem(job.name);
        nameItem->setData(Qt::UserRole, job.id);
        m_jobTable->setItem(row, 0, nameItem);
        
        QTableWidgetItem* scriptItem = new QTableWidgetItem(job.scriptPath);
        scriptItem->setToolTip(job.scriptPath);
        m_jobTable->setItem(row, 1, scriptItem);
        
        m_jobTable->setItem(row, 2, new QTableWidgetItem(formatInterval(job.intervalMinutes)));
        m_jobTable->setItem(row, 3, new QTableWidgetItem(formatDateTime(job.lastRun)));
        m_jobTable->setItem(row, 4, new QTableWidgetItem(formatDateTime(job.nextRun)));
        
        QString status = job.enabled ? "Enabled" : "Disabled";
        QTableWidgetItem* statusItem = new QTableWidgetItem(status);
        statusItem->setForeground(job.enabled ? Qt::darkGreen : Qt::gray);
        m_jobTable->setItem(row, 5, statusItem);
    }
}

void MainWindow::updateStatusBar()
{
    int total = m_cronManager->getJobs().size();
    int enabled = 0;
    for (const CronJob& job : m_cronManager->getJobs()) {
        if (job.enabled) enabled++;
    }
    m_statusLabel->setText(QString("Jobs: %1 total, %2 enabled | Scheduler: Running").arg(total).arg(enabled));
}

QString MainWindow::formatInterval(int minutes)
{
    if (minutes >= 1440) {
        int days = minutes / 1440;
        return QString("%1 day%2").arg(days).arg(days > 1 ? "s" : "");
    } else if (minutes >= 60) {
        int hours = minutes / 60;
        return QString("%1 hour%2").arg(hours).arg(hours > 1 ? "s" : "");
    } else {
        return QString("%1 minute%2").arg(minutes).arg(minutes > 1 ? "s" : "");
    }
}

QString MainWindow::formatDateTime(const QDateTime& dt)
{
    if (!dt.isValid()) {
        return "Never";
    }
    return dt.toString("yyyy-MM-dd hh:mm:ss");
}
