// 将今日估算收入映射到生活目标；隐私模式隐藏金额和进度。
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Frame {
    id: card
    required property var desktop
    required property var timerController
    background: Rectangle { color: "#f4faff"; radius: 8; border.color: "#86ace5" }
    ColumnLayout {
        width: card.availableWidth; spacing: 8
        Label { text: qsTr("今天赚够了什么？"); font.bold: true; color: Theme.textPrimary; font.pixelSize: 16 }
        Repeater {
            model: card.desktop.incomeGoals
            delegate: ColumnLayout {
                id: goalRow
                required property var modelData
                readonly property double progress: Math.min(1,Number(card.timerController.earnedCents)/(100*Number(modelData.amount)))
                Layout.fillWidth: true
                RowLayout {
                    Layout.fillWidth: true
                    Label { Layout.fillWidth:true; text:card.desktop.privacy ? qsTr("生活目标（已隐藏）") : (goalRow.progress >= 1 ? "✓ " : "○ ") + goalRow.modelData.title; color:Theme.textPrimary }
                    Label { text:card.desktop.privacy ? "••••" : "¥ " + Number(goalRow.modelData.amount).toFixed(2); color:Theme.textSecondary }
                    Button { text:"×"; implicitWidth:30; implicitHeight:26; onClicked:card.desktop.removeIncomeGoal(goalRow.modelData.id) }
                }
                ProgressBar { Layout.fillWidth:true; visible:!card.desktop.privacy; value:goalRow.progress }
            }
        }
        Label { visible:card.desktop.incomeGoals.length === 0; text:qsTr("添加一杯咖啡、一份午餐，给今天一个小目标。"); wrapMode:Text.WordWrap; Layout.fillWidth:true; color:Theme.textMuted }
        RowLayout {
            Layout.fillWidth:true
            TextField { id:nameField; Layout.fillWidth:true; placeholderText:qsTr("例如：一杯咖啡"); maximumLength:40 }
            TextField { id:amountField; Layout.preferredWidth:90; placeholderText:qsTr("金额"); validator:DoubleValidator {bottom:0.01;top:10000000;decimals:2;locale:"C"} echoMode:card.desktop.privacy ? TextInput.Password : TextInput.Normal }
            Button { text:qsTr("添加"); onClicked: { if (amountField.acceptableInput && card.desktop.addIncomeGoal(nameField.text,Number(amountField.text))) { nameField.clear();amountField.clear() } } }
        }
    }
}
