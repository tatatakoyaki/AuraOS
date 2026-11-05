#include "Services/hvachandler.h"
#include "HAL/IVehicleHardware.h"
#include <QDebug>

HvacHandler::HvacHandler(IVehicleHardware *hardware, QObject *parent)
    : QObject{parent}, m_hardware(hardware)
{
    // 1. 初始化时，从底层硬件读取真实的初始物理状态
    if (m_hardware) {
        m_targetTemp = m_hardware->getAcTemperature();

        // 2. 向底层硬件注册状态变更回调。
        // 当底层硬件状态发生改变（如收到 CAN 报文、或被物理按键修改），会触发此回调。
        m_hardware->setAcTemperatureChangedCallback([this](int newTemp) {
            this->onHardwareTempChanged(newTemp);
        });
    } else {
        // 兜底逻辑：未注入硬件时，使用默认值以保证 UI 能正常渲染
        m_targetTemp = 24;
        qWarning() << "[HvacHandler] Hardware interface not injected! Using default temp.";
    }
}

int HvacHandler::targetTemp() const
{
    return m_targetTemp;
}

/**
 * @brief 增量调节温度 (供 QML UI 调用)
 */
void HvacHandler::changeTargetTemp(int delta)
{
    // 业务层预先计算并做边界防呆校验
    int newTemp = m_targetTemp + delta;
    if (newTemp < 16) newTemp = 16;
    if (newTemp > 30) newTemp = 30;

    if (newTemp == m_targetTemp) return;

    // 核心架构约束：不直接修改业务层的缓存值，而是将控制请求下发给 HAL 硬件，
    // 等待硬件执行完毕后的异步回调来更新状态。
    if (m_hardware) {
        qDebug() << "[HvacHandler] Requesting hardware to set temp:" << newTemp;
        m_hardware->setAcTemperature(newTemp);
    }
}

/**
 * @brief 接收底层硬件状态变更的异步回调
 * @param newTemp 底层硬件反馈的最新真实温度
 */
void HvacHandler::onHardwareTempChanged(int newTemp)
{
    if (m_targetTemp == newTemp) return;

    qDebug() << "[HvacHandler] Hardware callback received. Temp updated to:" << newTemp;
    m_targetTemp = newTemp;

    // 广播信号，通知 QML 引擎刷新绑定此属性的 UI 元素
    emit targetTempChanged();
}

/**
 * @brief 绝对温度设置 (供大总管或 AI Agent 直接调用)
 */
void HvacHandler::setTargetTemp(int targetTemp)
{
    // 物理边界防呆校验
    if (targetTemp < 16) targetTemp = 16;
    if (targetTemp > 30) targetTemp = 30;

    if (m_targetTemp == targetTemp) return;

    // 同样不直接修改内存，而是请求底层 HAL 硬件执行
    if (m_hardware) {
        qDebug() << "[HvacHandler] Absolute temp request received:" << targetTemp;
        m_hardware->setAcTemperature(targetTemp);
    }
}
