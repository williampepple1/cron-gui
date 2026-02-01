#include "mainwindow.h"

#include <QApplication>
#include <QSharedMemory>
#include <QMessageBox>
#include <QSystemTrayIcon>
#include <QLocalServer>
#include <QLocalSocket>

const QString SERVER_NAME = "CronGUI_LocalServer";

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
        // Another instance is already running - try to activate it
        QLocalSocket socket;
        socket.connectToServer(SERVER_NAME);
        if (socket.waitForConnected(1000)) {
            // Send "show" command to existing instance
            socket.write("show");
            socket.waitForBytesWritten(1000);
            socket.disconnectFromServer();
        }
        // Exit silently - the existing instance will show itself
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
    
    // Set up local server to listen for activation requests from other instances
    QLocalServer::removeServer(SERVER_NAME);  // Clean up any stale server
    QLocalServer server;
    if (server.listen(SERVER_NAME)) {
        QObject::connect(&server, &QLocalServer::newConnection, [&w, &server]() {
            QLocalSocket* socket = server.nextPendingConnection();
            if (socket) {
                socket->waitForReadyRead(1000);
                QByteArray data = socket->readAll();
                if (data == "show") {
                    w.showWindow();
                }
                socket->disconnectFromServer();
                socket->deleteLater();
            }
        });
    }
    
    if (!startHidden) {
        w.show();
    }
    
    return a.exec();
}
