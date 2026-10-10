// 独立专注账本：不写入工资、排班或上班天数经验。
#pragma once
#include <QObject>
#include <QDateTime>
#include <QJsonObject>
#include <QElapsedTimer>
#include <QTimer>
#include <QVariantList>
#include <functional>
class WorkHistoryStore;
class FocusController final : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString state READ state NOTIFY changed)
    Q_PROPERTY(QString countdown READ countdown NOTIFY changed)
    Q_PROPERTY(QString taskTitle READ taskTitle NOTIFY changed)
    Q_PROPERTY(QVariantList tasks READ tasks NOTIFY tasksChanged)
    Q_PROPERTY(QVariantList recentSessions READ recentSessions NOTIFY recordsChanged)
    Q_PROPERTY(QVariantList thoughts READ thoughts NOTIFY thoughtsChanged)
    Q_PROPERTY(QVariantMap summary READ summary NOTIFY changed)
    Q_PROPERTY(QVariantMap settings READ settings NOTIFY changed)
    Q_PROPERTY(QString error READ error NOTIFY changed)
    Q_PROPERTY(bool engaged READ engaged NOTIFY changed)
public:
    FocusController(WorkHistoryStore *history, std::function<QDateTime()> clock, QObject *parent=nullptr);
    ~FocusController() override;
    QString state() const { return state_; }
    QString countdown() const;
    QString taskTitle() const { return active_.value("title").toString(); }
    QVariantList tasks() const;
    QVariantList recentSessions() const;
    QVariantList thoughts() const;
    QVariantMap summary() const;
    QVariantMap settings() const { return data_.value("settings").toObject().toVariantMap(); }
    QString error() const { return error_; }
    bool engaged() const { return state_ != "idle"; }
    Q_INVOKABLE bool configure(int focus, int shortBreak, int longBreak, int rounds, bool strict);
    Q_INVOKABLE bool planTask(const QString &id, int estimate, int priority, bool today);
    Q_INVOKABLE void start(const QString &id);
    Q_INVOKABLE void pause();
    Q_INVOKABLE void wait();
    Q_INVOKABLE void resume();
    Q_INVOKABLE void finish();
    Q_INVOKABLE void interrupt(bool external, const QString &note, bool stop);
    Q_INVOKABLE void capture(const QString &note);
    Q_INVOKABLE void removeThought(const QString &id);
    Q_INVOKABLE void review(const QString &result, const QString &adjustment);
signals:
    void changed();
    void tasksChanged();
    void recordsChanged();
    void thoughtsChanged();
    void message(const QString &text);
private:
    // 原子提交失败保留旧账本；每秒按单调时钟计时，休眠空档不补算。
    bool commit(const QJsonObject &next);
    void tick();
    void checkpoint();
    bool settle(bool complete);
    int secondsFor(const QString &phase) const;
    QString date() const;
    WorkHistoryStore *history_;
    std::function<QDateTime()> clock_;
    QJsonObject data_, active_;
    QString path_, state_="idle", phase_="focus", error_;
    bool writable_=true;
    qint64 remainingMs_=0, spentMs_=0, waitingMs_=0;
    int round_=0, checkpointTicks_=0;
    QElapsedTimer elapsed_;
    QTimer pulse_;
};
