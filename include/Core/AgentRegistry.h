#ifndef AGENTREGISTRY_H
#define AGENTREGISTRY_H

#include <QObject>
#include <QHash>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

/**
 * @brief AgentRegistry (Agent 档案注册中心)
 *
 * 架构设计：
 * 作为微内核中所有 AI Agent 的“花名册”。
 * 不限制 Agent 的物理部署位置与算力来源，只接收并管理其能力契约。
 * 强制校验核心四要素 (agent_id, role_name, compute_source, allowed_tools)，
 * 确保任何接入系统的 Agent 均具备明确的身份与权限边界。
 */
class AgentRegistry {
public:
    // 获取全局唯一的注册中心实例
    static AgentRegistry& instance() {
        static AgentRegistry registry;
        return registry;
    }

    /**
     * @brief 接收外部 JSON 档案并执行严格的四要素校验后注册
     * @param profile Agent 的 JSON 配置对象
     */
    void registerAgent(const QJsonObject& profile) {
        // 校验 1：必须包含 agent_id
        if (!profile.contains("agent_id") || profile["agent_id"].toString().isEmpty()) {
            qWarning() << "[AgentRegistry] Registration failed: Missing [agent_id]";
            return;
        }
        // 校验 2：必须包含 role_name
        if (!profile.contains("role_name") || profile["role_name"].toString().isEmpty()) {
            qWarning() << "[AgentRegistry] Registration failed: Missing [role_name]";
            return;
        }
        // 校验 3：必须包含 compute_source (算力来源)
        if (!profile.contains("compute_source") || profile["compute_source"].toString().isEmpty()) {
            qWarning() << "[AgentRegistry] Registration failed: Missing [compute_source]";
            return;
        }
        // 校验 4：必须包含 allowed_tools 数组
        if (!profile.contains("allowed_tools") || !profile["allowed_tools"].isArray()) {
            qWarning() << "[AgentRegistry] Registration failed: Missing [allowed_tools]";
            return;
        }

        // 所有校验通过，写入内存档案库
        QString agentId = profile["agent_id"].toString();
        m_agentProfiles[agentId] = profile;

        qDebug() << "[AgentRegistry] Agent registered successfully. ID:" << agentId
                 << "| Role:" << profile["role_name"].toString();
    }

    /**
     * @brief 根据 Agent ID 获取完整档案
     * @param agentId Agent 唯一标识
     * @return 档案 JSON 对象，不存在则返回空对象
     */
    QJsonObject getAgentProfile(const QString& agentId) {
        return m_agentProfiles.value(agentId, QJsonObject());
    }

    /**
     * @brief 检查指定 Agent 是否已注册
     * @param agentId Agent 唯一标识
     */
    bool exists(const QString& agentId) {
        return m_agentProfiles.contains(agentId);
    }

private:
    AgentRegistry() = default;
    ~AgentRegistry() = default;

    // 禁止拷贝与赋值，确保单例唯一性
    AgentRegistry(const AgentRegistry&) = delete;
    AgentRegistry& operator=(const AgentRegistry&) = delete;

private:
    QHash<QString, QJsonObject> m_agentProfiles;
};

#endif // AGENTREGISTRY_H
