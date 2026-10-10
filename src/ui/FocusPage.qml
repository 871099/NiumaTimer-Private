// 专注页负责输入和展示，计时与持久化由独立控制器完成。
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Item {
    id: page
    required property var focusModel
    required property var sound
    property string selectedId: ""
    function stateLabel() {
        const names={idle:"准备专注",focus:"专注中",short:"短休息 · 起身喝水",long:"长休息 · 离开工位",paused:"已暂停 · 可恢复",waiting:"等待编译 / AI · 不计专注"}
        return names[focusModel.state] || focusModel.state
    }
    ScrollView {
        anchors.fill:parent;contentWidth:availableWidth;clip:true
        ColumnLayout {
            width:parent.width-28;x:14;spacing:14
            Label {text:qsTr("牛马专注空间");font.pixelSize:24;font.bold:true;color:Theme.textPrimary}
            Rectangle {
                Layout.fillWidth:true;implicitHeight:340;color:"#0c1e48";radius:12;border.color:"#18cfff";border.width:2
                ColumnLayout {
                    anchors.fill:parent;anchors.margins:16;spacing:8
                    Image { Layout.alignment:Qt.AlignHCenter;Layout.preferredWidth:72;Layout.preferredHeight:72;source:"qrc:/qt/qml/NiumaTimer/assets/worker-cow-mascot.png";smooth:false;opacity:page.focusModel.state==="short"||page.focusModel.state==="long"?.65:1
                        SequentialAnimation on rotation { running:page.focusModel.state==="focus";loops:Animation.Infinite;NumberAnimation {to:3;duration:500} NumberAnimation {to:-3;duration:500} }
                    }
                    Label {Layout.alignment:Qt.AlignHCenter;text:page.stateLabel();color:"#b8e8ff";font.pixelSize:15}
                    Label {Layout.alignment:Qt.AlignHCenter;text:page.focusModel.countdown;font.pixelSize:52;font.bold:true;font.family:"Consolas";color:page.focusModel.state==="focus"?"#00f5a0":"#ffe27a"}
                    Label {Layout.fillWidth:true;horizontalAlignment:Text.AlignHCenter;text:page.focusModel.taskTitle || "选择一个任务，或自由专注";elide:Text.ElideRight;color:"#dceaff"}
                    Flow {
                        Layout.fillWidth:true;Layout.preferredHeight:childrenRect.height;spacing:6
                        Button {text:"开始";enabled:!page.focusModel.engaged;onClicked:page.focusModel.start(page.selectedId)}
                        Button {text:"暂停";enabled:["focus","short","long","waiting"].indexOf(page.focusModel.state)>=0;onClicked:page.focusModel.pause()}
                        Button {text:"继续";enabled:page.focusModel.state==="paused"||page.focusModel.state==="waiting";onClicked:page.focusModel.resume()}
                        Button {text:"等待";enabled:page.focusModel.state==="focus";onClicked:page.focusModel.wait()}
                        Button {text:"结束本轮";enabled:page.focusModel.engaged;onClicked:page.focusModel.finish()}
                    }
                    Label {Layout.fillWidth:true;text:"工资、下班倒计时和上班经验继续按排班推进。休息结束后手动开始下一轮。";wrapMode:Text.WordWrap;color:"#a3bedf";font.pixelSize:11}
                }
            }
            Label {Layout.fillWidth:true;text:page.focusModel.error;color:Theme.red;visible:text.length>0;wrapMode:Text.WordWrap}
            GroupBox {
                title:"活动清单与今日计划";Layout.fillWidth:true
                ColumnLayout {
                    anchors.fill:parent;spacing:8
                    Label {Layout.fillWidth:true;text:"在任务页添加事项，往日未完成任务也会保留。优先级 1 最高；大任务建议拆成可验收的小步骤。";wrapMode:Text.WordWrap;color:Theme.textMuted}
                    Button {text:page.selectedId.length?"切换自由专注":"已选择自由专注";onClicked:page.selectedId=""}
                    Repeater {
                        model:page.focusModel.tasks
                        delegate:Frame {
                            id:taskRow
                            required property var modelData
                            Layout.fillWidth:true
                            ColumnLayout {
                                anchors.fill:parent
                                Label {Layout.fillWidth:true;text:(taskRow.modelData.done?"✓ ":"")+taskRow.modelData.title;color:Theme.textPrimary;wrapMode:Text.WordWrap;font.bold:true}
                                Label {text:"预计 "+taskRow.modelData.estimate+" / 完整 "+taskRow.modelData.complete+" 轮 · 投入 "+taskRow.modelData.minutes+" 分钟";color:Theme.textMuted;font.pixelSize:11}
                                Flow {
                                    Layout.fillWidth:true;Layout.preferredHeight:childrenRect.height;spacing:5
                                    SpinBox {id:estimate;from:1;to:99;value:taskRow.modelData.estimate;editable:true;width:106}
                                    ComboBox {id:priority;model:["优先级 1","优先级 2","优先级 3"];currentIndex:taskRow.modelData.priority-1;width:105}
                                    CheckBox {id:planned;text:"今日";checked:taskRow.modelData.planned}
                                    Button {text:"保存计划";onClicked:page.focusModel.planTask(taskRow.modelData.id,estimate.value,priority.currentIndex+1,planned.checked)}
                                    Button {text:page.selectedId===taskRow.modelData.id?"已选择":"选择";enabled:!page.focusModel.engaged&&!taskRow.modelData.done;onClicked:page.selectedId=taskRow.modelData.id}
                                }
                            }
                        }
                    }
                }
            }
            GroupBox {
                title:"打断与临时想法";Layout.fillWidth:true
                ColumnLayout {
                    anchors.fill:parent
                    TextField {id:thought;Layout.fillWidth:true;maximumLength:300;placeholderText:"先记下来，稍后处理…"}
                    Flow {
                        Layout.fillWidth:true;Layout.preferredHeight:childrenRect.height;spacing:6
                        Button {text:"只记想法";onClicked:{page.focusModel.capture(thought.text);thought.clear()}}
                        Button {text:"内部打断";enabled:page.focusModel.engaged;onClicked:{page.focusModel.interrupt(false,thought.text,stopRound.checked);thought.clear()}}
                        Button {text:"外部打断";enabled:page.focusModel.engaged;onClicked:{page.focusModel.interrupt(true,thought.text,stopRound.checked);thought.clear()}}
                        CheckBox {id:stopRound;text:"真正中断，结束本轮"}
                    }
                    Repeater {
                        model:page.focusModel.thoughts
                        delegate:RowLayout {
                            id:noteRow
                            required property var modelData
                            Layout.fillWidth:true
                            Label {Layout.fillWidth:true;text:noteRow.modelData.date+" · "+noteRow.modelData.text;wrapMode:Text.WordWrap;color:Theme.textSecondary}
                            Button {text:"处理完";onClicked:page.focusModel.removeThought(noteRow.modelData.id)}
                        }
                    }
                }
            }
            GroupBox {
                title:"声音空间 · 原创合成环境声";Layout.fillWidth:true
                ColumnLayout {
                    anchors.fill:parent
                    Label {Layout.fillWidth:true;text:page.sound.available?"离线合成：环境声是氛围模拟，不是真实场景录音。":"当前平台声音后端尚未接入";wrapMode:Text.WordWrap;color:Theme.textMuted}
                    ComboBox {Layout.fillWidth:true;model:["白噪音","合成雨声","合成森林","抽象咖啡馆"];currentIndex:page.sound.scene;onActivated:page.sound.scene=currentIndex}
                    RowLayout {
                        Layout.fillWidth:true
                        Button {text:page.sound.playing?"停止声音":"播放声音";enabled:page.sound.available;onClicked:page.sound.toggle()}
                        Slider {Layout.fillWidth:true;from:0;to:100;stepSize:1;value:page.sound.volume;onMoved:page.sound.volume=Math.round(value)}
                        Label {text:page.sound.volume+"%";color:Theme.textSecondary}
                    }
                    CheckBox {text:"开始 / 恢复专注时播放";checked:page.sound.autoFocus;onClicked:page.sound.autoFocus=checked}
                    CheckBox {text:"离开专注状态时暂停联动声音";checked:page.sound.pauseOnRest;onClicked:page.sound.pauseOnRest=checked}
                    RowLayout {
                        Label {text:"定时关闭";color:Theme.textSecondary}
                        ComboBox {model:["不定时","15 分钟","30 分钟","60 分钟","120 分钟"];onActivated:page.sound.stopAfter([0,15,30,60,120][currentIndex])}
                    }
                    Label {Layout.fillWidth:true;text:page.sound.error;visible:text.length>0;color:Theme.red;wrapMode:Text.WordWrap}
                }
            }
            GroupBox {
                title:"番茄设置";Layout.fillWidth:true;enabled:!page.focusModel.engaged
                ColumnLayout {
                    anchors.fill:parent
                    Flow {
                        Layout.fillWidth:true;Layout.preferredHeight:childrenRect.height;spacing:6
                        Column {Label {text:"专注 / 分钟"} SpinBox {id:focusMinutes;from:1;to:180;value:25;editable:true;width:120}}
                        Column {Label {text:"短休息 / 分钟"} SpinBox {id:shortMinutes;from:1;to:60;value:5;editable:true;width:120}}
                        Column {Label {text:"长休息 / 分钟"} SpinBox {id:longMinutes;from:1;to:120;value:15;editable:true;width:120}}
                        Column {Label {text:"每组轮数"} SpinBox {id:rounds;from:1;to:12;value:4;editable:true;width:120}}
                    }
                    CheckBox {id:strict;text:"严格模式：暂停 / 等待结束本轮，不计完整番茄"}
                    Button {text:"保存设置";onClicked:page.focusModel.configure(focusMinutes.value,shortMinutes.value,longMinutes.value,rounds.value,strict.checked)}
                }
            }
            GroupBox {
                title:"今日简报与复盘";Layout.fillWidth:true
                ColumnLayout {
                    anchors.fill:parent
                    Label {Layout.fillWidth:true;text:"完整 "+page.focusModel.summary.complete+" 轮 · 专注 "+page.focusModel.summary.minutes+" 分钟 · 等待 "+page.focusModel.summary.waiting+" 分钟\n内部打断 "+page.focusModel.summary.internal+" 次 / 外部打断 "+page.focusModel.summary.external+" 次";wrapMode:Text.WordWrap;color:Theme.textPrimary}
                    TextArea {id:result;Layout.fillWidth:true;placeholderText:"今天完成了哪些可检查的结果？";wrapMode:TextEdit.Wrap}
                    TextArea {id:adjustment;Layout.fillWidth:true;placeholderText:"主要打断原因、估算偏差及明天的一项调整";wrapMode:TextEdit.Wrap}
                    Button {text:"保存复盘";onClicked:page.focusModel.review(result.text,adjustment.text)}
                    Repeater {
                        model:page.focusModel.recentSessions
                        delegate:Label {
                            required property var modelData
                            Layout.fillWidth:true;wrapMode:Text.WordWrap;color:Theme.textMuted
                            text:modelData.date+" · "+modelData.title+" · "+(modelData.complete?"完整番茄":"未完成")+" · "+Math.floor(modelData.seconds/60)+" 分钟"
                        }
                    }
                }
            }
            Item {Layout.preferredHeight:20}
        }
    }
    Component.onCompleted:{
        focusMinutes.value=focusModel.settings.focus;shortMinutes.value=focusModel.settings.short;longMinutes.value=focusModel.settings.long;rounds.value=focusModel.settings.rounds;strict.checked=focusModel.settings.strict
        result.text=focusModel.summary.result;adjustment.text=focusModel.summary.adjustment
    }
}
