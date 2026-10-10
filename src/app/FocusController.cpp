#include "FocusController.h"
#include "WorkHistoryStore.h"
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSaveFile>
#include <QStandardPaths>
#include <QUuid>
#include <algorithm>

// 新账本与旧任务仓库分离，损坏文件只读保护，不覆盖用户数据。
FocusController::FocusController(WorkHistoryStore *history, std::function<QDateTime()> clock, QObject *parent)
    : QObject(parent), history_(history), clock_(std::move(clock)) {
    path_=QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+"/focus.json";
    QFile file(path_);
    if (file.exists()) {
        QJsonParseError parse;
        if (!file.open(QIODevice::ReadOnly)) { writable_=false; error_=QStringLiteral("无法读取专注记录"); }
        else {
            const auto doc=QJsonDocument::fromJson(file.readAll(), &parse);
            if (parse.error!=QJsonParseError::NoError || !doc.isObject() || doc.object().value("version").toInt()!=1) {
                writable_=false; error_=QStringLiteral("专注记录格式异常，已保护原文件");
            } else data_=doc.object();
        }
    }
    if (data_.isEmpty()) data_=QJsonObject{{"version",1},{"settings",QJsonObject{{"focus",25},{"short",5},{"long",15},{"rounds",4},{"strict",false}}}};
    round_=qMax(0,data_.value("round").toInt());
    const auto saved=data_.value("active").toObject();
    if (!saved.isEmpty()) {
        active_=saved; phase_=saved.value("phase").toString("focus");
        if (phase_!="focus" && phase_!="short" && phase_!="long") phase_="focus";
        remainingMs_=qBound<qint64>(0,qint64(saved.value("remaining").toDouble()),qint64(180)*60000);
        spentMs_=qMax<qint64>(0,qint64(saved.value("spent").toDouble()));
        waitingMs_=qMax<qint64>(0,qint64(saved.value("waiting").toDouble()));
        round_=qBound(0,saved.value("round").toInt(),12); state_="paused";
    }
    connect(history_, &WorkHistoryStore::tasksChanged,this,&FocusController::tasksChanged);
    connect(&pulse_, &QTimer::timeout,this,&FocusController::tick);
    elapsed_.start(); pulse_.start(250);
}
// 退出保留最后检查点，恢复时暂停，不补算离线时间。
FocusController::~FocusController() { checkpoint(); }
void FocusController::checkpoint() {
    if(!engaged())return;
    auto saved=active_; saved["phase"]=phase_; saved["remaining"]=double(remainingMs_); saved["spent"]=double(spentMs_); saved["waiting"]=double(waitingMs_); saved["round"]=round_;
    auto next=data_; next["active"]=saved; if(!commit(next))state_="paused";
}
QString FocusController::date() const { return clock_().date().toString(Qt::ISODate); }
QString FocusController::countdown() const {
    const auto sec=(remainingMs_+999)/1000;
    return QStringLiteral("%1:%2").arg(sec/60,2,10,QChar('0')).arg(sec%60,2,10,QChar('0'));
}
// 设置仅对下一阶段生效，进行中计时保持原长度。
int FocusController::secondsFor(const QString &phase) const {
    const int fallback=phase=="focus"?25:phase=="short"?5:15;
    return qBound(1,settings().value(phase,fallback).toInt(),180)*60;
}
bool FocusController::commit(const QJsonObject &next) {
    if (!writable_) { emit changed(); return false; }
    QDir().mkpath(QFileInfo(path_).absolutePath());
    QSaveFile file(path_);
    const auto bytes=QJsonDocument(next).toJson();
    if (!file.open(QIODevice::WriteOnly) || file.write(bytes)!=bytes.size() || !file.commit()) {
        error_=QStringLiteral("专注记录保存失败，请检查磁盘空间与权限"); emit changed(); return false;
    }
    data_=next; error_.clear(); emit changed(); return true;
}
bool FocusController::configure(int focus,int shortBreak,int longBreak,int rounds,bool strict) {
    if (engaged()) { emit message(QStringLiteral("请结束当前轮次后修改设置")); return false; }
    if(focus<1||focus>180||shortBreak<1||shortBreak>60||longBreak<1||longBreak>120||rounds<1||rounds>12) return false;
    auto next=data_; next["settings"]=QJsonObject{{"focus",focus},{"short",shortBreak},{"long",longBreak},{"rounds",rounds},{"strict",strict}};
    return commit(next);
}
// 任务使用旧仓库 UUID 关联，今日计划以网络日期标记，不改旧存档结构。
bool FocusController::planTask(const QString &id,int estimate,int priority,bool today) {
    bool exists=false; for(const auto &v:history_->activityTasks()) if(v.toMap().value("id").toString()==id) exists=true;
    if(!exists||estimate<1||estimate>99||priority<1||priority>3) return false;
    auto next=data_; auto plans=next.value("plans").toObject();
    plans[id]=QJsonObject{{"estimate",estimate},{"priority",priority},{"date",today?date():QString()}};
    next["plans"]=plans; const bool ok=commit(next); if(ok)emit tasksChanged(); return ok;
}
QVariantList FocusController::tasks() const {
    QVariantList result;
    const auto plans=data_.value("plans").toObject();
    for(const auto &value:history_->activityTasks()) {
        auto task=value.toMap(); const auto id=task.value("id").toString(); const auto plan=plans.value(id).toObject();
        task["estimate"]=plan.value("estimate").toInt(1); task["priority"]=plan.value("priority").toInt(2);
        task["planned"]=plan.value("date").toString()==date(); int complete=0,seconds=0;
        for(const auto &v:data_.value("sessions").toArray()) { const auto s=v.toObject(); if(s.value("taskId").toString()==id) { complete+=s.value("complete").toBool(); seconds+=s.value("seconds").toInt(); } }
        task["complete"]=complete; task["minutes"]=seconds/60; result.append(task);
    }
    std::stable_sort(result.begin(),result.end(),[](const QVariant &a,const QVariant &b) {
        const auto x=a.toMap(),y=b.toMap(); if(x.value("planned")!=y.value("planned")) return x.value("planned").toBool();
        return x.value("priority").toInt()<y.value("priority").toInt();
    }); return result;
}
// 每次仅推进当前阶段，结束休息后等待用户主动开始下一轮。
void FocusController::start(const QString &id) {
    if(engaged()||!writable_) return;
    QString title; for(const auto &v:tasks()) if(v.toMap().value("id").toString()==id) title=v.toMap().value("title").toString();
    if(!id.isEmpty()&&title.isEmpty()) return;
    active_=QJsonObject{{"id",QUuid::createUuid().toString(QUuid::WithoutBraces)},{"taskId",id},{"title",title.isEmpty()?QStringLiteral("自由专注"):title},{"date",date()},{"internal",0},{"external",0},{"pauses",0}};
    phase_="focus"; remainingMs_=secondsFor(phase_)*1000LL; spentMs_=waitingMs_=0; state_="focus"; elapsed_.restart(); tick(); checkpoint();
}
void FocusController::pause() {
    if(state_=="focus" && settings().value("strict").toBool()) { interrupt(false,QStringLiteral("严格模式暂停"),true); return; }
    if(state_=="focus"||state_=="short"||state_=="long"||state_=="waiting") {
        tick(); if(!engaged())return; active_["pauses"]=active_.value("pauses").toInt()+1; state_="paused"; tick(); checkpoint();
    }
}
void FocusController::wait() {
    if(state_!="focus") return;
    if(settings().value("strict").toBool()) { interrupt(false,QStringLiteral("进入等待"),true); return; }
    tick(); if(state_=="focus") { state_="waiting"; tick(); checkpoint(); }
}
void FocusController::resume() {
    if(state_!="paused"&&state_!="waiting")return;
    elapsed_.restart(); state_=phase_; tick();
}
void FocusController::finish() {
    if(!engaged())return;
    tick();
    if(phase_=="focus"&&!settle(false))return;
    auto next=data_; next.remove("active"); next["round"]=round_; if(commit(next)) { state_="idle"; active_={}; remainingMs_=0; emit changed(); }
}
// 分心记号与真正中断分开，想法收件箱不会停止计时。
void FocusController::interrupt(bool external,const QString &note,bool stop) {
    if(phase_!="focus"||!engaged()) return;
    tick(); if(phase_!="focus"||!engaged())return;
    const auto key=external?"external":"internal"; active_[key]=active_.value(key).toInt()+1;
    if(!note.trimmed().isEmpty()) capture(note);
    if(stop)finish(); else tick();
}
void FocusController::capture(const QString &note) {
    if(note.trimmed().isEmpty()||note.size()>300)return;
    auto next=data_; auto notes=next.value("thoughts").toArray();
    notes.append(QJsonObject{{"id",QUuid::createUuid().toString(QUuid::WithoutBraces)},{"date",date()},{"text",note.trimmed()}});
    next["thoughts"]=notes; if(commit(next))emit thoughtsChanged();
}
void FocusController::removeThought(const QString &id) {
    auto next=data_; auto notes=next.value("thoughts").toArray();
    for(int i=notes.size()-1;i>=0;--i)if(notes[i].toObject().value("id").toString()==id)notes.removeAt(i);
    next["thoughts"]=notes; if(commit(next))emit thoughtsChanged();
}
void FocusController::review(const QString &result,const QString &adjustment) {
    if(result.size()>1000||adjustment.size()>1000)return;
    auto next=data_; auto reviews=next.value("reviews").toObject(); reviews[date()]=QJsonObject{{"result",result},{"adjustment",adjustment}};
    next["reviews"]=reviews; commit(next);
}
// 唯一轮次 ID 防止保存后崩溃导致重复入账；失败时暂停待重试。
bool FocusController::settle(bool complete) {
    auto next=data_; auto sessions=next.value("sessions").toArray();
    for(const auto &v:sessions)if(v.toObject().value("id")==active_.value("id"))return true;
    auto record=active_; record["seconds"]=int(spentMs_/1000); record["waitingSeconds"]=int(waitingMs_/1000); record["complete"]=complete;
    sessions.append(record); next["sessions"]=sessions; next.remove("active");
    if(!commit(next)) { state_="paused"; return false; } emit recordsChanged(); emit tasksChanged(); return true;
}
void FocusController::tick() {
    const auto delta=elapsed_.restart();
    if(delta>5000 && (state_=="focus"||state_=="short"||state_=="long"||state_=="waiting")) {
        state_="paused"; emit message(QStringLiteral("检测到休眠或长时间停顿，计时已暂停，请继续或结束"));
    } else if(state_=="focus"||state_=="short"||state_=="long") {
        const auto counted=qMin(remainingMs_,delta); remainingMs_-=counted; if(state_=="focus")spentMs_+=counted;
        if(remainingMs_<=0) {
            if(phase_=="focus") {
                if(!settle(true)) { emit changed(); return; }
                ++round_; const int rounds=qBound(1,settings().value("rounds",4).toInt(),12);
                phase_=round_%rounds==0?"long":"short"; remainingMs_=secondsFor(phase_)*1000LL; state_=phase_;
                emit message(QStringLiteral("本轮专注完成，起来活动一下吧"));
            } else { state_="idle"; active_={}; auto next=data_; next.remove("active");next["round"]=round_;commit(next); emit message(QStringLiteral("休息结束，可以选择下一项任务")); }
        }
    } else if(state_=="waiting") waitingMs_+=delta;
    if(engaged() && ++checkpointTicks_%20==0) checkpoint();
    emit changed();
}
QVariantList FocusController::recentSessions() const {
    QVariantList result; const auto sessions=data_.value("sessions").toArray();
    for(int i=sessions.size()-1;i>=0&&result.size()<20;--i)result.append(sessions[i].toObject().toVariantMap()); return result;
}
QVariantList FocusController::thoughts() const { return data_.value("thoughts").toArray().toVariantList(); }
QVariantMap FocusController::summary() const {
    int complete=0,seconds=0,waiting=0,internal=0,external=0;
    for(const auto &v:data_.value("sessions").toArray()) { const auto s=v.toObject(); if(s.value("date").toString()!=date())continue;
        complete+=s.value("complete").toBool(); seconds+=s.value("seconds").toInt();waiting+=s.value("waitingSeconds").toInt();internal+=s.value("internal").toInt();external+=s.value("external").toInt(); }
    const auto r=data_.value("reviews").toObject().value(date()).toObject();
    return {{"complete",complete},{"minutes",seconds/60},{"waiting",waiting/60},{"internal",internal},{"external",external},{"result",r.value("result").toString()},{"adjustment",r.value("adjustment").toString()}};
}
