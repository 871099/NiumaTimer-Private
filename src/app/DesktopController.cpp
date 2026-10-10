#include "FocusController.h"
#include "DesktopController.h"
#include "WorkTimer.h"
#include <QCoreApplication>
#include <QGuiApplication>
#include <QScreen>
#include <QCursor>
#ifdef Q_OS_WIN
#include <qt_windows.h>
#endif
#include <QDir>
#include <cmath>
#include <QUuid>

// 托盘提供穿透挂件的恢复入口，只有托盘可用时才允许点击穿透。
DesktopController::DesktopController(WorkTimer *timer, QObject *parent)
    : QObject(parent), timer_(timer) {
    miniVisible_ = settings_.value("miniVisible", false).toBool();
    privacy_ = settings_.value("privacy", false).toBool();
    locked_ = settings_.value("locked", false).toBool();
    closeToTray_ = settings_.value("closeToTray", true).toBool();
    reminders_ = settings_.value("reminders", true).toBool();
    restReminders_ = settings_.value("restReminders", false).toBool();
    animations_ = settings_.value("animations", true).toBool();
    const double savedScale = settings_.value("miniScale", 1.0).toDouble();
    miniScale_ = std::isfinite(savedScale) ? qBound(0.75, savedScale, 1.5) : 1.0;
    const double savedOpacity = settings_.value("miniOpacity", 1.0).toDouble();
    miniOpacity_ = std::isfinite(savedOpacity) ? qBound(0.2, savedOpacity, 1.0) : 1.0;
    const auto savedGoals = settings_.value("incomeGoals").toList();
    for (const auto &value : savedGoals) {
        const auto goal = value.toMap(); const auto amount = goal.value("amount").toDouble();
        if (goals_.size() < 20 && !goal.value("id").toString().isEmpty() &&
            !goal.value("title").toString().trimmed().isEmpty() && std::isfinite(amount) && amount > 0 && amount <= 10000000)
            goals_.append(goal);
    }
#ifdef Q_OS_WIN
    tray_.setIcon(QIcon(QStringLiteral(":/qt/qml/NiumaTimer/assets/worker-cow-mascot.png")));
    tray_.setToolTip(QStringLiteral("牛马打工计时器"));
    menu_.addAction(QStringLiteral("打开主窗口"), this, &DesktopController::showMain);
    menu_.addAction(QStringLiteral("显示 / 隐藏挂件"), this, [this]{ setMiniVisible(!miniVisible_); });
    menu_.addAction(QStringLiteral("隐藏 / 显示工资"), this, [this]{ setPrivacy(!privacy_); });
    menu_.addAction(QStringLiteral("取消穿透与锁定"), this, [this]{ setClickThrough(false); setLocked(false); });
    menu_.addAction(QStringLiteral("将挂件移回屏幕"), this, &DesktopController::resetMiniPosition);
    menu_.addSeparator();
    menu_.addAction(QStringLiteral("退出程序"), this, &DesktopController::quit);
    tray_.setContextMenu(&menu_);
    tray_.show();
    connect(&tray_, &QSystemTrayIcon::activated, this, [this](auto reason){
        if (reason == QSystemTrayIcon::DoubleClick || reason == QSystemTrayIcon::Trigger) showMain();
    });
    connect(&tray_, &QSystemTrayIcon::messageClicked, this, &DesktopController::showMain);
#endif
    reminderTimer_.setInterval(1000);
    connect(&reminderTimer_, &QTimer::timeout, this, &DesktopController::checkReminders);
    if (available()) reminderTimer_.start();
    // 穿透窗口收不到鼠标事件，主动追踪光标，为按钮区恢复原生输入。
    inputTimer_.setInterval(16);
    inputTimer_.setTimerType(Qt::PreciseTimer);
    connect(&inputTimer_, &QTimer::timeout, this, &DesktopController::updateMiniInput);
    if (available()) inputTimer_.start();
}
bool DesktopController::available() const {
#ifdef Q_OS_WIN
    return true;
#else
    return false;
#endif
}
bool DesktopController::trayAvailable() const {
#ifdef Q_OS_WIN
    return QSystemTrayIcon::isSystemTrayAvailable();
#else
    return false;
#endif
}
// 每个桌面偏好立即同步；穿透每次启动默认关闭，防止失去控制入口。
void DesktopController::save() {
    settings_.setValue("incomeGoals", goals_);
    settings_.setValue("miniVisible", miniVisible_);
    settings_.setValue("privacy", privacy_);
    settings_.setValue("locked", locked_);
    settings_.setValue("closeToTray", closeToTray_);
    settings_.setValue("reminders", reminders_);
    settings_.setValue("restReminders", restReminders_);
    settings_.setValue("animations", animations_);
    settings_.setValue("miniScale", miniScale_);
    settings_.setValue("miniOpacity", miniOpacity_);
    settings_.sync();
    if (settings_.status() != QSettings::NoError) emit message(QStringLiteral("桌面偏好保存失败，请检查配置权限。"));
    emit changed();
}
void DesktopController::setMiniVisible(bool v) { miniVisible_ = available() && v; save(); }
void DesktopController::setPrivacy(bool v) { privacy_ = v; save(); }
void DesktopController::setLocked(bool v) { locked_ = v; save(); }
void DesktopController::setClickThrough(bool v) { clickThrough_ = v && trayAvailable(); updateMiniInput(); save(); }
void DesktopController::setCloseToTray(bool v) { closeToTray_ = v; save(); }
void DesktopController::setReminders(bool v) { reminders_ = v; save(); }
void DesktopController::setRestReminders(bool v) { restReminders_ = v; restSeconds_ = 0; save(); }
void DesktopController::setAnimations(bool v) { animations_ = v; save(); }
// 挂件整体透明度限制在可见范围，非法数值不会写入配置。
void DesktopController::setMiniOpacity(double value) {
    if (!std::isfinite(value)) return;
    const double bounded = qBound(0.2, value, 1.0);
    if (qFuzzyCompare(miniOpacity_, bounded)) return;
    miniOpacity_ = bounded;
    save();
}
void DesktopController::setMiniScale(double v) { if (std::isfinite(v)) { miniScale_ = qBound(0.75, v, 1.5); save(); } }
// 开机启动只写当前用户的 Run 项，不申请管理员权限。
bool DesktopController::autoStart() const {
#ifdef Q_OS_WIN
    QSettings run("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
    return run.value("NiumaTimer").toString() == QStringLiteral("\"%1\" --mini").arg(QDir::toNativeSeparators(QCoreApplication::applicationFilePath()));
#else
    return false;
#endif
}
void DesktopController::setAutoStart(bool v) {
#ifdef Q_OS_WIN
    QSettings run("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
    if (v) run.setValue("NiumaTimer", QStringLiteral("\"%1\" --mini").arg(QDir::toNativeSeparators(QCoreApplication::applicationFilePath())));
    else run.remove("NiumaTimer");
    run.sync();
    if (run.status() != QSettings::NoError) emit message(QStringLiteral("开机启动设置失败。"));
#else
    Q_UNUSED(v);
#endif
    emit changed();
}
void DesktopController::attach(QWindow *main, QWindow *mini) {
    main_ = main; mini_ = mini;
    if (!mini_) return;
    const QPoint saved(settings_.value("miniX", -10000).toInt(), settings_.value("miniY", -10000).toInt());
    bool visible = false;
    for (auto *screen : QGuiApplication::screens())
        if (screen->availableGeometry().contains(QRect(saved, mini_->size()))) visible = true;
    if (visible) mini_->setPosition(saved); else resetMiniPosition();
    if (QCoreApplication::arguments().contains("--mini") && available()) {
        setMiniVisible(true);
        if (trayAvailable()) main_->hide();
    }
}
void DesktopController::showMain() {
    if (!main_) return;
    main_->showNormal(); main_->raise(); main_->requestActivate();
}
void DesktopController::hideMain() {
    if (main_ && trayAvailable()) main_->hide();
}
void DesktopController::quit() { QCoreApplication::quit(); }
void DesktopController::resetMiniPosition() {
    if (!mini_ || !QGuiApplication::primaryScreen()) return;
    const auto rect = QGuiApplication::primaryScreen()->availableGeometry();
    mini_->setPosition(rect.topRight() - QPoint(mini_->width() + 20, -20));
    rememberMiniPosition();
}
void DesktopController::rememberMiniPosition() {
    if (!mini_) return;
    settings_.setValue("miniX", mini_->x()); settings_.setValue("miniY", mini_->y()); settings_.sync();
}
void DesktopController::notify(const QString &title, const QString &body) {
    emit message(body);
#ifdef Q_OS_WIN
    if (trayAvailable()) tray_.showMessage(title, body, QSystemTrayIcon::Information, 6000);
#endif
}
// 专注提示共用托盘通知；专注期间停用重复的久坐提醒。
void DesktopController::focusReminder(const QString &text) { if(reminders_)notify(QStringLiteral("牛马专注"),text); }
void DesktopController::previewReminder() { notify(QStringLiteral("牛马提醒"), QStringLiteral("提醒预览：再坚持一下，下班就在前方！")); }
// 收入目标只保存名称和金额，不修改工资计算或工作记录。
bool DesktopController::addIncomeGoal(const QString &title, double amount) {
    if (goals_.size() >= 20 || title.trimmed().isEmpty() || title.size() > 40 ||
        !std::isfinite(amount) || amount < 0.01 || amount > 10000000) {
        emit message(QStringLiteral("目标最多 20 项，名称不超过 40 字，金额应在 0 到 1000 万之间。")); return false;
    }
    const auto previous = goals_;
    goals_.append(QVariantMap{{"id",QUuid::createUuid().toString(QUuid::WithoutBraces)}, {"title",title.trimmed()}, {"amount",std::round(amount*100)/100}});
    save();
    if (settings_.status() != QSettings::NoError) { goals_ = previous; emit changed(); return false; }
    return true;
}
void DesktopController::removeIncomeGoal(const QString &id) {
    const auto previous = goals_;
    for (qsizetype i=0; i<goals_.size(); ++i) if (goals_[i].toMap().value("id").toString() == id) { goals_.removeAt(i); break; }
    save();
    if (settings_.status() != QSettings::NoError) { goals_ = previous; emit changed(); }
}
// 以班次结束时间去重；休眠跨度不计作连续久坐，不在启动时补发旧提醒。
void DesktopController::checkReminders() {
    const auto now = timer_->currentDateTime();
    const QString status = timer_->statusKind();
    const auto summary = timer_->liveSummary();
    for (const auto &value : goals_) {
        const auto goal = value.toMap();
        const QString key = timer_->dateText() + "/" + goal.value("id").toString();
        if (timer_->earnedCents() >= qRound64(goal.value("amount").toDouble()*100) && !announcedGoals_.contains(key)) {
            announcedGoals_.insert(key);
            if (goalsInitialized_ && reminders_) notify(QStringLiteral("收入目标达成"),
                privacy_ ? QStringLiteral("今日一个收入目标已达成！") : QStringLiteral("今日已赚够：%1").arg(goal.value("title").toString()));
        }
    }
    goalsInitialized_ = true;
    const QString shift = summary.value("shiftEnd").toString();
    if (reminders_ && status == "work" && timer_->countdownLabel() == QStringLiteral("距离下班还有")) {
        const auto parts = timer_->countdownText().split(':');
        if (parts.size() == 3) {
            const int seconds = parts[0].toInt()*3600 + parts[1].toInt()*60 + parts[2].toInt();
            if (seconds > 0 && seconds <= 300 && notifiedShift_ != shift) {
                notifiedShift_ = shift; notify(QStringLiteral("准备下班"), QStringLiteral("距离下班不足 5 分钟，整理一下今天的成果吧。"));
            }
        }
    }
    if (reminders_ && !lastCheck_.isNull() && lastCheck_.secsTo(now) >= 0 && lastCheck_.secsTo(now) <= 5 &&
        lastShiftEnd_.isValid() && lastCheck_ < lastShiftEnd_ && lastShiftEnd_ <= now &&
        (lastStatus_ == "work" || lastStatus_ == "rest") && status != "work" && status != "rest")
        notify(QStringLiteral("下班啦"), QStringLiteral("今日班次结束，牛马可以休息了！"));
    if (status == "work" && !lastCheck_.isNull() && lastCheck_.secsTo(now) >= 0 && lastCheck_.secsTo(now) <= 5) ++restSeconds_;
    else restSeconds_ = 0;
    if (restReminders_ && !(focus_ && focus_->engaged()) && restSeconds_ >= 3600) { restSeconds_ = 0; notify(QStringLiteral("活动一下"), QStringLiteral("已经连续工作约一小时，起来喝水、活动一下吧。")); }
    lastStatus_ = status; lastCheck_ = now;
    lastShiftEnd_ = QDateTime::fromString(shift, Qt::ISODate);
}

// 光标进入控制按钮时取消穿透，展示区域继续穿透；逻辑坐标兼容缩放与高DPI。
void DesktopController::updateMiniInput() {
#ifdef Q_OS_WIN
    if (!mini_ || !mini_->isVisible()) return;
    const auto point = mini_->mapFromGlobal(QCursor::pos());
    const QPointF local(point.x()/miniScale_, point.y()/miniScale_);
    const bool controls = QRectF(205, 5, 75, 32).contains(local)
        || QRectF(10, 216, 310, 33).contains(local);
    const bool transparent = clickThrough_ && trayAvailable() && !controls;
    const auto handle = reinterpret_cast<HWND>(mini_->winId());
    const auto style = GetWindowLongPtrW(handle, GWL_EXSTYLE);
    const auto desired = transparent ? style | WS_EX_TRANSPARENT | WS_EX_LAYERED
                                     : style & ~LONG_PTR(WS_EX_TRANSPARENT);
    if (style != desired) SetWindowLongPtrW(handle, GWL_EXSTYLE, desired);
#endif
}
