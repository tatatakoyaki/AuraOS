#ifndef BLACKBOARD_H
#define BLACKBOARD_H

#include <QObject>
#include <QVariantMap>
#include <QMutex>
#include <QMutexLocker>
#include <QDebug>

/**
 * @brief Blackboard (全局黑板 / 上下文环境中心)
 *
 * 架构设计：
 * 基于“黑板模式 (Blackboard Pattern)”，作为整个座舱系统的“单一事实来源 (Single Source of Truth)”。
 * 底层的传感器数据（如车速、环境温度）和业务状态（如当前场景）都会被写入这里。
 * 所有 Agent 均可访问此黑板，以获取全局上下文，从而做出更智能的协同决策。
 *
 * 注意：所有读写操作均通过 QMutex 保护，以保证多线程环境下的数据安全。
 */
class Blackboard : public QObject {
    Q_OBJECT
public:
    // 获取全局唯一的黑板实例
    static Blackboard& instance() {
        static Blackboard board;
        return board;
    }

    /**
     * @brief 更新黑板上的全局状态（支持任意类型 QVariant）
     * @param key 状态键名（如 "Speed", "Weather"）
     * @param value 状态值
     */
    void updateContext(const QString& key, const QVariant& value) {
        QMutexLocker locker(&m_mutex);
        m_context[key] = value;

        // 解除锁后再广播，防止槽函数内部发生死锁或阻塞
        locker.unlock();
        emit contextChanged(key, value);
    }

    /**
     * @brief 读取黑板上指定的全局状态
     * @param key 状态键名
     * @return 状态值，若不存在则返回无效 QVariant
     */
    QVariant getContext(const QString& key) {
        QMutexLocker locker(&m_mutex);
        return m_context.value(key);
    }

    /**
     * @brief 获取黑板全量状态的快照（可用于打包发送给大模型）
     * @return QVariantMap 包含所有当前上下文的键值对
     */
    QVariantMap getFullContext() {
        QMutexLocker locker(&m_mutex);
        return m_context;
    }

signals:
    // 当黑板上的某个状态发生变化时触发，供外部订阅
    void contextChanged(const QString& key, const QVariant& value);

private:
    Blackboard() = default;
    ~Blackboard() = default;

    // 禁止拷贝构造和赋值，确保单例的唯一性
    Blackboard(const Blackboard&) = delete;
    Blackboard& operator=(const Blackboard&) = delete;

private:
    QVariantMap m_context;
    QMutex m_mutex;
};

#endif // BLACKBOARD_H
