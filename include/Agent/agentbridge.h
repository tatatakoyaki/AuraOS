#ifndef AGENTBRIDGE_H
#define AGENTBRIDGE_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QHash>
#include <QJsonObject>
#include <QNetworkAccessManager>

class AgentGuard;
class SystemManager;

/**
 * @brief AgentBridge (AI 通信网关 / 外交官)
 *
 * 职责：
 * 1. 对内：启动本地 TCP Server (8003 端口)，接收来自 Python 大模型发来的 HTTP 工具调用请求。
 * 2. 对外：封装 HTTP 客户端，将前端/系统的文本指令发送给 Python AI 服务。
 * 3. 协同：不直接操作底层业务，所有大模型指令必须先递交至 AgentGuard 进行安全审查。
 */
class AgentBridge : public QObject
{
    Q_OBJECT
public:
    explicit AgentBridge(AgentGuard *agentGuard, SystemManager *sysManager, QObject *parent = nullptr);

    /**
     * @brief 将用户的文本消息发送给 Python AI 大脑
     * @param message 用户输入的自然语言
     */
    Q_INVOKABLE void sendMessageToAgent(const QString &message);

signals:
    void agentReplied(const QString &replyText);

private slots:
    void onNewConnection();
    void onReadyRead();

    // 接收大总管处理完毕后的回调，用于给 Python 返回 HTTP 响应
    void onAgentCommandCompleted(const QString &cmdId, bool success, const QString &msg);

private:
    QTcpServer *m_server;
    QNetworkAccessManager *m_networkManager;

    // 依赖注入的安全网关与中央调度器
    AgentGuard *m_agentGuard;
    SystemManager *m_sysManager;

    // 用于暂存处于待处理状态的 HTTP 请求 Socket（等待异步处理结果）
    QHash<QString, QTcpSocket*> m_pendingRequests;
};

#endif // AGENTBRIDGE_H
