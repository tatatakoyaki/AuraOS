#include "Services/audiorecorder.h"
#include <QAudioInput>
#include <QAudioFormat>
#include <QWebSocket>
#include <QDebug>

AudioRecorder::AudioRecorder(QObject *parent) : QObject{parent} {
    // 1. 配置底层音频采集格式
    // 注：48kHz 双声道为当前测试硬件（USB 麦克风）的物理支持格式。
    // 真实车载麦克风阵列通常配置为 16kHz、16bit、单声道 (云端 ASR 通用标准)。
    m_format.setSampleRate(48000);
    m_format.setChannelCount(2);
    m_format.setSampleSize(16);
    m_format.setCodec("audio/pcm");
    m_format.setByteOrder(QAudioFormat::LittleEndian);
    m_format.setSampleType(QAudioFormat::SignedInt);

    // 2. 初始化底层麦克风采集接口
    m_audioInput = new QAudioInput(m_format, this);

    // 3. 初始化 WebSocket 长连接，用于与 Python AI 微服务进行流式通信
    m_webSocket = new QWebSocket();
    connect(m_webSocket, &QWebSocket::connected, this, &AudioRecorder::onWebSocketConnected);
    connect(m_webSocket, &QWebSocket::textMessageReceived, this, &AudioRecorder::onWebSocketMessage);
    connect(m_webSocket, &QWebSocket::binaryMessageReceived, this, &AudioRecorder::onWebSocketBinaryMessage);
}

/**
 * @brief 开启麦克风录音并建立 WebSocket 连接
 */
void AudioRecorder::startRecording() {
    if (!m_webSocket->isValid()) {
        // 建立连接后，在 onWebSocketConnected 回调中自动启动录音
        m_webSocket->open(QUrl("ws://127.0.0.1:8002/ws_audio"));
        return;
    }

    // 开启底层麦克风数据流
    m_audioDevice = m_audioInput->start();
    if (m_audioDevice) {
        connect(m_audioDevice, &QIODevice::readyRead, this, &AudioRecorder::onAudioReadyRead);
        qDebug() << "[AudioRecorder] Streaming audio capture started.";
    } else {
        qWarning() << "[AudioRecorder] Failed to start microphone input!";
    }
}

/**
 * @brief 接收来自 Python 服务端的 TTS 二进制音频流
 */
void AudioRecorder::onWebSocketBinaryMessage(const QByteArray &data)
{
    qDebug() << "[AudioRecorder] TTS audio stream received. Size:" << data.size() << "bytes";
    // 音频播放不在此处处理，通过信号交由 SystemManager 统一进行音频焦点管理
    emit ttsAudioReady(data);
}

/**
 * @brief 实时读取麦克风数据，并以流式推送到 AI 服务端
 */
void AudioRecorder::onAudioReadyRead() {
    if (!m_audioDevice) return;

    // 读取当前可用的 PCM 音频数据块
    QByteArray data = m_audioDevice->readAll();

    if (!data.isEmpty() && m_webSocket->isValid()) {
        // 将 PCM 裸数据直接通过 WebSocket 发送
        m_webSocket->sendBinaryMessage(data);
    }
}

/**
 * @brief 停止录音，并发送控制指令通知 AI 服务端
 */
void AudioRecorder::stopRecording() {
    if (m_audioInput) {
        m_audioInput->stop();
    }

    if (m_webSocket && m_webSocket->isValid()) {
        // 发送文本指令，通知服务端音频流已结束
        m_webSocket->sendTextMessage("STOP");
        qDebug() << "[AudioRecorder] STOP signal sent to AI server.";
    }
}

/**
 * @brief WebSocket 连接成功回调
 */
void AudioRecorder::onWebSocketConnected() {
    qDebug() << "[AudioRecorder] WebSocket connected to AI ASR service. Starting capture...";
    startRecording();
}

/**
 * @brief 接收来自 Python 服务端的文本消息 (通常为 ASR 识别结果)
 */
void AudioRecorder::onWebSocketMessage(const QString &message) {
    qDebug() << "[AudioRecorder] ASR result received:" << message;
    emit asrResultReady(message);
}
