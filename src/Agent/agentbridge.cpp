#include "Agent/agentbridge.h"
#include "Services/AgentGuard.h"
#include "Services/systemmanager.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <QHostAddress>
#include <QUuid>
#include <QNetworkRequest>
#include <QNetworkReply>

AgentBridge::AgentBridge(AgentGuard *agentGuard, SystemManager *sysManager, QObject *parent)
    : QObject{parent}, m_agentGuard(agentGuard), m_sysManager(sysManager)
{
    // 启动本地 HTTP 服务，监听 8003 端口 (接收 Python 工具调用请求)
    m_server = new QTcpServer(this);
    if (m_server->listen(QHostAddress::LocalHost, 8003)) {
        qDebug() << "[AgentBridge] HTTP Tool Calling Server started on port 8003";
    } else {
        qCritical() << "[AgentBridge] Failed to bind port 8003! It may be occupied.";
    }

    connect(m_server, &QTcpServer::newConnection, this, &AgentBridge::onNewConnection);

    // 初始化网络管理器 (用于向 Python AI 服务发送文本请求)
    m_networkManager = new QNetworkAccessManager(this);

    // 监听大总管的异步执行结果，以便给 Python 返回 HTTP 响应
    connect(m_sysManager, &SystemManager::agentCommandCompleted,
            this, &AgentBridge::onAgentCommandCompleted);
}

/**
 * @brief 向 Python AI 大脑发送用户文本
 */
void AgentBridge::sendMessageToAgent(const QString &message)
{
    qDebug() << "[AgentBridge] Sending message to AI:" << message;

    QUrl url("http://127.0.0.1:8002/chat");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject json;
    json["message"] = message;

    QNetworkReply *reply = m_networkManager->post(request, QJsonDocument(json).toJson());

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray res = reply->readAll();
            QJsonDocument doc = QJsonDocument::fromJson(res);
            QString replyText = doc.object()["reply"].toString();

            qDebug() << "[AgentBridge] Received AI reply:" << replyText;
            emit agentReplied(replyText);
        } else {
            qWarning() << "[AgentBridge] AI request failed:" << reply->errorString();
            emit agentReplied("网络连接失败，无法联系 AI 大脑。");
        }
        reply->deleteLater();
    });
}

void AgentBridge::onNewConnection()
{
    QTcpSocket *socket = m_server->nextPendingConnection();
    connect(socket, &QTcpSocket::readyRead, this, &AgentBridge::onReadyRead);
    // 确保 Socket 在断开连接后及时释放，防止内存泄漏
    connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);
}

/**
 * @brief 解析来自 Python 的 HTTP 工具调用请求，并交由 AgentGuard 审查
 */
void AgentBridge::onReadyRead()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;

    QByteArray requestData = socket->readAll();

    // HTTP 报文头与报文体以 \r\n\r\n 分隔，提取 JSON Body
    int headerEndIndex = requestData.indexOf("\r\n\r\n");
    if (headerEndIndex == -1) return;

    QByteArray body = requestData.mid(headerEndIndex + 4);
    QJsonDocument doc = QJsonDocument::fromJson(body);
    if (doc.isNull() || !doc.isObject()) return;

    QJsonObject obj = doc.object();
    QString toolName = obj["tool"].toString();
    QJsonObject args = obj["args"].toObject();

    // 提取 Agent 身份标识，默认缺省为 MainAgent 方便测试
    QString agentId = obj["agent_id"].toString("MainAgent");

    qDebug() << "[AgentBridge] Tool call received:" << toolName << "from Agent:" << agentId;

    // 生成全局唯一指令 ID，并将 Socket 指针暂存，用于后续的异步响应
    QString cmdId = QUuid::createUuid().toString();
    m_pendingRequests.insert(cmdId, socket);

    // 核心链路：请求首先递交至安全网关 (AgentGuard) 进行合法性审查
    m_agentGuard->verifyAndForward(cmdId, agentId, toolName, args);
}

/**
 * @brief 接收大总管执行结果，构造 HTTP 响应回传给 Python
 */
void AgentBridge::onAgentCommandCompleted(const QString &cmdId, bool success, const QString &msg)
{
    if (m_pendingRequests.contains(cmdId)) {
        QTcpSocket *socket = m_pendingRequests.take(cmdId);
        if (!socket) return;

        // 构造标准 JSON 响应体
        QJsonObject responseObj;
        responseObj["status"] = success ? "success" : "error";
        if (!success) responseObj["error"] = msg;

        QByteArray responseBody = QJsonDocument(responseObj).toJson(QJsonDocument::Compact);

        // 拼装原始 HTTP 响应报文并发送
        QString httpResponse = "HTTP/1.1 200 OK\r\n";
        httpResponse += "Content-Type: application/json\r\n";
        httpResponse += "Content-Length: " + QString::number(responseBody.size()) + "\r\n";
        httpResponse += "Connection: close\r\n\r\n";

        socket->write(httpResponse.toUtf8());
        socket->write(responseBody);
        socket->flush();
        socket->disconnectFromHost();
    }
}
