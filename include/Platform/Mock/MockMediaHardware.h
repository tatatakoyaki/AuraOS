#ifndef MOCKMEDIAHARDWARE_H
#define MOCKMEDIAHARDWARE_H

#include "HAL/IMediaHardware.h"
#include <QMediaPlayer>

/**
 * @brief MockMediaHardware (多媒体硬件模拟实现)
 *
 * 架构设计：
 * 这是 HAL 层 IMediaHardware 接口的 Mock 实现类。
 * 在开发机上，利用 Qt 的 QMediaPlayer 模拟真实的音频 DSP 或底层硬件解码器。
 * 同时负责将底层播放器的异步事件（进度、状态）通过 std::function 回调反馈给上层业务。
 */
class MockMediaHardware : public IMediaHardware
{
public:
    MockMediaHardware();
    virtual ~MockMediaHardware() override;

    // 控制接口实现
    void playUrl(const QString& url) override;
    void pause() override;
    void resume() override;
    void setVolume(int volume) override;

    // 回调注册实现
    void setPositionChangedCallback(std::function<void(qint64)> callback) override;
    void setStateChangedCallback(std::function<void(bool isPlaying)> callback) override;

private:
    QMediaPlayer *m_player;                             // Qt 底层播放器实例（模拟硬件解码）
    std::function<void(qint64)> m_positionCallback;     // 向上层反馈播放进度的回调
    std::function<void(bool)> m_stateCallback;          // 向上层反馈播放状态的回调
};

#endif // MOCKMEDIAHARDWARE_H
