#include "Platform/Mock/MockMediaHardware.h"
#include <QDebug>
#include <QUrl>

MockMediaHardware::MockMediaHardware()
{
    m_player = new QMediaPlayer();

    // 监听底层播放器进度，触发业务层注册的进度回调
    QObject::connect(m_player, &QMediaPlayer::positionChanged, [this](qint64 position) {
        if (m_positionCallback) {
            m_positionCallback(position);
        }
    });

    // 监听底层播放器状态，触发业务层注册的状态回调
    QObject::connect(m_player, &QMediaPlayer::stateChanged, [this](QMediaPlayer::State state) {
        bool isPlaying = (state == QMediaPlayer::PlayingState);
        if (m_stateCallback) {
            m_stateCallback(isPlaying);
        }
    });

    qDebug() << "[MockMediaHardware] Audio hardware simulation initialized.";
}

MockMediaHardware::~MockMediaHardware()
{
    // 释放底层模拟播放器资源
    delete m_player;
}

/**
 * @brief 模拟底层硬件解码并播放指定的音频流
 * @param url 音频流地址
 */
void MockMediaHardware::playUrl(const QString& url)
{
    qDebug() << "[MockMediaHardware] Hardware decoding request received. URL:" << url;
    m_player->setMedia(QUrl(url));
    m_player->play();
}

/**
 * @brief 模拟底层硬件暂停播放
 */
void MockMediaHardware::pause()
{
    qDebug() << "[MockMediaHardware] Hardware pause executed.";
    m_player->pause();
}

/**
 * @brief 模拟底层硬件恢复播放
 */
void MockMediaHardware::resume()
{
    qDebug() << "[MockMediaHardware] Hardware resume executed.";
    m_player->play();
}

/**
 * @brief 模拟底层硬件调节物理音量
 * @param volume 音量值 (0-100)
 */
void MockMediaHardware::setVolume(int volume)
{
    qDebug() << "[MockMediaHardware] Setting hardware volume:" << volume;
    m_player->setVolume(volume);
}

/**
 * @brief 注册播放进度变更回调
 */
void MockMediaHardware::setPositionChangedCallback(std::function<void(qint64)> callback)
{
    m_positionCallback = callback;
}

/**
 * @brief 注册播放状态变更回调
 */
void MockMediaHardware::setStateChangedCallback(std::function<void(bool)> callback)
{
    m_stateCallback = callback;
}
