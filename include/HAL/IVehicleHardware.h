#ifndef IVEHICLEHARDWARE_H
#define IVEHICLEHARDWARE_H

#include <functional>

/**
 * @brief IVehicleHardware (车辆控制硬件抽象层接口)
 *
 * 架构设计：
 * 定义车辆底层硬件（如空调压缩机、电机控制器等）的标准控制接口。
 * 通过此抽象层，彻底屏蔽了底层通信差异（无论是高通 BSP、瑞萨 MCU，还是 CAN 总线/串口协议）。
 * 上层业务只依赖此纯 C++ 接口，实现真正意义上的“软件定义汽车”与跨平台移植。
 */
class IVehicleHardware {
public:
    virtual ~IVehicleHardware() = default;

    /**
     * @brief 设置空调目标温度（通过 CAN 或串口下发指令）
     * @param temp 目标温度值（需在物理安全范围内，如 16-30 度）
     */
    virtual void setAcTemperature(int temp) = 0;

    /**
     * @brief 获取底层硬件当前的真实温度
     * @return 当前温度值
     */
    virtual int getAcTemperature() const = 0;

    /**
     * @brief 注册温度变化回调
     *
     * 说明：底层硬件状态可能会因外部因素（如物理按键、车内温度传感器）发生改变。
     * 上层业务通过注册此回调，可实时监听底层状态，实现 UI 与底层硬件的双向绑定。
     *
     * @param callback 回调函数，参数为底层发生变更后的最新真实温度
     */
    virtual void setAcTemperatureChangedCallback(std::function<void(int)> callback) = 0;
};

#endif // IVEHICLEHARDWARE_H
