#ifndef HVACHANDLER_H
#define HVACHANDLER_H

#include <QObject>

// 前向声明 HAL 接口，避免头文件循环包含
class IVehicleHardware;

/**
 * @brief HvacHandler (空调业务管家)
 *
 * 架构设计：
 * 位于业务逻辑层。采用依赖注入 (Dependency Injection) 接收底层 HAL 硬件接口。
 * 业务层本身不存储真实的物理状态，仅持有用于 UI 渲染的缓存值。
 * 所有控制指令均下发给底层硬件执行，待硬件回调后再更新缓存并通知 UI 刷新。
 */
class HvacHandler : public QObject
{
    Q_OBJECT
    // 暴露给 QML 的只读属性（UI 绑定此缓存值进行渲染）
    Q_PROPERTY(int targetTemp READ targetTemp NOTIFY targetTempChanged)

public:
    /**
     * @brief 构造函数，注入底层硬件抽象接口
     * @param hardware 实现了 IVehicleHardware 的硬件实例指针
     */
    explicit HvacHandler(IVehicleHardware *hardware, QObject *parent = nullptr);

    /**
     * @brief 获取当前 UI 渲染用的缓存温度
     */
    int targetTemp() const;

    /**
     * @brief 增量调节温度（供 QML 的加/减按钮调用）
     * @param delta 温度增量（如 +1 或 -1）
     */
    Q_INVOKABLE void changeTargetTemp(int delta);

    /**
     * @brief 设置绝对目标温度（供大总管或 AI Agent 直接调用）
     * @param targetTemp 目标温度值
     */
    Q_INVOKABLE void setTargetTemp(int targetTemp);

public slots:
    /**
     * @brief 接收底层硬件温度变更的回调槽
     * @param newTemp 底层硬件反馈的最新真实温度
     */
    void onHardwareTempChanged(int newTemp);

signals:
    // 数据变化广播，通知 QML 自动刷新界面
    void targetTempChanged();

private:
    IVehicleHardware *m_hardware; // 底层硬件抽象层接口指针
    int m_targetTemp;             // 当前温度缓存值（仅用于界面显示）
};

#endif // HVACHANDLER_H
