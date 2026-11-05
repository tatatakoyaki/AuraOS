#include "Platform/Mock/MockHvacHardware.h"
#include <QDebug>

MockHvacHardware::MockHvacHardware()
    : m_currentTemp(24) // 初始化默认物理温度
{
    qDebug() << "[MockHvacHardware] Initialized. Default temp: 24C";
}

/**
 * @brief 模拟底层硬件接收控制指令
 *
 * 在真实量产车中，此处通常封装了将业务参数转化为底层 CAN 总线报文
 * 或串口字节流的具体实现。
 */
void MockHvacHardware::setAcTemperature(int temp)
{
    qDebug() << "[MockHvacHardware] Simulating CAN frame Tx: set temp ->" << temp;

    // 更新模拟的物理硬件寄存器/内存状态
    m_currentTemp = temp;

    // 模拟底层硬件处理完成后的异步回调。
    // 真实场景中，该回调通常由硬件中断或底层通信接收线程触发。
    if (m_callback) {
        m_callback(m_currentTemp);
    }
}

/**
 * @brief 读取底层硬件的当前物理状态
 */
int MockHvacHardware::getAcTemperature() const
{
    return m_currentTemp;
}

/**
 * @brief 注册底层硬件的状态变更监听回调
 */
void MockHvacHardware::setAcTemperatureChangedCallback(std::function<void(int)> callback)
{
    m_callback = callback;
}
