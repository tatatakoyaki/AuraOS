#include "Services/AgentGuard.h"
#include "Core/AgentRegistry.h"
#include "Core/RuleEngine.h"
#include "Services/systemmanager.h"

#include <QJsonArray>
#include <QDebug>

AgentGuard::AgentGuard(SystemManager *sysManager, QObject *parent)
    : QObject{parent}, m_sysManager(sysManager)
{
}

/**
 * @brief 核心审查与转发接口
 *
 * 采用零信任策略，依次进行三重校验。仅当所有校验均通过后，才将指令放行。
 */
void AgentGuard::verifyAndForward(const QString &cmdId, const QString &agentId, const QString &toolName, const QJsonObject &args)
{
    qDebug() << "[AgentGuard] Verifying request from Agent:" << agentId << " Tool:" << toolName;

    // ==========================================
    // 校验 1：零信任身份认证
    // 拒绝未在 AgentRegistry 中注册的非法 Agent
    // ==========================================
    if (!AgentRegistry::instance().exists(agentId)) {
        qWarning() << "[AgentGuard] Blocked: Unregistered Agent!" << agentId;
        m_sysManager->rejectAgentCommand(cmdId, "未授权的智能体，拒绝访问");
        return;
    }

    // ==========================================
    // 校验 2：RBAC 权限白名单校验
    // 检查该 Agent 是否拥有调用此工具的权限
    // ==========================================
    QJsonObject profile = AgentRegistry::instance().getAgentProfile(agentId);
    QJsonArray allowedTools = profile["allowed_tools"].toArray();

    bool hasPermission = false;
    for (const auto& tool : allowedTools) {
        if (tool.toString() == toolName) {
            hasPermission = true;
            break;
        }
    }

    if (!hasPermission) {
        qWarning() << "[AgentGuard] Blocked: Agent" << agentId << "has no permission to call" << toolName;
        m_sysManager->rejectAgentCommand(cmdId, "权限不足，该操作已被拦截");
        return;
    }

    // ==========================================
    // 校验 3：业务规则与物理边界校验
    // 交由 RuleEngine 检查参数是否突破物理极限
    // ==========================================
    if (!RuleEngine::instance().checkBusinessRule(toolName, args)) {
        qWarning() << "[AgentGuard] Blocked: Violation of physical safety rules.";
        m_sysManager->rejectAgentCommand(cmdId, "违反安全规则，已被系统拦截");
        return;
    }

    // ==========================================
    // 校验通过，安全放行至 SystemManager 执行
    // ==========================================
    qDebug() << "[AgentGuard] All checks passed. Forwarding to SystemManager.";
    m_sysManager->requestAgentAction(cmdId, agentId, toolName, args);
}
