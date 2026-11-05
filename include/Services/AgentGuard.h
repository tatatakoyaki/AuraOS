#ifndef AGENTGUARD_H
#define AGENTGUARD_H

#include <QObject>
#include <QJsonObject>
#include <QString>

class SystemManager;

/**
 * @brief AgentGuard (AI 安全网关 / 卫兵)
 *
 * 架构设计：
 * 作为 Agent 指令进入底层业务执行前的“唯一防线”。
 * 采用零信任策略，对所有外来指令执行多级安全审查：
 * 1. 身份审查：拦截未注册的非法 Agent。
 * 2. 权限审查：基于 RBAC（基于角色的访问控制），校验 Agent 是否拥有调用该工具的权限。
 * 3. 规则审查：依赖 RuleEngine 校验指令参数是否突破物理边界。
 *
 * 仅当所有审查通过后，才会将指令放行并转交至 SystemManager 执行。
 */
class AgentGuard : public QObject
{
    Q_OBJECT
public:
    explicit AgentGuard(SystemManager *sysManager, QObject *parent = nullptr);

    /**
     * @brief 核心审查与转发接口
     *
     * @param cmdId 唯一指令 ID (用于异步回调追踪)
     * @param agentId 发起请求的 Agent 标识
     * @param toolName 目标调用的工具/服务名称
     * @param args 调用参数
     */
    void verifyAndForward(const QString &cmdId, const QString &agentId, const QString &toolName, const QJsonObject &args);

private:
    SystemManager *m_sysManager; // 中央调度器指针，负责执行通过审查的合法指令
};

#endif // AGENTGUARD_H
