#include "mainwindow.h"

#include <QApplication>
#include <QSharedMemory>
#include <QMessageBox>
#include <QSystemTrayIcon>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    // Set application metadata
    QCoreApplication::setApplicationName("CronGUI");
    QCoreApplication::setOrganizationName("CronGUI");
    QCoreApplication::setApplicationVersion("1.0");
    
    // Check for system tray availability
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        QMessageBox::critical(nullptr, "Cron Job Manager",
            "System tray is not available on this system.\n"
            "The application requires system tray support to run in the background.");
        return 1;
    }
    
    // Single instance check using shared memory
    QSharedMemory sharedMem("CronGUI_SingleInstance_Lock");
    
    if (!sharedMem.create(1)) {
        // Another instance is already running
        QMessageBox::information(nullptr, "Cron Job Manager",
            "Another instance of Cron Job Manager is already running.\n"
            "Check the system tray for the existing instance.");
        return 0;
    }
    
    // Check for --hidden argument (used when starting with Windows)
    bool startHidden = false;
    QStringList args = QCoreApplication::arguments();
    if (args.contains("--hidden") || args.contains("-h")) {
        startHidden = true;
    }
    
    // Don't quit when last window closes (we want to stay in tray)
    QApplication::setQuitOnLastWindowClosed(false);
    
    MainWindow w(startHidden);
    
    if (!startHidden) {
        w.show();
    }
    
    return a.exec();
}
