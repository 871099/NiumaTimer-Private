// 应用入口：初始化 Qt Quick、工作计时控制器并加载跨平台 QML 界面。
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>

#include <cstdlib>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>
#include <QLockFile>
#include <QLocalServer>
#include <QLocalSocket>
#include <QCryptographicHash>
#include <QMutex>
#include <QMutexLocker>
#include <QDebug>
#ifdef Q_OS_WIN
#include <QApplication>
#include <QMessageBox>
#endif

#include "app/WorkTimer.h"
#include "app/WorkHistoryStore.h"
#include "app/DesktopController.h"
#include "app/FocusController.h"
#include "app/SoundController.h"

// 有界启动日志不包含工资和任务内容，QML 加载失败时可据此定位。
static QString startupLogPath;
static void writeStartupLog(QtMsgType type, const QMessageLogContext &, const QString &message) {
    static QMutex mutex;
    QMutexLocker guard(&mutex);
    QFile file(startupLogPath);
    if (file.open(QIODevice::Append | QIODevice::Text)) {
        const QByteArray line = (QDateTime::currentDateTime().toString(Qt::ISODate) + " [" + QString::number(type) + "] " + message + '\n').toUtf8();
        file.write(line);
    }
    if (type == QtFatalMsg) std::abort();
}

int main(int argc, char *argv[]) {
#ifdef Q_OS_WIN
    QApplication app(argc, argv);
#else
    QGuiApplication app(argc, argv);
#endif
    QCoreApplication::setOrganizationName(QStringLiteral("NiumaTimer"));
    QCoreApplication::setApplicationName(QStringLiteral("牛马打工计时器"));
    // 隔离预览使用独立业务数据目录，不触碰正在运行版本的工资与历史。
    if (app.arguments().contains("--isolated-preview"))
        QCoreApplication::setApplicationName(QStringLiteral("牛马打工计时器-专注预览"));
    QCoreApplication::setApplicationVersion(QStringLiteral("0.1.0"));
    const auto dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataDir);
    startupLogPath = QDir(dataDir).filePath("startup.log");
    if (QFileInfo(startupLogPath).size() > 1024*1024) {
        QFile::remove(startupLogPath + ".previous");
        QFile::rename(startupLogPath, startupLogPath + ".previous");
    }
    qInstallMessageHandler(writeStartupLog);
    // 同一用户只运行一个实例，第二次启动唤回现有窗口，避免并发覆盖存档。
    QLockFile instanceLock(QDir(dataDir).filePath("instance.lock"));
    instanceLock.setStaleLockTime(0);
    QLocalServer instanceServer;
#ifdef Q_OS_WIN
    const auto serverName = "NiumaTimer-" + QString::fromLatin1(QCryptographicHash::hash(dataDir.toUtf8(), QCryptographicHash::Sha256).toHex().left(20));
    if (!instanceLock.tryLock(0)) {
        QLocalSocket socket;
        socket.connectToServer(serverName);
        if (socket.waitForConnected(1500)) { socket.write("show"); socket.waitForBytesWritten(1000); }
        else QMessageBox::information(nullptr, QStringLiteral("牛马打工计时器"), QStringLiteral("程序已在运行，请从托盘打开；如刚启动，请稍后重试。"));
        return 0;
    }
    QLocalServer::removeServer(serverName);
    instanceServer.listen(serverName);
#endif

    QQuickStyle::setStyle(QStringLiteral("Basic"));

    WorkHistoryStore workHistory;
    WorkTimer timer;
    timer.setHistoryStore(&workHistory);
    DesktopController desktop(&timer);
    FocusController focus(&workHistory, [&timer] { return timer.currentDateTime(); });
    SoundController sound;
    QObject::connect(&focus, &FocusController::changed, &sound, [&] { sound.followFocus(focus.state()); });
    QObject::connect(&focus, &FocusController::message, &desktop, &DesktopController::focusReminder);
    desktop.setFocusController(&focus);
#ifdef Q_OS_WIN
    app.setQuitOnLastWindowClosed(!desktop.trayAvailable());
#endif
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("workTimer"), &timer);
    engine.rootContext()->setContextProperty(QStringLiteral("workHistory"), &workHistory);
    engine.rootContext()->setContextProperty(QStringLiteral("desktopController"), &desktop);
    engine.rootContext()->setContextProperty(QStringLiteral("focusController"), &focus);
    engine.rootContext()->setContextProperty(QStringLiteral("soundController"), &sound);
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreationFailed,
        &app, [] {
#ifdef Q_OS_WIN
            QMessageBox::critical(nullptr, QStringLiteral("启动失败"), QStringLiteral("界面加载失败。请使用完整解压的发布目录。诊断日志：\n") + startupLogPath);
#endif
            QCoreApplication::exit(EXIT_FAILURE);
        },
        Qt::QueuedConnection);
    engine.loadFromModule(QStringLiteral("NiumaTimer"), QStringLiteral("Main"));
    if (!engine.rootObjects().isEmpty()) {
        auto *mainWindow = qobject_cast<QWindow *>(engine.rootObjects().first());
        desktop.attach(mainWindow, mainWindow->findChild<QWindow *>("miniWidget"));
        qInfo("Startup complete: QML window loaded.");
    }
    QObject::connect(&instanceServer, &QLocalServer::newConnection, &desktop, [&] {
        while (auto *socket = instanceServer.nextPendingConnection()) {
            desktop.showMain(); socket->disconnectFromServer(); socket->deleteLater();
        }
    });

    QObject::connect(&app, &QGuiApplication::applicationStateChanged,
                     &timer, [&timer](Qt::ApplicationState state) {
        if (state == Qt::ApplicationActive) {
            timer.syncNetworkTime();
            timer.refresh();
        }
    });

    return app.exec();
}
