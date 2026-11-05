#ifndef SYSTEMMANAGER_H
#define SYSTEMMANAGER_H

#include <QObject>
#include <QJsonObject>
#include <QDateTime>
#include <QVariantList>
#include <QMediaPlayer>
#include <QBuffer>
#include <QByteArray>
#include <QStringList>

/**
 * @brief SystemManager (中央调度器 / 大总管)
 *
 * 架构设计：
 * 作为整个座舱 OS 的核心调度枢纽，对上层 (UI/Agent) 屏蔽底层具体业务实现。
 * 1. 单一事实来源：对外暴露 Q_PROPERTY 数据接口，供 QML 唯一绑定。
 * 2. 人机仲裁：处理人类 UI 操作与 AI Agent 指令的优先级冲突。
 * 3. 动态查表：内部通过 ServiceRegistry 动态拉取底层服务指针，拒绝硬编码依赖。
 * 4. 音频焦点管理：统一管理 TTS 语音播报与底层音乐播放器的音频冲突。
 */
class SystemManager : public QObject
{
    Q_OBJECT

    // ==========================================
    // 暴露给前端 QML 的数据接口
    // ==========================================
    Q_PROPERTY(int currentTemp READ currentTemp NOTIFY currentTempChanged)
    Q_PROPERTY(QString currentSong READ currentSong NOTIFY currentSongChanged)
    Q_PROPERTY(bool isPlaying READ isPlaying NOTIFY isPlayingChanged)
    Q_PROPERTY(int volume READ volume NOTIFY volumeChanged)
    Q_PROPERTY(QVariantList widgets READ widgets NOTIFY widgetsChanged)
    Q_PROPERTY(QVariantList installedApps READ installedApps NOTIFY appsChanged)
    Q_PROPERTY(QString coverUrl READ coverUrl NOTIFY coverUrlChanged)
    Q_PROPERTY(QStringList lyricLines READ lyricLines NOTIFY lyricsChanged)
    Q_PROPERTY(int currentLyricIndex READ currentLyricIndex NOTIFY currentLyricChanged)

public:
    explicit SystemManager(QObject *parent = nullptr);

    // --- 数据读取接口 (供 QML 调用) ---
    int currentTemp() const;
    QString currentSong() const;
    bool isPlaying() const;
    int volume() const;
    QVariantList widgets() const;
    QVariantList installedApps() const;
    QString coverUrl() const;
    QStringList lyricLines() const;
    int currentLyricIndex() const;

    // ==========================================
    // 人类 UI 操作入口 (统一由大总管代理分发)
    // ==========================================
    Q_INVOKABLE void requestHvacChange(int delta);
    Q_INVOKABLE void requestPlayMusic(const QString &song);
    Q_INVOKABLE void requestSwitchPage(const QString &pageName);
    Q_INVOKABLE void killApp(const QString &appName);
    Q_INVOKABLE void togglePlay();
    Q_INVOKABLE void playNext();
    Q_INVOKABLE void playPrevious();
    Q_INVOKABLE void changeVolume(int delta);
    Q_INVOKABLE void saveWidgetLayout(const QVariantList &newWidgets);
    Q_INVOKABLE void syncAppLayout(const QStringList &appNames);
    Q_INVOKABLE void startVoiceRecord();
    Q_INVOKABLE void stopVoiceRecord();

    // ==========================================
    // AI 操作入口 (仅限通过 AgentGuard 审核后调用)
    // ==========================================
    /**
     * @brief 执行通过安全审查的 AI 指令
     */
    void requestAgentAction(const QString &cmdId, const QString &agentId, const QString &toolName, const QJsonObject &args);

    /**
     * @brief 供 AgentGuard 调用的安全拒单接口
     */
    void rejectAgentCommand(const QString &cmdId, const QString &reason) {
        emit agentCommandCompleted(cmdId, false, reason);
    }

signals:
    // 数据状态变更信号（供 UI 绑定刷新）
    void currentTempChanged();
    void currentSongChanged();
    void isPlayingChanged();
    void volumeChanged();
    void widgetsChanged();
    void appsChanged();
    void coverUrlChanged();
    void lyricsChanged();
    void currentLyricChanged();

    // 跨模块指令信号
    void switchPageRequested(const QString &pageName);

    // 异步执行结果回调 (通知 AgentBridge)
    void agentCommandCompleted(const QString &cmdId, bool success, const QString &msg);

    // 语音识别结果广播 (通知悬浮窗 UI)
    void voiceResultReady(const QString &text);

private:
    // TTS 语音播报播放器（负责抢占音频焦点）
    QMediaPlayer *m_ttsPlayer;
    QBuffer *m_ttsBuffer;

    qint64 m_lastUserActionTime = 0;

    /**
     * @brief 基础物理边界校验（主要由 AgentGuard 调用，大总管做最后兜底）
     */
    bool validateAgentRequest(const QString &toolName, const QJsonObject &args);

private slots:
    /**
     * @brief 播放底层抛上来的 TTS 语音流
     */
    void handleTtsAudio(const QByteArray &audioData);
};

#endif // SYSTEMMANAGER_H
