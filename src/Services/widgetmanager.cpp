#include "Services/widgetmanager.h"
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>

WidgetManager::WidgetManager(QObject *parent) : QObject{parent}
{
    QString configPath = QDir::homePath() + "/SmartIVI/widgets_layout.json";
    QFile file(configPath);

    if (file.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        QJsonArray arr = doc.array();
        for (int i = 0; i < arr.size(); ++i) {
            QJsonObject obj = arr[i].toObject();
            int sw = obj["spanW"].toInt();
            int sh = obj["spanH"].toInt();

            // 防御性校验：忽略非法的尺寸数据（宽或高 <= 0），防止前端渲染异常
            if (sw <= 0 || sh <= 0) continue;

            m_widgets.append(QVariantMap{
                {"type", obj["type"].toString()},
                {"x", obj["x"].toInt()},
                {"y", obj["y"].toInt()},
                {"spanW", sw},
                {"spanH", sh}
            });
        }
        file.close();

        // 兜底策略：若配置文件为空或全为脏数据，则加载系统默认布局
        if (m_widgets.isEmpty()) {
            qWarning() << "[WidgetManager] Invalid or empty config detected. Loading default layout.";
            m_widgets.append(QVariantMap{{"type", "wifi"}, {"x", 0}, {"y", 0}, {"spanW", 1}, {"spanH", 1}});
            m_widgets.append(QVariantMap{{"type", "media"}, {"x", 1}, {"y", 0}, {"spanW", 2}, {"spanH", 2}});
            m_widgets.append(QVariantMap{{"type", "ac"}, {"x", 3}, {"y", 0}, {"spanW", 2}, {"spanH", 1}});
        } else {
            qDebug() << "[WidgetManager] Layout restored successfully. Widgets count:" << m_widgets.size();
        }
    } else {
        // 首次运行或配置文件不存在时的初始化逻辑
        m_widgets.append(QVariantMap{{"type", "wifi"}, {"x", 0}, {"y", 0}, {"spanW", 1}, {"spanH", 1}});
        m_widgets.append(QVariantMap{{"type", "media"}, {"x", 1}, {"y", 0}, {"spanW", 2}, {"spanH", 2}});
        m_widgets.append(QVariantMap{{"type", "ac"}, {"x", 3}, {"y", 0}, {"spanW", 2}, {"spanH", 1}});
        qDebug() << "[WidgetManager] Config file not found. Initialized with default layout.";
    }
}

QVariantList WidgetManager::widgets() const { return m_widgets; }

/**
 * @brief 保存前端传来的小组件布局数据并持久化到 JSON 文件
 * @param newWidgets 包含组件类型、坐标、宽高信息的数据列表
 */
void WidgetManager::saveLayout(const QVariantList &newWidgets)
{
    // 防御性校验：拒绝空列表落库
    if (newWidgets.isEmpty()) {
        qWarning() << "[WidgetManager] Save rejected: Empty payload.";
        return;
    }

    // 防御性校验：遍历检查，过滤包含非法尺寸（宽或高 <= 0）的数据
    for (const QVariant &item : newWidgets) {
        QVariantMap map = item.toMap();
        if (map["spanW"].toInt() <= 0 || map["spanH"].toInt() <= 0) {
            qWarning() << "[WidgetManager] Save rejected: Invalid data detected (spanW or spanH <= 0).";
            return;
        }
    }

    m_widgets = newWidgets;
    emit widgetsChanged();

    // 序列化为 JSON 数组
    QJsonArray arr;
    for (int i = 0; i < m_widgets.size(); ++i) {
        QVariantMap map = m_widgets[i].toMap();
        QJsonObject obj;
        obj["type"] = map["type"].toString();
        obj["x"] = map["x"].toInt();
        obj["y"] = map["y"].toInt();
        obj["spanW"] = map["spanW"].toInt();
        obj["spanH"] = map["spanH"].toInt();
        arr.append(obj);
    }

    // 持久化写入文件
    QString configPath = QDir::homePath() + "/SmartIVI/widgets_layout.json";
    QFile file(configPath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        file.write(QJsonDocument(arr).toJson());
        file.close();
        qDebug() << "[WidgetManager] Layout persisted to JSON successfully.";
    } else {
        qCritical() << "[WidgetManager] Failed to write layout to JSON file!";
    }
}
