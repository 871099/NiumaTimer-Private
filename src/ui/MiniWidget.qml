// 轻量桌面挂件：共用真实快照，托盘负责恢复穿透和隐藏状态。
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Window {
    id: widget
    required property var timerController
    required property var focusModel
    required property var sound
    property bool showFocus: false
    required property var desktop
    visible: desktop.available && desktop.miniVisible
    width: 330 * desktop.miniScale
    height: 250 * desktop.miniScale
    title: qsTr("牛马桌面挂件")
    // 整个挂件一起淡化，主窗口保持原有显示。
    opacity: desktop.miniOpacity
    color: "transparent"
    flags: Qt.Tool | Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint
    onXChanged: positionSave.restart()
    onYChanged: positionSave.restart()
    Timer { id: positionSave; interval: 600; onTriggered: widget.desktop.rememberMiniPosition() }
    Item {
        width: 330; height: 250
        scale: widget.desktop.miniScale; transformOrigin: Item.TopLeft
        Rectangle { anchors.fill: parent; radius: 12; color: "#f0081738"; border.width: 2; border.color: "#18d7ff" }
        MouseArea {
            anchors.fill: parent
            enabled: !widget.desktop.locked
            onPressed: widget.startSystemMove()
            onDoubleClicked: widget.desktop.showMain()
        }
        Row {
            x: 12; y: 8; spacing: 6
            Label { text: qsTr("牛马陪你等下班"); color: "#d6f5ff"; font.bold: true; width: 190 }
            Button { text: widget.desktop.privacy ? "◉" : "◌"; width: 30; height: 25; onClicked: widget.desktop.privacy = !widget.desktop.privacy; ToolTip.text: qsTr("隐藏 / 显示工资"); ToolTip.visible: hovered }
            Button { text: "×"; width: 30; height: 25; onClicked: widget.desktop.miniVisible = false }
        }
        AnimatedSprite {
            id: mascot
            x: 1; y: 37; width: 122; height: 122
            source: "qrc:/qt/qml/NiumaTimer/assets/cow-fatigue-spritesheet.png"
            frameWidth: 512; frameHeight: 512; frameCount: 6
            currentFrame: Math.round(widget.timerController.progress * 5)
            running: false; paused: true; smooth: false; interpolate: false
            // 现有姿态增加轻微呼吸节奏，完整逐帧动作素材单独制作。
            SequentialAnimation on scale {
                running: widget.visible && widget.desktop.animations; loops: Animation.Infinite
                NumberAnimation { to: 1.025; duration: 1100; easing.type: Easing.InOutSine }
                NumberAnimation { to: 1; duration: 1100; easing.type: Easing.InOutSine }
            }
        }
        Column {
            x: 124; y: 48; width: 190; spacing: 7
            Label { text: widget.showFocus ? "专注 · " + widget.focusModel.state : widget.timerController.countdownLabel; color: "#acc6ef"; font.pixelSize: 12 }
            Label { text: widget.showFocus ? widget.focusModel.countdown : widget.timerController.countdownText; color: "#00f5a0"; font.pixelSize: 29; font.bold: true; font.family: "Consolas" }
            Label { text: widget.desktop.privacy ? qsTr("今日收入已隐藏") : qsTr("今日 ¥ %1").arg(widget.timerController.earnedText); color: "#ffe27a"; font.pixelSize: 16; font.bold: true }
            Label { text: qsTr("Lv.%1 · EXP %2 / 500").arg(widget.timerController.experienceLevel).arg(widget.timerController.experiencePoints); color: "#b8d8ff"; font.pixelSize: 11 }
        }
        Column {
            x: 14; y: 166; width: 302; spacing: 5
            Row { spacing: 8; Label { text: "HP"; color: "#ff9caa"; width: 26 } PixelMeter { width: 264; height: 12; value: 1-widget.timerController.progress; fillColor: "#ff4862" } }
            Row { spacing: 8; Label { text: "XP"; color: "#88c7ff"; width: 26 } PixelMeter { width: 264; height: 12; value: widget.timerController.experienceProgress; fillColor: "#168cff" } }
            Label { width: 302; text: widget.timerController.scheduleReason; color: "#a8c3ed"; font.pixelSize: 10; elide: Text.ElideRight }
        }
        Button { x: 240; y: 221; width: 76; height: 23; text: qsTr("主窗口"); onClicked: widget.desktop.showMain() }
        Button { x:14; y:220; width:90; height:26; text:widget.showFocus?"看下班":"看专注";onClicked:widget.showFocus=!widget.showFocus }
        Button { x:110; y:220; width:100; height:26; text:widget.sound.playing?"静音":"播放声音";enabled:widget.sound.available;onClicked:widget.sound.toggle() }
        Label { visible:false; x: 14; y: 226; text: widget.desktop.locked ? qsTr("位置已锁定 · 托盘可解锁") : qsTr("拖动移动 · 双击打开"); color: "#a6bddf"; font.pixelSize: 10 }
    }
}
