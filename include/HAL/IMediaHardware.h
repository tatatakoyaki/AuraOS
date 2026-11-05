#ifndef IMEDIAHARDWARE_H
#define IMEDIAHARDWARE_H

#include <QString>
#include <functional>

/**
 * @brief IMediaHardware (多媒体硬件抽象层接口)
 *
 * 架构设计：
 * 定义音频硬件（如高通 DSP、Android Audio、本地 QMediaPlayer 等）的标准控制接口。
 * 上层业务逻辑 (MediaHandler) 仅依赖此接口，从而屏蔽底层硬件差异。
 *
 * 采用纯虚类与 std::function 回调机制，避免了对 Qt 信号槽的强依赖，
 * 确保了接口的跨平台纯净度，方便未来移植到异构芯片或跨进程通信。
 */
class IMediaHardware {
public:
    virtual ~IMediaHardware() = default;

    /**
     * @brief 硬件解码并播放指定的音频流/本地路径
     * @param url 音频流地址或本地绝对路径
     */
    virtual void playUrl(const QString& url) = 0;

    /**
     * @brief 暂停播放
     */
    virtual void pause() = 0;

    /**
     * @brief 恢复播放
     */
    virtual void resume() = 0;

    /**
     * @brief 调节硬件物理音量
     * @param volume 音量值 (0-100)
     */
    virtual void setVolume(int volume) = 0;

    /**
     * @brief 注册播放进度回调（用于上层业务同步 UI，如滚动歌词）
     * @param callback 回调函数，参数为当前播放进度（毫秒）
     */
    virtual void setPositionChangedCallback(std::function<void(qint64)> callback) = 0;

    /**
     * @brief 注册播放状态回调（用于上层业务同步 UI，如按钮状态切换）
     * @param callback 回调函数，参数为当前是否正在播放 (true为播放，false为暂停)
     */
    virtual void setStateChangedCallback(std::function<void(bool isPlaying)> callback) = 0;
};

#endif // IMEDIAHARDWARE_H
