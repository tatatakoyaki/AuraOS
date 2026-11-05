#ifndef MOCKHVACHARDWARE_H
#define MOCKHVACHARDWARE_H

#include "HAL/IVehicleHardware.h"
#include <functional>

/**
 * @brief MockHvacHardware (空调硬件模拟实现)
 *
 * 架构设计：
 * 这是 HAL 层 IVehicleHardware 接口的 Mock 实现类。
 * 用于在没有真实车载硬件（如 CAN 总线收发器、空调 ECU）的开发环境下，
 * 模拟底层硬件的物理状态、报文延迟以及状态变更回调。
 *
 * 方便开发者在上层业务（大总管、UI、Agent）进行全链路闭环测试。
 */
class MockHvacHardware : public IVehicleHardware
{
public:
    MockHvacHardware();
    virtual ~MockHvacHardware() = default;

    // 模拟向底层硬件发送控制指令
    void setAcTemperature(int temp) override;

    // 模拟读取底层硬件的当前真实状态
    int getAcTemperature() const override;

    // 注册底层硬件状态变更的监听回调
    void setAcTemperatureChangedCallback(std::function<void(int)> callback) override;

private:
    int m_currentTemp;              // 模拟的硬件当前真实温度
    std::function<void(int)> m_callback; // 硬件状态变更时向上层触发的回调
};

#endif // MOCKHVACHARDWARE_H
