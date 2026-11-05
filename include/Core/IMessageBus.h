#ifndef IMESSAGEBUS_H
#define IMESSAGEBUS_H

#include <QByteArray>
#include <QString>
#include <functional>

/**
 * @brief IMessageBus (系统消息总线抽象接口)
 *
 * 架构设计：
 * 采用发布-订阅 (Pub/Sub) 模式，作为整个微内核 OS 内部各服务、Agent 和模块之间的通信总线。
 * 通过此接口，彻底解耦了通信的发送方与接收方，屏蔽了底层通信实现的差异（如：本地信号槽、TCP/UDP、共享内存、DDS 等）。
 *
 * 注意：为保证跨进程/跨设备通信的兼容性，Payload 统一采用 QByteArray 序列化数据（如 JSON、Protobuf）。
 */
class IMessageBus {
public:
    virtual ~IMessageBus() = default;

    /**
     * @brief 发布消息到指定的主题 (Topic)
     * @param topic 消息主题（如 "vehicle/speed", "system/notification"）
     * @param payload 序列化后的消息负载（如 JSON 字符串转成的字节流）
     */
    virtual void publish(const QString& topic, const QByteArray& payload) = 0;

    /**
     * @brief 订阅指定主题的消息
     * @param topic 消息主题
     * @param callback 收到消息时的回调函数（传入接收到的 Payload）
     */
    virtual void subscribe(const QString& topic, std::function<void(const QByteArray&)> callback) = 0;
};

#endif // IMESSAGEBUS_H
