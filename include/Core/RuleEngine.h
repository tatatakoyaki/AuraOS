#ifndef RULEENGINE_H
#define RULEENGINE_H

#include <QObject>
#include <QJsonObject>
#include <QHash>
#include <QDebug>

/**
 * @brief RuleEngine (业务规则引擎 / 物理边界护栏)
 *
 * 架构设计：
 * 将硬编码的安全边界（如温度上下限、音量极值）从业务逻辑中剥离。
 * 核心职责是作为 Agent 指令的“最后一道物理防线”，防止 AI 产生幻觉或发出越界指令。
 * 未来可无缝扩展为读取外部 JSON/YAML 配置文件，由 OEM 或产品厂商自定义规则。
 */
class RuleEngine {
public:
    // 获取规则引擎单例
    static RuleEngine& instance() {
        static RuleEngine engine;
        return engine;
    }

    /**
     * @brief 加载业务规则
     *
     * 当前以预置数据模拟，后续可扩展为从 rules.json 或云端下发配置中动态加载。
     */
    void loadRules() {
        // 配置：空调温度的物理限制范围
        QJsonObject acRule;
        acRule["min_temp"] = 16;
        acRule["max_temp"] = 30;
        m_rules["set_ac_temp"] = acRule;

        // 配置：音量的物理限制范围
        QJsonObject volumeRule;
        volumeRule["min_vol"] = 0;
        volumeRule["max_vol"] = 100;
        m_rules["change_volume"] = volumeRule;

        qDebug() << "[RuleEngine] Safety rules loaded, total:" << m_rules.size();
    }

    /**
     * @brief 核心校验接口（由 AgentGuard 卫兵调用）
     * @param toolName 目标调用的工具名
     * @param args 调用参数
     * @return true 校验通过；false 触发拦截
     */
    bool checkBusinessRule(const QString& toolName, const QJsonObject& args) {
        // 若无规则定义，默认放行（交由上层业务或 HAL 兜底）
        if (!m_rules.contains(toolName)) {
            return true;
        }

        QJsonObject rule = m_rules[toolName];

        // 校验：空调温度是否越界
        if (toolName == "set_ac_temp") {
            int temp = args["temperature"].toInt();
            int maxT = rule["max_temp"].toInt();
            int minT = rule["min_temp"].toInt();

            if (temp < minT || temp > maxT) {
                qWarning() << "[RuleEngine] Intercepted: Temp out of bounds" << temp
                           << "Allowed: [" << minT << "-" << maxT << "]";
                return false;
            }
        }

        // 校验：音量是否越界（示例扩展）
        if (toolName == "change_volume") {
            int vol = args["volume"].toInt();
            int maxV = rule["max_vol"].toInt();
            int minV = rule["min_vol"].toInt();

            if (vol < minV || vol > maxV) {
                qWarning() << "[RuleEngine] Intercepted: Volume out of bounds" << vol;
                return false;
            }
        }

        return true;
    }

private:
    RuleEngine() = default;
    QHash<QString, QJsonObject> m_rules;
};

#endif // RULEENGINE_H
