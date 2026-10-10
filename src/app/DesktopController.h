// 桌面能力集中管理，移动端保留安全的空实现。
#pragma once
#include <QObject>
#include <QSettings>
#include <QPointer>
#include <QWindow>
#include <QTimer>
#include <QDateTime>
#include <QVariantList>
#include <QSet>
#ifdef Q_OS_WIN
#include <QSystemTrayIcon>
#include <QMenu>
#endif
class WorkTimer;
class FocusController;
class DesktopController final : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool available READ available CONSTANT)
    Q_PROPERTY(bool trayAvailable READ trayAvailable NOTIFY changed)
    Q_PROPERTY(bool miniVisible READ miniVisible WRITE setMiniVisible NOTIFY changed)
    Q_PROPERTY(bool privacy READ privacy WRITE setPrivacy NOTIFY changed)
    Q_PROPERTY(bool locked READ locked WRITE setLocked NOTIFY changed)
    Q_PROPERTY(bool clickThrough READ clickThrough WRITE setClickThrough NOTIFY changed)
    Q_PROPERTY(bool closeToTray READ closeToTray WRITE setCloseToTray NOTIFY changed)
    Q_PROPERTY(bool autoStart READ autoStart WRITE setAutoStart NOTIFY changed)
    Q_PROPERTY(bool reminders READ reminders WRITE setReminders NOTIFY changed)
    Q_PROPERTY(bool restReminders READ restReminders WRITE setRestReminders NOTIFY changed)
    Q_PROPERTY(bool animations READ animations WRITE setAnimations NOTIFY changed)
    Q_PROPERTY(double miniOpacity READ miniOpacity WRITE setMiniOpacity NOTIFY changed)
    Q_PROPERTY(double miniScale READ miniScale WRITE setMiniScale NOTIFY changed)
    Q_PROPERTY(QVariantList incomeGoals READ incomeGoals NOTIFY changed)
public:
    explicit DesktopController(WorkTimer *timer, QObject *parent = nullptr);
    bool available() const;
    bool trayAvailable() const;
    bool miniVisible() const { return miniVisible_; }
    bool privacy() const { return privacy_; }
    bool locked() const { return locked_; }
    bool clickThrough() const { return clickThrough_; }
    bool closeToTray() const { return closeToTray_; }
    bool autoStart() const;
    bool reminders() const { return reminders_; }
    bool restReminders() const { return restReminders_; }
    bool animations() const { return animations_; }
    double miniOpacity() const { return miniOpacity_; }
    double miniScale() const { return miniScale_; }
    QVariantList incomeGoals() const { return goals_; }
    Q_INVOKABLE bool addIncomeGoal(const QString &title, double amount);
    Q_INVOKABLE void removeIncomeGoal(const QString &id);
    void setMiniVisible(bool value);
    void setPrivacy(bool value);
    void setLocked(bool value);
    void setClickThrough(bool value);
    void setCloseToTray(bool value);
    void setAutoStart(bool value);
    void setReminders(bool value);
    void setRestReminders(bool value);
    void setAnimations(bool value);
    void setMiniScale(double value);
    void setMiniOpacity(double value);
    void attach(QWindow *main, QWindow *mini);
    Q_INVOKABLE void showMain();
    Q_INVOKABLE void hideMain();
    Q_INVOKABLE void quit();
    Q_INVOKABLE void resetMiniPosition();
    Q_INVOKABLE void rememberMiniPosition();
    Q_INVOKABLE void previewReminder();
    void focusReminder(const QString &text);
    void setFocusController(FocusController *focus) { focus_=focus; }
signals:
    void changed();
    void message(const QString &text);
private:
    void save();
    void notify(const QString &title, const QString &body);
    void checkReminders();
    void updateMiniInput();
    WorkTimer *timer_;
    FocusController *focus_=nullptr;
    QSettings settings_{QStringLiteral("NiumaTimer"), QStringLiteral("Desktop")};
    QPointer<QWindow> main_, mini_;
    bool miniVisible_ = false, privacy_ = false, locked_ = false, clickThrough_ = false;
    bool closeToTray_ = true, reminders_ = true, restReminders_ = false, animations_ = true;
    double miniScale_ = 1.0, miniOpacity_ = 1.0;
    QString lastStatus_, notifiedShift_;
    QDateTime lastCheck_;
    QDateTime lastShiftEnd_;
    int restSeconds_ = 0;
    QTimer reminderTimer_, inputTimer_;
    QVariantList goals_;
    QSet<QString> announcedGoals_;
    bool goalsInitialized_ = false;
#ifdef Q_OS_WIN
    QSystemTrayIcon tray_;
    QMenu menu_;
#endif
};
