import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Car.Backend 1.0
import "UI/Views"
import "UI/Components"

Window {
    id: rootWindow
    width: 1280
    height: 720
    visible: true
    title: qsTr("智能座舱 IVI - V1.5")
    color: "#121212"

    // 👇 全局历史任务模型
    ListModel {
        id: recentAppsModel
    }

    // 记录打开的应用
    function recordAppOpened(appName, appIcon, appType) {
        for (let i = 0; i < recentAppsModel.count; i++) {
            if (recentAppsModel.get(i).name === appName) {
                recentAppsModel.remove(i)
                break
            }
        }
        recentAppsModel.insert(0, {"name": appName, "icon": appIcon, "type": appType})
    }

    // ==========================================
    // 👇 核心：统一的页面导航调度器
    // 无论是人类点 Dock 栏，还是 AI 发指令，都走这里
    // ==========================================
    function navigateToPage(pageName, qmlPath = "") {
        // 1. 关掉所有悬浮层，清理系统状态
        controlCenter.visible = false
        taskViewer.visible = false

        // 2. 多任务后台特殊处理
        if (pageName === "recent") {
            taskViewer.visible = true
            return
        }

        // 3. 任何页面跳转，先清空历史栈回到主页
        if (stackView.depth > 1) {
            stackView.pop(null)
        }

        if (pageName === "home") {
            return
        }

        // 4. 跳转到目标页面
        if (pageName === "ac") {
            recordAppOpened("空调", "❄️", "system://ac")
            stackView.push(Qt.resolvedUrl("UI/Views/HvacPanel.qml"))
        } else if (pageName === "media") {
            recordAppOpened("音乐", "🎵", "system://media")
            stackView.push(Qt.resolvedUrl("UI/Views/MediaPanel.qml"))
        } else if (pageName === "settings") {
            stackView.push(Qt.resolvedUrl("UI/Views/SettingsPanel.qml"))
        } else if (qmlPath !== "") {
            // 加载第三方 App
            recordAppOpened("第三方应用", "📦", qmlPath)
            stackView.push(qmlPath)
        }
    }


     // 挂载 AI 悬浮球
    AgentFloatingBall {
           id: agentBall
           anchors.right: parent.right
           anchors.bottom: parent.bottom
           anchors.rightMargin: 40
           anchors.bottomMargin: 100 // 👈 往上提一点，避开底部的 Dock 栏
           z: 9999 // 👈 极其关键：保证它悬浮在所有界面之上
       }

    // ==========================================
    // 👇 监听 AI 大总管发来的切页面信号
    // ==========================================
    Connections {
        target: SystemManager // 现在信号由大总管统一发出
        function onSwitchPageRequested(pageName) {
            console.log("【大总管】通知 UI 切换页面:", pageName)
            rootWindow.navigateToPage(pageName)
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 15

        TopBar {
            Layout.fillWidth: true
            Layout.preferredHeight: 45
        }

        StackView {
            id: stackView
            Layout.fillWidth: true
            Layout.fillHeight: true

            initialItem: HomeScreen {
                id: homeScreen
                onOpenAppRequested: (qmlPath) => {
                    // 人类点击桌面图标
                    if (qmlPath === "system://settings") rootWindow.navigateToPage("settings")
                    else if (qmlPath === "system://ac") rootWindow.navigateToPage("ac")
                    else if (qmlPath === "system://media") rootWindow.navigateToPage("media")
                    else rootWindow.navigateToPage("app", qmlPath)
                }
            }

            pushEnter: Transition {
                PropertyAnimation { property: "opacity"; from: 0.0; to: 1.0; duration: 250 }
                PropertyAnimation { property: "scale"; from: 0.9; to: 1.0; duration: 250 }
            }
            popExit: Transition {
                PropertyAnimation { property: "opacity"; from: 1.0; to: 0.0; duration: 250 }
            }
        }

        DockBar {
            Layout.fillWidth: true
            Layout.preferredHeight: 70

            onPageChanged: (pageName) => {
                console.log("【系统级接管】点击:", pageName, "当前栈深度:", stackView.depth)
                // 人类点击底部 Dock 栏，也走统一的导航器
                rootWindow.navigateToPage(pageName)
            }
        }
    }

    ControlCenter {
        id: controlCenter
    }

    // 多任务后台挂载
    TaskViewer {
        id: taskViewer
        anchors.fill: parent
        model: recentAppsModel

        onCloseRequested: {
            taskViewer.visible = false
        }

        onAppKilled: (appName) => {
            console.log("【系统】用户上滑杀掉应用:", appName)
            // 👇 修复：直接向大总管汇报杀进程请求！
            SystemManager.killApp(appName)
        }

        onResumeApp: (type, name, icon) => {
            taskViewer.visible = false
            if (type === "system://ac") stackView.push(Qt.resolvedUrl("UI/Views/HvacPanel.qml"))
            else if (type === "system://media") stackView.push(Qt.resolvedUrl("UI/Views/MediaPanel.qml"))
            else stackView.push(type)
        }
    }

    // 👇 底部上滑呼出手势
    MouseArea {
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: 20
        property point startPoint
        onPressed: (mouse) => startPoint = Qt.point(mouse.x, mouse.y)
        onPositionChanged: (mouse) => {
            if (startPoint.y - mouse.y > 50) taskViewer.visible = true
        }
    }

    // 右侧下拉 -> 呼出控制中心
    MouseArea {
        anchors.top: parent.top
        anchors.right: parent.right
        width: parent.width / 2
        height: 30
        property point startPoint
        onPressed: (mouse) => startPoint = Qt.point(mouse.x, mouse.y)
        onReleased: (mouse) => {
            if (mouse.y - startPoint.y > 50) controlCenter.visible = true
        }
    }

    // 左侧下拉 -> 呼出通知中心
    MouseArea {
        anchors.top: parent.top
        anchors.left: parent.left
        width: parent.width / 2
        height: 30
        property point startPoint
        onPressed: (mouse) => startPoint = Qt.point(mouse.x, mouse.y)
        onReleased: (mouse) => {
            if (mouse.y - startPoint.y > 50) console.log("左侧下拉：准备打开通知中心")
        }
    }
}
