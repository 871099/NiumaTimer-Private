// 原生顶层窗口保留系统标题栏；共享应用壳负责桌面和手机的页面导航。
import QtQuick
import QtQuick.Controls

ApplicationWindow {
    id: window
    visible: true
    width: 1180
    height: 790
    minimumWidth: 360
    minimumHeight: 600
    title: isolatedPreview ? qsTr("牛马打工计时器 · 隔离预览（非原记录）") : qsTr("牛马打工计时器")
    property bool isolatedPreview: Qt.application.arguments.indexOf("--isolated-preview") >= 0
    color: Theme.background
    // qmllint disable unqualified
    property var appController: workTimer
    property var historyStore: workHistory
    property var desktop: desktopController
    property var focusModel: focusController
    property var sound: soundController
    // qmllint enable unqualified
    // 置顶只用于 Windows 桌面窗口，Android 不创建跨应用悬浮窗。
    flags: Qt.Window | (Qt.platform.os === "windows" && appController.windowPinned
                        ? Qt.WindowStaysOnTopHint : 0)

    AppShell {
        anchors.fill: parent
        timerController: window.appController
        historyStore: window.historyStore
        desktop: window.desktop
        focusModel: window.focusModel
        sound: window.sound
    }
    // 有托盘入口才拦截关闭，防止没有恢复入口的后台实例。
    onClosing: function(close) {
        if (desktop.available && desktop.trayAvailable && desktop.closeToTray) {
            close.accepted = false
            desktop.hideMain()
        }
    }
    MiniWidget { objectName: "miniWidget"; timerController: window.appController; desktop: window.desktop; focusModel: window.focusModel; sound: window.sound }
}
