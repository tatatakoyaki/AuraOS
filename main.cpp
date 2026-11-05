#include <QGuiApplication>
#include <QQmlApplicationEngine>

// ==========================================
// 核心底层：服务注册中心
// ==========================================
#include "Core/ServiceRegistry.h"

// ==========================================
// 硬件抽象层 (HAL) - 模拟实现
// 未来切至真车环境时，替换为 RealXxxHardware 即可
// ==========================================
#include "Platform/Mock/MockHvacHardware.h"
#include "Platform/Mock/MockMediaHardware.h"

// ==========================================
// 业务逻辑层 (Services) - 具体的功能业务管家
// ==========================================
#include "Services/hvachandler.h"
#include "Services/mediahandler.h"
#include "Services/appmanager.h"
#include "Services/widgetmanager.h"
#include "Services/audiorecorder.h"

// ==========================================
// 中央调度层 (SystemManager) 与 通信安全层
// ==========================================
#include "Services/systemmanager.h"
#include "Services/AgentGuard.h"
#include "Agent/agentbridge.h"

/**
 * @brief 应用程序主入口
 *
 * 该函数负责整个系统的依赖注入与模块装配。装配顺序严格遵循：
 * HAL 硬件层 -> 业务逻辑层 -> 服务注册中心 -> 中央调度与安全网关 -> UI 引擎。
 */
int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    QGuiApplication app(argc, argv);

    // =========================================================
    // 阶段一：硬件抽象层装配 (HAL)
    // 实例化底层模拟硬件。在未来整车项目中，仅需将此处的 Mock 替换为真实的芯片驱动。
    // =========================================================
    MockHvacHardware mockHvacHardware;
    MockMediaHardware mockMediaHardware;

    // =========================================================
    // 阶段二：业务逻辑层装配 (Services)
    // 将硬件接口注入至业务管家，遵循“先底层，后上层”的依赖原则。
    // =========================================================
    HvacHandler myHvac(&mockHvacHardware);
    MediaHandler myMedia(&mockMediaHardware);
    AppManager myAppManager;
    WidgetManager myWidgetManager;
    AudioRecorder myAudioRecorder;

    // =========================================================
    // 阶段三：服务注册中心装配 (Registry)
    // 将所有业务管家注册至全局微内核中心，以便中央调度器与 AI 网关进行动态查找。
    // =========================================================
    ServiceRegistry::instance().registerService("HvacService", &myHvac);
    ServiceRegistry::instance().registerService("MediaService", &myMedia);
    ServiceRegistry::instance().registerService("AppService", &myAppManager);
    ServiceRegistry::instance().registerService("WidgetService", &myWidgetManager);
    ServiceRegistry::instance().registerService("AudioService", &myAudioRecorder);

    // =========================================================
    // 阶段四：中央调度层装配 (SystemManager)
    // 实例化大总管。其内部会自动通过 ServiceRegistry 动态获取依赖，无需外部传入。
    // 注意：此行必须在所有服务注册完毕后执行。
    // =========================================================
    SystemManager mySystemManager;

    // =========================================================
    // 阶段五：安全网关与通信层装配 (AgentGuard & AgentBridge)
    // 装配逻辑：大总管 -> 安全网关 -> 通信网关。
    // 确保所有外来 AI 指令均需通过安全网关 (AgentGuard) 的零信任审查。
    // =========================================================
    AgentGuard myAgentGuard(&mySystemManager);
    AgentBridge myAgentBridge(&myAgentGuard, &mySystemManager);

    // =========================================================
    // 阶段六：向 QML 暴露全局单例接口
    // 严格的权限控制：前端 UI 仅可访问大总管与通信网关，底层业务组件完全隐藏。
    // =========================================================
    qmlRegisterSingletonInstance("Car.Backend", 1, 0, "SystemManager", &mySystemManager);
    qmlRegisterSingletonInstance("Car.Backend", 1, 0, "AgentBridge", &myAgentBridge);

    // =========================================================
    // 阶段七：启动 QML 渲染引擎
    // =========================================================
    QQmlApplicationEngine engine;
    const QUrl url(QStringLiteral("qrc:/main.qml"));

    // 监听引擎加载状态，若主 QML 加载失败则直接退出程序
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);

    engine.load(url);

    return app.exec();
}
