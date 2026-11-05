# 🚀 AuraOS - Microkernel-based Intelligent Cockpit OS

> **“当物理硬件遇到性能瓶颈，未来的智能设备都将成为 AI 的载体。”**
> AuraOS 是一个基于 C++ 与 QML 构建的微内核车载智能座舱框架。项目严格遵循 MVVM 与前后端分离思想，已完成微内核解耦、HAL 硬件抽象以及**单 Agent 语音控车全链路闭环**。同时，本架构从设计之初就为**未来多智能体（Multi-Agent）协同**预留了完整的安全网关、权限注册中心与黑板通信骨架。

---

## 📢 项目状态与诚实说明 (Project Status)

作为一个个人开源项目，受限于硬件资源与算力，我在这里诚实地交代项目的进展与边界：

1. **已落地验证的底座**：微内核服务注册发现（ServiceRegistry）、HAL 硬件抽象层（Mock）、MVVM 数据绑定、以及单 Agent 的 ASR -> LLM -> 车控 -> TTS 完整闭环，均已在本地 Linux 环境下跑通。
2. **预留的骨架与设计**：多智能体协同（Multi-Agent）、场景黑板（Blackboard）、RBAC 权限校验（AgentGuard）、物理边界规则引擎（RuleEngine），目前**已完成架构设计与 C++ 骨架代码**。由于缺乏丰富的真实模型和车队环境，这部分主要作为“架构可行性探索”，期待未来有更多的开发者一起接入真实的模型进行实验验证。

---

## ✨ 核心特性 (Core Features)

*   🏛️ **微内核架构 (Microkernel)**：基于 `ServiceRegistry` 实现业务模块的动态注册与发现。中央调度器（SystemManager）内部彻底清空依赖指针，通过动态查表实现极高的可裁剪性。
*   🎨 **MVVM 视图分离**：前端 QML 仅负责渲染与交互，C++ 业务层通过 `Q_PROPERTY` 与信号槽单向驱动 UI 刷新。
*   🔌 **硬件抽象层 (HAL)**：定义 `IVehicleHardware` 标准纯虚接口。底层高通/瑞萨或 CAN 总线只需实现此接口，业务层代码零修改。
*   🧠 **端云协同语音闭环**：基于 WebSocket 的流式音频推流，结合 Python (Whisper + LangChain) 微服务，实现真实的自然语言控车体验。
*   🛡️ **为多智能体协作而生的安全骨架**：设计了 `AgentGuard` 零信任网关、`AgentRegistry` 四要素强校验档案、以及 `Blackboard` 全局状态黑板，为未来的多 Agent 协同提供了严密的准入与通信规范。

---

## 🏗️ 系统架构图 (Architecture)

graph TD
    %% 表现层
    subgraph UI_Layer ["表现层 / QML - MVVM"]
        MainUI["Main Screen"]
        DashUI["Dashboard"]
        Dock["Dock Bar"]
    end

    %% 中央调度与安全层
    subgraph Core_Layer ["微内核调度与安全层 / C++"]
        SM["SystemManager<br/>中央调度器"]
        AG["AgentGuard<br/>AI 安全网关 (骨架)"]
        AR["AgentRegistry<br/>Agent 档案中心 (骨架)"]
        RE["RuleEngine<br/>物理规则引擎 (骨架)"]
        BB["Blackboard<br/>全局黑板 (骨架)"]
        AgentBridge["AgentBridge<br/>通信网关"]
    end

    %% 业务逻辑层
    subgraph Service_Layer ["业务逻辑层 / Services"]
        Hvac["HvacHandler"]
        Media["MediaHandler"]
        App["AppManager"]
    end

    %% 硬件抽象层
    subgraph HAL_Layer ["硬件抽象层 / HAL"]
        IHvac["IHvacHardware"]
        IMedia["IMediaHardware"]
    end

    %% AI 外部服务
    subgraph AI_Cloud ["云端 / 本地 AI 微服务 (已跑通)"]
        Python["Python + LangChain"]
        Whisper["Whisper ASR / TTS"]
    end

    %% 连线关系
    UI_Layer <-->|Q_PROPERTY / Signals| SM
    SM <-->|动态查表| Service_Layer
    Service_Layer <-->|依赖注入| HAL_Layer
    
    Python <-->|HTTP / WebSocket| AgentBridge
    AgentBridge -->|提交审查| AG
    AG -->|权限与规则校验| AR & RE
    AG -->|审核通过| SM
    
🛠️ 技术栈 (Tech Stack)
前端/HMI：Qt 5.15 / QML / Qt Quick (MVVM)

后端/中间件：C++ 11/14 / Qt Framework (Microkernel & HAL)

AI 微服务 (Python)：FastAPI / LangChain / Faster-Whisper / Edge-TTS

通信：HTTP (RESTful) / WebSocket (流式音频) / 本地信号槽

构建：CMake

开发与测试环境：Ubuntu 22.04 LTS (Linux Kernel 5.x) / Qt Creator / CMake 3.16+ / Python 3.10+

🚀 快速开始 (Quick Start)
(需要安装 Qt 5.15+ 开发环境与 CMake)
# 克隆项目
git clone https://github.com/tatatakoyaki/AuraOS.git
cd AuraOS

# 构建项目
mkdir build && cd build
cmake ..
make -j4

# 运行程序
./AuraOS

2. 启动 AI 智能体服务 (ai_server)
(需要 Python 3.10+ 环境)
cd ai_server
python -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt

# 配置千问 API Key 
export DASHSCOPE_API_KEY="真实千问API_KEY" 

python main.py

3. 启动音乐网关服务 (music_gateway_server)
cd music_gateway_server
python -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
python main.py

📂 工程目录结构 (Directory Structure)
AuraOS/
├── include/
│   ├── Core/           # 微内核核心 (注册中心, 规则引擎, 黑板)
│   ├── HAL/            # 硬件抽象层接口
│   ├── Platform/Mock/  # 模拟硬件实现 (Mock HAL)
│   ├── Services/       # 业务逻辑 (大总管, 卫兵, 业务Handler)
│   └── Agent/          # 通信网关
├── src/                # C++ 源文件 (与 include 对应)
├── ai_server/          # Python AI 智能体微服务
├── music_gateway_server/ # Python 网易云音乐网关微服务
├── UI/                 # QML 前端界面
└── CMakeLists.txt      # 构建脚本

🗺️ 演进路线 (Roadmap)
☑ 基础 HMI 与 MVVM 架构落地
☑ 微内核化重构 (服务注册、HAL 隔离)
☑ 单 Agent 语音控车全链路 (Whisper -> LangChain -> 车控 -> TTS)
☑ Agent 安全网关与规则引擎骨架搭建
□ 多智能体协同实验 (验证 Blackboard 黑板通信与 Leader 机制)
□ 多屏互动与分布式 (基于 Socket / DDS 的跨屏通信)
□ 端侧小模型部署 (结合 NPU 提升响应速度)

⚠️ 免责声明 (Disclaimer)
本项目仅作为个人学习与开源技术验证原型。当前底层硬件均采用 Mock 模拟，未连接真实车控总线（如 CAN 总线）。由于大模型存在幻觉，请勿直接用于实际车辆控制。音乐网关仅供技术测试，请支持正版音乐，严禁商业用途。
