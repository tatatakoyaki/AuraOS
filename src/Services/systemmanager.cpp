#include "Services/systemmanager.h"
#include "Core/ServiceRegistry.h"

// 引入底层具体类，用于获取服务后进行类型转换 (qobject_cast)
#include "Services/hvachandler.h"
#include "Services/mediahandler.h"
#include "Services/appmanager.h"
#include "Services/widgetmanager.h"
#include "Services/audiorecorder.h"

#include <QDateTime>
#include <QDebug>

/**
 * @brief 辅助模板函数：从 ServiceRegistry 中获取指定名称的服务并向下转型。
 * @tparam T 目标业务类型
 * @param name 注册的服务名称
 * @return 转换成功返回对象指针，失败返回 nullptr
 */
template<typename T>
T* getService(const QString& name) {
    return qobject_cast<T*>(ServiceRegistry::instance().getService(name));
}

SystemManager::SystemManager(QObject *parent) : QObject{parent}
{
    // 初始化 TTS 语音播报播放器与内存缓冲区
    m_ttsPlayer = new QMediaPlayer(this);
    m_ttsBuffer = new QBuffer(this);

    // ==========================================
    // 核心机制：动态查表并建立跨模块的信号槽监听网络
    // 此机制替代了传统的硬编码依赖，实现了完全的解耦。
    // ==========================================
    if (auto hvac = getService<HvacHandler>("HvacService")) {
        connect(hvac, &HvacHandler::targetTempChanged, this, &SystemManager::currentTempChanged);
    }

    if (auto media = getService<MediaHandler>("MediaService")) {
        connect(media, &MediaHandler::currentSongChanged, this, &SystemManager::currentSongChanged);
        connect(media, &MediaHandler::isPlayingChanged, this, &SystemManager::isPlayingChanged);
        connect(media, &MediaHandler::volumeChanged, this, &SystemManager::volumeChanged);
        connect(media, &MediaHandler::coverUrlChanged, this, &SystemManager::coverUrlChanged);
        connect(media, &MediaHandler::lyricsChanged, this, &SystemManager::lyricsChanged);
        connect(media, &MediaHandler::currentLyricChanged, this, &SystemManager::currentLyricChanged);
    }

    if (auto widget = getService<WidgetManager>("WidgetService")) {
        connect(widget, &WidgetManager::widgetsChanged, this, &SystemManager::widgetsChanged);
    }

    if (auto app = getService<AppManager>("AppService")) {
        connect(app, &AppManager::appsChanged, this, &SystemManager::appsChanged);
    }

    if (auto audio = getService<AudioRecorder>("AudioService")) {
        connect(audio, &AudioRecorder::asrResultReady, this, &SystemManager::voiceResultReady);
        connect(audio, &AudioRecorder::ttsAudioReady, this, &SystemManager::handleTtsAudio);
    }
}

// ==========================================
// 一、数据读取接口 (Getters) - 动态查表法
// ==========================================

int SystemManager::currentTemp() const {
    if (auto h = getService<HvacHandler>("HvacService")) return h->targetTemp();
    return 24;
}

QString SystemManager::currentSong() const {
    if (auto m = getService<MediaHandler>("MediaService")) return m->currentSong();
    return "暂无播放";
}

bool SystemManager::isPlaying() const {
    if (auto m = getService<MediaHandler>("MediaService")) return m->isPlaying();
    return false;
}

int SystemManager::volume() const {
    if (auto m = getService<MediaHandler>("MediaService")) return m->volume();
    return 50;
}

QVariantList SystemManager::widgets() const {
    if (auto w = getService<WidgetManager>("WidgetService")) return w->widgets();
    return {};
}

QVariantList SystemManager::installedApps() const {
    if (auto a = getService<AppManager>("AppService")) return a->installedApps();
    return {};
}

QString SystemManager::coverUrl() const {
    if (auto m = getService<MediaHandler>("MediaService")) return m->coverUrl();
    return "";
}

QStringList SystemManager::lyricLines() const {
    if (auto m = getService<MediaHandler>("MediaService")) return m->lyricLines();
    return {};
}

int SystemManager::currentLyricIndex() const {
    if (auto m = getService<MediaHandler>("MediaService")) return m->currentLyricIndex();
    return -1;
}

// ==========================================
// 二、人类 UI 操作入口
// ==========================================

void SystemManager::requestHvacChange(int delta)
{
    m_lastUserActionTime = QDateTime::currentMSecsSinceEpoch();
    if (auto h = getService<HvacHandler>("HvacService")) h->changeTargetTemp(delta);
    qDebug() << "[SystemManager] User action: HVAC temp changed by" << delta;
}

void SystemManager::requestPlayMusic(const QString &song)
{
    m_lastUserActionTime = QDateTime::currentMSecsSinceEpoch();
    if (auto m = getService<MediaHandler>("MediaService")) m->searchAndPlay(song);
    qDebug() << "[SystemManager] User action: Play music request" << song;
}

void SystemManager::requestSwitchPage(const QString &pageName)
{
    m_lastUserActionTime = QDateTime::currentMSecsSinceEpoch();
    emit switchPageRequested(pageName);
}

void SystemManager::killApp(const QString &appName)
{
    qDebug() << "[SystemManager] User action: Kill app" << appName;
    if (auto a = getService<AppManager>("AppService")) a->killApp(appName);
}

void SystemManager::togglePlay() {
    if (auto m = getService<MediaHandler>("MediaService")) m->togglePlay();
}

void SystemManager::playNext() {
    qWarning() << "[SystemManager] playNext not implemented yet.";
}

void SystemManager::playPrevious() {
    qWarning() << "[SystemManager] playPrevious not implemented yet.";
}

void SystemManager::changeVolume(int delta) {
    if (auto m = getService<MediaHandler>("MediaService")) m->changeVolume(delta);
}

void SystemManager::saveWidgetLayout(const QVariantList &newWidgets) {
    if (auto w = getService<WidgetManager>("WidgetService")) w->saveLayout(newWidgets);
}

void SystemManager::syncAppLayout(const QStringList &appNames) {
    if (auto a = getService<AppManager>("AppService")) a->syncLayout(appNames);
}

void SystemManager::startVoiceRecord() {
    m_lastUserActionTime = QDateTime::currentMSecsSinceEpoch();
    qDebug() << "[SystemManager] User action: Start voice recording.";
    if (auto a = getService<AudioRecorder>("AudioService")) a->startRecording();
}

void SystemManager::stopVoiceRecord() {
    qDebug() << "[SystemManager] User action: Stop voice recording.";
    if (auto a = getService<AudioRecorder>("AudioService")) a->stopRecording();
}

/**
 * @brief 处理底层抛上来的 TTS 语音流，进行音频焦点管理并播放
 */
void SystemManager::handleTtsAudio(const QByteArray &audioData)
{
    qDebug() << "[SystemManager] TTS audio stream received. Preparing to play...";

    // 音频焦点管理：当需要播放 TTS 语音时，优先暂停正在播放的底层音乐
    if (auto m = getService<MediaHandler>("MediaService")) {
        if (m->isPlaying()) m->togglePlay();
    }

    m_ttsBuffer->close();
    m_ttsBuffer->setData(audioData);
    m_ttsBuffer->open(QIODevice::ReadOnly);

    m_ttsPlayer->setMedia(QMediaContent(), m_ttsBuffer);
    m_ttsPlayer->play();
}

// ==========================================
// 三、AI 操作入口
// ==========================================

/**
 * @brief 执行通过 AgentGuard 安全审查后的 AI 指令
 *
 * @param cmdId 指令唯一标识 (用于异步追踪)
 * @param agentId 发起请求的 Agent 标识
 * @param toolName 目标工具名
 * @param args 参数列表
 */
void SystemManager::requestAgentAction(const QString &cmdId, const QString &agentId, const QString &toolName, const QJsonObject &args)
{
    qDebug() << "[SystemManager] Agent action received from:" << agentId << " Tool:" << toolName;

    // 人机仲裁：若人类在 100ms 内进行过物理/UI 操作，则拒绝 AI 请求（人类优先级最高）
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    if (now - m_lastUserActionTime < 100) {
        qWarning() << "[SystemManager] Arbitration: AI request rejected. User is currently operating.";
        emit agentCommandCompleted(cmdId, false, "用户正在操作，请稍后再试");
        return;
    }

    // 根据工具名分发至对应服务
    if (toolName == "set_ac_temp") {
        if (auto h = getService<HvacHandler>("HvacService")) {
            h->setTargetTemp(args["temperature"].toInt());
            emit agentCommandCompleted(cmdId, true, "空调温度已调整");
        } else {
            emit agentCommandCompleted(cmdId, false, "当前车型未配备空调模块");
        }
    }
    else if (toolName == "play_music") {
        if (auto m = getService<MediaHandler>("MediaService")) {
            m->searchAndPlay(args["song_name"].toString());
            emit agentCommandCompleted(cmdId, true, "音乐已开始播放");
        } else {
            emit agentCommandCompleted(cmdId, false, "当前车型未配备音乐模块");
        }
    }
    else if (toolName == "open_page") {
        emit switchPageRequested(args["page_name"].toString());
        emit agentCommandCompleted(cmdId, true, "页面已切换");
    }
    else {
        emit agentCommandCompleted(cmdId, false, "未知指令");
    }
}

/**
 * @brief AI 指令的物理边界兜底校验
 * (注：此处主要供 AgentGuard 调用，SystemManager 内部做最终防线)
 */
bool SystemManager::validateAgentRequest(const QString &toolName, const QJsonObject &args)
{
    if (toolName == "set_ac_temp") {
        int temp = args["temperature"].toInt();
        if (temp < 16 || temp > 30) {
            qWarning() << "[SystemManager] Safety interception: Temp out of bounds" << temp;
            return false;
        }
    }
    return true;
}
