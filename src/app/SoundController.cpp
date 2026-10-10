#include "SoundController.h"
#include <QRandomGenerator>
#include <QtMath>
#include <array>
#include <atomic>
#include <thread>
#include <chrono>
#ifdef Q_OS_WIN
#include <qt_windows.h>
#include <mmsystem.h>
#endif
struct SoundController::Backend {
    std::atomic_bool run{true}, requested{false};
    std::atomic_int volume{25}, scene{0}, failure{0};
    std::thread worker;
#ifdef Q_OS_WIN
    HWAVEOUT device=nullptr;
    std::array<WAVEHDR,6> headers{};
    std::array<std::array<qint16,2205>,6> buffers{};
    int nextBuffer=0;
#endif
    double amplitude=0, low=0;
    quint64 samples=0;
    int activeScene=0;
    QRandomGenerator random{0x4e49554d};

    // 所有设备操作和 PCM 合成都在单一音频线程，界面保存不阻塞补缓冲。
    void render();
    void close();
};
void SoundController::Backend::close() {
#ifdef Q_OS_WIN
    if(!device)return;
    waveOutReset(device);
    for(auto &header:headers) {
        if(header.dwFlags & WHDR_PREPARED)waveOutUnprepareHeader(device,&header,sizeof(header));
        header={};
    }
    waveOutClose(device);device=nullptr;nextBuffer=0;amplitude=0;
#endif
}
void SoundController::Backend::render() {
#ifdef Q_OS_WIN
    if(!device && requested.load()) {
        WAVEFORMATEX format{};format.wFormatTag=WAVE_FORMAT_PCM;format.nChannels=1;
        format.nSamplesPerSec=44100;format.wBitsPerSample=16;format.nBlockAlign=2;format.nAvgBytesPerSec=88200;
        auto result=waveOutOpen(&device,WAVE_MAPPER,&format,0,0,CALLBACK_NULL);
        if(result!=MMSYSERR_NOERROR) { device=nullptr;requested=false;failure=int(result);return; }
        activeScene=scene.load();
        // 首次预填缓冲期间暂停设备，防止只送出第一块就开始播放。
        waveOutPause(device);
        for(int i=0;i<int(headers.size());++i) {
            auto &header=headers[i];header={};
            header.lpData=reinterpret_cast<LPSTR>(buffers[i].data());header.dwBufferLength=DWORD(sizeof(buffers[i]));
            result=waveOutPrepareHeader(device,&header,sizeof(header));
            if(result!=MMSYSERR_NOERROR) {close();requested=false;failure=int(result);return;}
        }
    }
    if(!device)return;
    // 始终按提交顺序补充下一块，不因多个缓冲同时完成而打乱采样顺序。
    for(int count=0;count<int(headers.size());++count) {
        auto &header=headers[nextBuffer];
        if(header.dwFlags & WHDR_INQUEUE)break;
        if(!requested.load() && amplitude<.0001)break;
        for(auto &sample:buffers[nextBuffer]) {
            const int desiredScene=scene.load(std::memory_order_relaxed);
            if(activeScene!=desiredScene && amplitude<.0001)activeScene=desiredScene;
            const double target=requested.load(std::memory_order_relaxed) && activeScene==desiredScene
                ? volume.load(std::memory_order_relaxed)/100.0 : 0.;
            amplitude+=(target-amplitude)*.0003;
            const double noise=random.generateDouble()*2-1;low=low*.985+noise*.015;
            const double t=double(samples++)/44100.;double value=noise*.22;
            if(activeScene==1)value=(noise*.10+low*1.9)*(.7+.3*qSin(t*.8));
            if(activeScene==2) {const double chirp=qPow(qMax(0.,qSin(t*1.7)),18);value=low*1.5+chirp*qSin(t*11000+12*qSin(t*25))*.09;}
            if(activeScene==3)value=low*2.0+.035*qSin(t*720+4*qSin(t*3))+.02*qSin(t*1030);
            sample=qint16(qBound(-1.,value*amplitude,1.)*32767);
        }
        const auto result=waveOutWrite(device,&header,sizeof(header));
        if(result!=MMSYSERR_NOERROR) {close();requested=false;failure=int(result);return;}
        nextBuffer=(nextBuffer+1)%int(headers.size());
    }
    waveOutRestart(device);
    // 停止后让已排队的淡出音频自然播放完，避免 reset 截断声音。
    if(!requested.load() && amplitude<.0001) {
        bool queued=false;for(const auto &header:headers)queued|=(header.dwFlags&WHDR_INQUEUE)!=0;
        if(!queued)close();
    }
#endif
}
SoundController::SoundController(QObject *parent):QObject(parent),backend_(std::make_unique<Backend>()) {
    volume_=qBound(0,settings_.value("volume",25).toInt(),100);
    scene_=qBound(0,settings_.value("scene",0).toInt(),3);
    autoFocus_=settings_.value("autoFocus",false).toBool();pauseOnRest_=settings_.value("pauseOnRest",true).toBool();
    backend_->volume=volume_;backend_->scene=scene_;
    backend_->worker=std::thread([b=backend_.get()] {
        while(b->run.load()) {b->render();std::this_thread::sleep_for(std::chrono::milliseconds(5));}
        b->close();
    });
    // 偏好采用短延迟合并写盘，拖动音量时不逐次阻塞界面。
    preferenceTimer_.setSingleShot(true);preferenceTimer_.setInterval(300);
    connect(&preferenceTimer_,&QTimer::timeout,this,&SoundController::save);
    clock_.start();connect(&pulse_,&QTimer::timeout,this,&SoundController::pump);pulse_.start(30);
}
SoundController::~SoundController() {
    backend_->run=false;
    if(backend_->worker.joinable())backend_->worker.join();
    if(preferenceTimer_.isActive())save();
}
bool SoundController::available() const {
#ifdef Q_OS_WIN
    return true;
#else
    return false;
#endif
}
// 保存声音偏好，播放状态不持久化，重启始终静音。
void SoundController::save() {
    settings_.setValue("volume",volume_);settings_.setValue("scene",scene_);settings_.setValue("autoFocus",autoFocus_);settings_.setValue("pauseOnRest",pauseOnRest_);settings_.sync();
    if(settings_.status()!=QSettings::NoError)error_=QStringLiteral("声音偏好保存失败"); emit changed();
}
// 参数即时送入音频线程，持久化合并处理。
void SoundController::setVolume(int value) {
    volume_=qBound(0,value,100);backend_->volume=volume_;preferenceTimer_.start();emit changed();
}
void SoundController::setScene(int value) {
    scene_=qBound(0,value,3);backend_->scene=scene_;preferenceTimer_.start();emit changed();
}
void SoundController::setAutoFocus(bool value) {autoFocus_=value;save();}
void SoundController::setPauseOnRest(bool value) {
    pauseOnRest_=value;
    if(linked_ && pauseOnRest_ && lastFocus_!="focus") {requested_=false;backend_->requested=false;}
    save();
}
void SoundController::toggle() {
    if(!available()) {error_=QStringLiteral("当前平台音频后端尚未接入");emit changed();return;}
    requested_=!requested_;backend_->requested=requested_;linked_=false;error_.clear();emit changed();
}
void SoundController::stopAfter(int minutes) {
    stopAt_=minutes>0?clock_.elapsed()+qBound(1,minutes,480)*60000LL:0;emit changed();
}
// 声音联动只响应专注阶段转换；手动播放仍可独立于番茄钟使用。
void SoundController::followFocus(const QString &state) {
    if(lastFocus_==state)return;lastFocus_=state;
    if(autoFocus_ && state=="focus" && available()) {requested_=true;linked_=true;}
    else if(linked_ && pauseOnRest_ && state!="focus")requested_=false;
    backend_->requested=requested_;emit changed();
}
// 界面线程仅处理定时和错误通知，不生成音频。
void SoundController::pump() {
    if(stopAt_ && clock_.elapsed()>=stopAt_) {
        stopAt_=0;requested_=false;linked_=false;backend_->requested=false;emit changed();
    }
    const int failure=backend_->failure.exchange(0);
    if(failure) {
        requested_=false;linked_=false;
#ifdef Q_OS_WIN
        wchar_t description[MAXERRORLENGTH]{};
        waveOutGetErrorTextW(MMRESULT(failure),description,MAXERRORLENGTH);
        error_=QStringLiteral("声音播放失败（%1）：%2。检查输出设备后可重新播放。").arg(failure).arg(QString::fromWCharArray(description));
#else
        error_=QStringLiteral("声音播放失败");
#endif
        emit changed();
    }
}
