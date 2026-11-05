#ifndef MEDIAHANDLER_H
#define MEDIAHANDLER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QStringList>

class IMediaHardware;

/**
 * @brief MediaHandler (多媒体业务管家)
 *
 * 架构设计：
 * 1. 依赖注入：通过构造函数接收 IMediaHardware 抽象接口，与底层音频解码硬件彻底解耦。
 * 2. 网络交互：内部使用 QNetworkAccessManager 与远端流媒体服务 (网易云网关) 通信，获取歌曲、封面及歌词。
 * 3. 业务处理：负责解析 LRC 歌词时间戳，监听底层硬件播放进度，实时计算并更新当前歌词索引。
 * 4. 状态缓存：作为 UI 的数据源，缓存播放状态、音量等数据，供 QML 引擎直接绑定。
 */
class MediaHandler : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString currentSong READ currentSong NOTIFY currentSongChanged)
    Q_PROPERTY(bool isPlaying READ isPlaying NOTIFY isPlayingChanged)
    Q_PROPERTY(int volume READ volume NOTIFY volumeChanged)
    Q_PROPERTY(QString coverUrl READ coverUrl NOTIFY coverUrlChanged)
    Q_PROPERTY(QString lyrics READ lyrics NOTIFY lyricsChanged)
    Q_PROPERTY(QString currentLyric READ currentLyric NOTIFY currentLyricChanged)
    Q_PROPERTY(int currentLyricIndex READ currentLyricIndex NOTIFY currentLyricChanged)
    Q_PROPERTY(QStringList lyricLines READ lyricLines NOTIFY lyricsChanged)

public:
    /**
     * @brief 构造函数，注入底层音频硬件接口
     * @param hardware 实现了 IMediaHardware 的硬件实例指针
     */
    explicit MediaHandler(IMediaHardware *hardware, QObject *parent = nullptr);

    // 数据获取接口 (Getters)
    QString currentSong() const;
    bool isPlaying() const;
    int volume() const;
    QString coverUrl() const;
    QString lyrics() const;
    QString currentLyric() const;
    int currentLyricIndex() const;
    QStringList lyricLines() const;

    /**
     * @brief 搜索并播放指定歌曲（供 QML UI 或 Agent 调用）
     * @param keyword 搜索关键词（歌名/歌手）
     */
    Q_INVOKABLE void searchAndPlay(const QString &keyword);

    /**
     * @brief 切换播放/暂停状态
     */
    Q_INVOKABLE void togglePlay();

    /**
     * @brief 调节音量
     * @param delta 音量增量（如 -10 或 +10）
     */
    Q_INVOKABLE void changeVolume(int delta);

signals:
    // 状态变更广播，供 UI 自动刷新
    void currentSongChanged();
    void isPlayingChanged();
    void volumeChanged();
    void coverUrlChanged();
    void lyricsChanged();
    void currentLyricChanged();

private:
    IMediaHardware *m_hardware;             // 底层硬件抽象层接口指针
    QNetworkAccessManager *m_networkManager; // 用于向流媒体网关发起 HTTP 请求

    // 业务数据缓存
    QString m_currentSongName;
    QString m_coverUrl;
    QString m_lyrics;
    QStringList m_lyricLines;
    QList<qint64> m_lyricTimes;
    QString m_currentLyric;
    int m_currentLyricIndex = -1;

    // UI 状态缓存
    bool m_isPlayingCache = false;
    int m_volumeCache = 50;

    /**
     * @brief 解析 LRC 格式歌词，提取时间戳与文本，存入缓存列表
     * @param lrcText 原始 LRC 歌词文本
     */
    void parseLyrics(const QString &lrcText);
};

#endif // MEDIAHANDLER_H
