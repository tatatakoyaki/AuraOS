#include "Services/mediahandler.h"
#include "HAL/IMediaHardware.h"
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <QRegularExpression>

MediaHandler::MediaHandler(IMediaHardware *hardware, QObject *parent)
    : QObject{parent}, m_hardware(hardware), m_networkManager(new QNetworkAccessManager(this))
{
    m_currentSongName = "等待播放...";

    if (m_hardware) {
        // 初始化底层硬件物理音量
        m_hardware->setVolume(m_volumeCache);

        // 注册底层硬件播放进度回调。
        // 当硬件进度更新时，根据时间戳计算当前所处的歌词行索引，并通知 UI 刷新。
        m_hardware->setPositionChangedCallback([this](qint64 position) {
            int newIndex = -1;
            for (int i = 0; i < m_lyricTimes.size(); ++i) {
                if (position >= m_lyricTimes[i]) newIndex = i;
                else break;
            }
            if (newIndex != m_currentLyricIndex) {
                m_currentLyricIndex = newIndex;
                if (newIndex >= 0 && newIndex < m_lyricLines.size()) {
                    m_currentLyric = m_lyricLines[newIndex];
                } else {
                    m_currentLyric = "";
                }
                emit currentLyricChanged();
            }
        });

        // 注册底层硬件播放状态变更回调。
        // 更新业务层缓存，并通知 UI 更新播放/暂停按钮状态。
        m_hardware->setStateChangedCallback([this](bool isPlaying) {
            m_isPlayingCache = isPlaying;
            emit isPlayingChanged();
        });
    }
}

QString MediaHandler::currentSong() const { return m_currentSongName; }
bool MediaHandler::isPlaying() const { return m_isPlayingCache; }
int MediaHandler::volume() const { return m_volumeCache; }
QString MediaHandler::coverUrl() const { return m_coverUrl; }
QString MediaHandler::lyrics() const { return m_lyrics; }
QString MediaHandler::currentLyric() const { return m_currentLyric; }
int MediaHandler::currentLyricIndex() const { return m_currentLyricIndex; }
QStringList MediaHandler::lyricLines() const { return m_lyricLines; }

/**
 * @brief 解析 LRC 格式歌词文本，提取时间戳与文本内容
 * @param lrcText 原始 LRC 歌词文本
 */
void MediaHandler::parseLyrics(const QString &lrcText)
{
    m_lyricLines.clear();
    m_lyricTimes.clear();
    m_currentLyricIndex = -1;
    m_currentLyric = "";

    QStringList lines = lrcText.split('\n');
    // 正则表达式匹配 [00:12.34] 或 [00:12.345] 格式
    QRegularExpression re("\\[(\\d{2}):(\\d{2})\\.(\\d{2,3})\\](.*)");

    for (const QString &line : lines) {
        QRegularExpressionMatch match = re.match(line);
        if (match.hasMatch()) {
            int min = match.captured(1).toInt();
            int sec = match.captured(2).toInt();
            int ms = match.captured(3).toInt();
            // 若毫秒部分只有两位，需乘以 10 转换为标准毫秒
            if (match.captured(3).length() == 2) ms *= 10;

            qint64 timeMs = (min * 60 + sec) * 1000 + ms;
            QString text = match.captured(4).trimmed();

            m_lyricTimes.append(timeMs);
            m_lyricLines.append(text);
        }
    }
}

/**
 * @brief 向流媒体服务网关请求歌曲信息，并触发底层硬件播放
 * @param keyword 搜索关键词
 */
void MediaHandler::searchAndPlay(const QString &keyword)
{
    m_currentSongName = "搜索中...";
    emit currentSongChanged();

    // 请求 Python 音乐网关
    QUrl url("http://127.0.0.1:8001/search_song");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject json;
    json["keyword"] = keyword;
    QByteArray data = QJsonDocument(json).toJson();

    QNetworkReply *reply = m_networkManager->post(request, data);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            QJsonObject obj = doc.object();

            if (obj.contains("play_url")) {
                QString playUrl = obj["play_url"].toString();
                m_currentSongName = obj["song_name"].toString() + " - " + obj["artist"].toString();
                m_coverUrl = obj["cover_url"].toString();
                m_lyrics = obj["lyrics"].toString();

                parseLyrics(m_lyrics);

                emit currentSongChanged();
                emit coverUrlChanged();
                emit lyricsChanged();

                qDebug() << "[MediaHandler] Play URL acquired. Requesting hardware playback:" << playUrl;

                // 核心架构约束：业务层不直接解码播放，而是将音频流地址下发给底层 HAL 硬件处理
                if (m_hardware) {
                    m_hardware->playUrl(playUrl);
                }
            } else {
                m_currentSongName = "错误: " + obj["error"].toString();
                qWarning() << "[MediaHandler] Search failed:" << m_currentSongName;
                emit currentSongChanged();
            }
        } else {
            m_currentSongName = "连接网关失败";
            qWarning() << "[MediaHandler] Network error:" << reply->errorString();
            emit currentSongChanged();
        }
        reply->deleteLater();
    });
}

/**
 * @brief 切换播放/暂停状态
 */
void MediaHandler::togglePlay()
{
    // 业务层仅发送控制指令，实际状态变更依赖底层硬件回调更新
    if (m_isPlayingCache) {
        if (m_hardware) m_hardware->pause();
    } else {
        if (m_hardware) m_hardware->resume();
    }
}

/**
 * @brief 调节音量
 * @param delta 音量增量
 */
void MediaHandler::changeVolume(int delta)
{
    int newVol = m_volumeCache + delta;
    if (newVol < 0) newVol = 0;
    if (newVol > 100) newVol = 100;

    if (m_volumeCache == newVol) return;

    m_volumeCache = newVol;

    // 更新底层硬件物理音量
    if (m_hardware) {
        m_hardware->setVolume(m_volumeCache);
    }
    emit volumeChanged();
}
