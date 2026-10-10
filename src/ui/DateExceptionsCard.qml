// 六周排班预览与日期例外，共用业务层的日期校验和持久化。
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Frame {
    id: card
    required property var timerController
    property string selectedDate: ""
    property string result: ""
    Layout.fillWidth: true
    background: Rectangle { radius: 12; color: "#f4faff"; border.color: "#8ab4ed" }
    ColumnLayout {
        width: card.availableWidth; spacing: 8
        Label { text: qsTr("日期排班例外"); color: Theme.textPrimary; font.bold: true; font.pixelSize: 16 }
        Label { Layout.fillWidth: true; text: card.timerController.scheduleReason; color: Theme.cyan; wrapMode: Text.WordWrap }
        Label { Layout.fillWidth: true; text: qsTr("个人例外 > 官方放假/调休 > 固定周/大小周。可修改今天起两年内未结算班次；已结算历史暂不回写。请假按无薪处理。"); wrapMode: Text.WordWrap; color: Theme.textMuted; font.pixelSize: 11 }
        GridLayout {
            Layout.fillWidth: true; columns: 7; columnSpacing: 4; rowSpacing: 4
            Repeater {
                model: card.timerController.calendarDays
                delegate: Button {
                    required property var modelData
                    Layout.fillWidth: true; Layout.preferredHeight: 54
                    text: modelData.label + "\n" + (modelData.kind === "leave" ? qsTr("请假") : modelData.work ? qsTr("上班") : qsTr("休息"))
                    highlighted: card.selectedDate === modelData.date
                    ToolTip.visible: hovered; ToolTip.text: modelData.source + (modelData.work ? " " + modelData.start + "—" + modelData.end : "")
                    onClicked: {
                        card.selectedDate = modelData.date; dateField.text = modelData.date
                        kindBox.currentIndex = ["work","rest","leave","custom"].indexOf(modelData.kind)
                        if (kindBox.currentIndex < 0) kindBox.currentIndex = modelData.work ? 0 : 1
                        startField.text = modelData.start; endField.text = modelData.end; card.result = ""
                    }
                }
            }
        }
        TextField { id: dateField; Layout.fillWidth: true; placeholderText: qsTr("选择日期或输入 YYYY-MM-DD"); inputMask: "0000-00-00" }
        ComboBox { id: kindBox; Layout.fillWidth: true; model: [qsTr("按默认班次上班"),qsTr("休息"),qsTr("请假（无薪）"),qsTr("自定义班次")] }
        RowLayout {
            visible: kindBox.currentIndex === 3; Layout.fillWidth: true
            TextField { id: startField; Layout.fillWidth: true; text: "09:00"; inputMask: "00:00" }
            Label { text: "—" }
            TextField { id: endField; Layout.fillWidth: true; text: "18:00"; inputMask: "00:00" }
        }
        RowLayout {
            Layout.fillWidth: true
            Button { text: qsTr("保存日期安排"); Layout.fillWidth: true; onClicked: card.result = card.timerController.setDateException(dateField.text,["work","rest","leave","custom"][kindBox.currentIndex],startField.text,endField.text) ? qsTr("已保存") : qsTr("未保存，请检查日期或时间；已结算日期不可修改") }
            Button { text: qsTr("恢复默认"); onClicked: card.result = card.timerController.clearDateException(dateField.text) ? qsTr("已恢复默认") : qsTr("未恢复，日期已结算") }
        }
        Label { Layout.fillWidth: true; text: card.result; wrapMode: Text.WordWrap; color: Theme.textSecondary }
    }
}
