// 桌面设置立即保存；不在移动端启用系统常驻与穿透。
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Item {
    id: page
    required property var desktop
    ScrollView {
        anchors.fill: parent; contentWidth: availableWidth; clip: true
        ColumnLayout {
            x: 16; width: parent.width-32; spacing: 12
            Label { text: qsTr("桌面陪伴"); font.pixelSize: 23; font.bold: true; color: Theme.textPrimary }
            Label { Layout.fillWidth: true; text: qsTr("设置即时保存。穿透仅作用于挂件展示区，按钮保持可点；托盘可取消穿透，每次启动自动关闭穿透。"); wrapMode: Text.WordWrap; color: Theme.textSecondary }
            Repeater {
                model: [
                    {key:"miniVisible",label:qsTr("显示迷你挂件"),value:page.desktop.miniVisible,desktop:true},
                    {key:"locked",label:qsTr("锁定挂件位置"),value:page.desktop.locked,desktop:true},
                    {key:"clickThrough",label:qsTr("挂件展示区穿透（按钮可点击）"),value:page.desktop.clickThrough,desktop:true},
                    {key:"closeToTray",label:qsTr("关闭主窗口时收起到托盘"),value:page.desktop.closeToTray,desktop:true},
                    {key:"autoStart",label:qsTr("开机启动（进入挂件模式）"),value:page.desktop.autoStart,desktop:true},
                    {key:"privacy",label:qsTr("隐藏工资金额"),value:page.desktop.privacy,desktop:false},
                    {key:"reminders",label:qsTr("下班前 5 分钟及下班提醒"),value:page.desktop.reminders,desktop:true},
                    {key:"restReminders",label:qsTr("连续工作一小时提醒活动"),value:page.desktop.restReminders,desktop:true},
                    {key:"animations",label:qsTr("角色呼吸动效"),value:page.desktop.animations,desktop:false}
                ]
                delegate: Frame {
                    id: option
                    required property var modelData
                    Layout.fillWidth: true
                    background: Rectangle { radius: 8; color: "#f8fcff"; border.color: "#9bbce6" }
                    RowLayout {
                        anchors.fill: parent
                        Label { Layout.fillWidth: true; text: option.modelData.label; wrapMode: Text.WordWrap; color: Theme.textPrimary }
                        Switch {
                            checked: option.modelData.value
                            enabled: !option.modelData.desktop || (page.desktop.available && (option.modelData.key !== "clickThrough" || page.desktop.trayAvailable))
                            onClicked: page.desktop[option.modelData.key] = checked
                        }
                    }
                    implicitHeight: 58
                }
            }
            Label { text: qsTr("挂件缩放：%1%").arg(Math.round(page.desktop.miniScale*100)); color: Theme.textPrimary }
            Slider { Layout.fillWidth: true; from: 0.75; to: 1.5; stepSize: 0.05; value: page.desktop.miniScale; onMoved: page.desktop.miniScale = value; enabled: page.desktop.available }
            Label { text: qsTr("挂件透明度：%1%").arg(Math.round((1-page.desktop.miniOpacity)*100)); color: Theme.textPrimary }
            Slider {
                Layout.fillWidth: true; from: 0; to: 80; stepSize: 5
                value: Math.round((1-page.desktop.miniOpacity)*100)
                onMoved: page.desktop.miniOpacity = 1-value/100
                enabled: page.desktop.available
            }
            Label { Layout.fillWidth: true; text: qsTr("0% 为不透明，80% 为最淡；文字、角色与背景一起淡化，拖动即时生效并保存。"); wrapMode: Text.WordWrap; color: Theme.textMuted }
            Flow {
                Layout.fillWidth: true; spacing: 8
                Button { text: qsTr("找回挂件位置"); enabled: page.desktop.available; onClicked: page.desktop.resetMiniPosition() }
                Button { text: qsTr("预览提醒"); enabled: page.desktop.available; onClicked: page.desktop.previewReminder() }
                Button { text: qsTr("退出程序"); onClicked: page.desktop.quit() }
            }
            Label { Layout.fillWidth: true; text: qsTr("系统通知是否显示取决于 Windows 通知和勿扰设置。开机启动后请保留程序所在目录。"); wrapMode: Text.WordWrap; color: Theme.textMuted }
            Item { Layout.preferredHeight: 10 }
        }
    }
}
