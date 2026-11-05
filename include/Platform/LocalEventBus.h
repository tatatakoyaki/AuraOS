#ifndef LOCALEVENTBUS_H
#define LOCALEVENTBUS_H

#include "Core/IMessageBus.h"
#include <QHash>
#include <QList>
#include <QMutex>
#include <QMutexLocker>
#include <QDebug>

/**
 * @brief LocalEventBus (本地进程内事件总线实现)
 *
 * 架构设计：
 * 基于 IMessageBus 接口的本地实现，利用 Qt 容器与 std::function 实现进程内的高效发布-订阅。
 * 适用于单一进程内的模块解耦。
 *
 * 注意：所有操作均通过 QMutex 保护，且在执行回调前释放锁，防止死锁。
 */
class LocalEventBus : public IMessageBus
{
public:
    LocalEventBus() = default;
    virtual ~LocalEventBus() = default;

    /**
     * @brief 发布消息到指定主题
     * @param topic 消息主题
     * @param payload 序列化后的消息负载
     */
    void publish(const QString& topic, const QByteArray& payload) override {
        QList<std::function<void(const QByteArray&)>> callbacksToExecute;

        {
            // 在作用域内加锁，将订阅者列表拷贝出来
            QMutexLocker locker(&m_mutex);
            if (!m_subscribers.contains(topic)) {
                return;
            }
            callbacksToExecute = m_subscribers[topic];
        } // 离开作用域，锁自动释放！这步极其关键，防止回调中再次调用总线引发死锁

        // 在无锁状态下遍历执行回调，避免阻塞总线
        for (const auto& callback : callbacksToExecute) {
            if (callback) {
                // 注：若回调极其耗时，建议在此处使用 QtConcurrent::run 将其扔进线程池
                callback(payload);
            }
        }
    }

    /**
     * @brief 订阅主题
     * @param topic 消息主题
     * @param callback 收到消息时的回调函数
     */
    void subscribe(const QString& topic, std::function<void(const QByteArray&)> callback) override {
        QMutexLocker locker(&m_mutex);
        m_subscribers[topic].append(callback);
        qDebug() << "[LocalEventBus] Subscriber added. Topic:" << topic
                 << "Total subscribers:" << m_subscribers[topic].size();
    }

private:
    // 核心数据结构：主题 -> 回调函数列表
    QHash<QString, QList<std::function<void(const QByteArray&)>>> m_subscribers;

    // 互斥锁，保证多线程环境下的安全
    QMutex m_mutex;
};

#endif // LOCALEVENTBUS_H
