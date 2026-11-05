#ifndef AUDIORECORDER_H
#define AUDIORECORDER_H

#include <QObject>
#include <QAudioInput>
#include <QAudioFormat>
#include <QWebSocket>
#include <QUrl>

/**
 * @brief AudioRecorder (音频采集与流式通信网关)
 *
 * 架构设计：
 * 负责端侧音频的底层采集，并利用 WebSocket 将 PCM 裸数据实时流式推送到 Python 云端/本地微服务。
 * 同时，接收来自 AI 服务端返回的 TTS 语音流，通过信号抛出，交由上层（大总管）进行音频焦点管理与播放。
 * 彻底剥离了音频硬件操作与 UI 逻辑的耦合。
 */
class AudioRecorder : public QObject
{
    Q_OBJECT
public:
    explicit AudioRecorder(QObject *parent = nullptr);

    /**
     * @brief 开启麦克风录音，并建立 WebSocket 连接
     */
    Q_INVOKABLE void startRecording();

    /**
     * @brief 停止录音，并发送结束指令
     */
    Q_INVOKABLE void stopRecording();

signals:
    // 实时输出的音频块（PCM 二进制流），可用于本地音频可视化分析
    void audioChunkReady(const QByteArray &chunk);

    // 收到云端 ASR（语音识别）返回的文本结果，通知上层业务
    void asrResultReady(const QString &text);

    // 收到云端 TTS（语音合成）返回的音频数据流，抛给大总管进行硬件播放
    void ttsAudioReady(const QByteArray &audioData);

private slots:
    void onAudioReadyRead();
    void onWebSocketConnected();
    void onWebSocketMessage(const QString &message);
    void onWebSocketBinaryMessage(const QByteArray &data);

private:
    QAudioInput *m_audioInput = nullptr; // 麦克风硬件采集接口
    QIODevice *m_audioDevice = nullptr; // 麦克风数据流读取接口
    QWebSocket *m_webSocket = nullptr;  // 用于与 AI 服务端流式通信的长连接

    QAudioFormat m_format;              // 音频采集格式参数（如采样率、声道数）
};

#endif // AUDIORECORDER_H
