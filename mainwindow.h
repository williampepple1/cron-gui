#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QTableWidget>
#include <QTextEdit>
#include <QCloseEvent>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>
#include "cronmanager.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(bool startHidden = false, QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void showWindow();  // Public so it can be called from other instances

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void addJob();
    void editJob();
    void removeJob();
    void runJobNow();
    void toggleJobEnabled();
    
    void onJobExecuted(const QString& jobId, bool success, const QString& output);
    void onJobsChanged();
    void onLogMessage(const QString& message);
    
    void trayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void toggleAutoStart();

private:
    void setupUI();
    void setupSystemTray();
    void refreshJobTable();
    void updateStatusBar();
    QString formatInterval(int minutes);
    QString formatDateTime(const QDateTime& dt);

    Ui::MainWindow *ui;
    CronManager* m_cronManager;
    
    // UI Elements
    QTableWidget* m_jobTable;
    QTextEdit* m_logView;
    QPushButton* m_addButton;
    QPushButton* m_editButton;
    QPushButton* m_removeButton;
    QPushButton* m_runNowButton;
    QCheckBox* m_autoStartCheck;
    QLabel* m_statusLabel;
    
    // System Tray
    QSystemTrayIcon* m_trayIcon;
    QMenu* m_trayMenu;
    QAction* m_showAction;
    QAction* m_quitAction;
    
    bool m_startHidden;
};
#endif // MAINWINDOW_H
