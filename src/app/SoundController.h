// 原创程序合成声音，离线播放；不包含第三方录音素材。
#pragma once
#include <QObject>
#include <QTimer>
#include <QElapsedTimer>
#include <QSettings>
#include <memory>
class SoundController final : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool available READ available CONSTANT)
    Q_PROPERTY(bool playing READ playing NOTIFY changed)
    Q_PROPERTY(int volume READ volume WRITE setVolume NOTIFY changed)
    Q_PROPERTY(int scene READ scene WRITE setScene NOTIFY changed)
    Q_PROPERTY(bool autoFocus READ autoFocus WRITE setAutoFocus NOTIFY changed)
    Q_PROPERTY(bool pauseOnRest READ pauseOnRest WRITE setPauseOnRest NOTIFY changed)
    Q_PROPERTY(QString error READ error NOTIFY changed)
public:
    explicit SoundController(QObject *parent=nullptr);
    ~SoundController() override;
    bool available() const;
    bool playing() const { return requested_; }
    int volume() const { return volume_; }
    int scene() const { return scene_; }
    bool autoFocus() const { return autoFocus_; }
    bool pauseOnRest() const { return pauseOnRest_; }
    QString error() const { return error_; }
    void setVolume(int value);
    void setScene(int value);
    void setAutoFocus(bool value);
    void setPauseOnRest(bool value);
    Q_INVOKABLE void toggle();
    Q_INVOKABLE void stopAfter(int minutes);
    void followFocus(const QString &state);
signals:
    void changed();
private:
    // 独立音频线程按顺序补缓冲，界面仅同步定时与错误状态。
    void pump();
    void save();
    struct Backend;
    std::unique_ptr<Backend> backend_;
    QSettings settings_{"NiumaTimer","Sound"};
    QTimer pulse_, preferenceTimer_;
    QElapsedTimer clock_;
    qint64 stopAt_=0;
    bool requested_=false, autoFocus_=false, pauseOnRest_=true, linked_=false;
    int volume_=25, scene_=0;
    QString error_, lastFocus_;
};
