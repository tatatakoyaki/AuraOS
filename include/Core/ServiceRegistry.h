#ifndef SERVICEREGISTRY_H
#define SERVICEREGISTRY_H

#include <QObject>
#include <QHash>
#include <QMutex>
#include <QMutexLocker>
#include <QDebug>

/**
 * @brief ServiceRegistry (微内核服务注册中心)
 *
 * 架构设计：
 * 实现服务发现 (Service Discovery) 机制。所有底层业务组件 (Handler/Service) 在启动时
 * 必须向此注册中心登记。上层调度器 (如 SystemManager) 通过名称动态查表获取服务指针，
 * 从而彻底解耦上层业务与底层具体实现，支持服务的按需加载与动态热插拔。
 *
 * 注意：内部通过 QMutex 保证多线程环境下的注册与查找安全。
 */
class ServiceRegistry
{
public:
    // 获取全局唯一的注册中心实例
    static ServiceRegistry& instance() {
        static ServiceRegistry registry;
        return registry;
    }

    /**
     * @brief 注册服务
     * @param serviceName 服务唯一标识名（如 "HvacService"）
     * @param service 服务对象的基类指针 (QObject*)
     */
    void registerService(const QString& serviceName, QObject* service) {
        QMutexLocker locker(&m_mutex);
        m_services[serviceName] = service;
        qDebug() << "[ServiceRegistry] Service registered:" << serviceName;
    }

    /**
     * @brief 获取服务
     * @param serviceName 服务唯一标识名
     * @return 服务对象指针，若不存在则返回 nullptr
     */
    QObject* getService(const QString& serviceName) {
        QMutexLocker locker(&m_mutex);
        if (m_services.contains(serviceName)) {
            return m_services.value(serviceName);
        }
        qWarning() << "[ServiceRegistry] Service NOT found:" << serviceName;
        return nullptr;
    }

private:
    ServiceRegistry() = default;
    ~ServiceRegistry() = default;

    // 禁用拷贝构造和赋值，确保单例的绝对唯一
    ServiceRegistry(const ServiceRegistry&) = delete;
    ServiceRegistry& operator=(const ServiceRegistry&) = delete;

private:
    QHash<QString, QObject*> m_services;
    QMutex m_mutex;
};

#endif // SERVICEREGISTRY_H
