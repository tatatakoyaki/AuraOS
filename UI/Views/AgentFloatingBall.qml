import QtQuick 2.15
import QtQuick.Controls 2.15
import Car.Backend 1.0

Item {
    id: root
    width: 80; height: 80

    // 状态：0=待机，1=录音中，2=思考中
    property int agentState: 0

    Rectangle {
        id: ball
        anchors.fill: parent
        radius: width / 2
        color: agentState === 1 ? "#FF3B30" : (agentState === 2 ? "#8801E6DE" : "#01E6DE")
        Behavior on color { ColorAnimation { duration: 200 } }

        Text {
            anchors.centerIn: parent
            text: agentState === 1 ? "🔴" : "🤖"
            font.pixelSize: 40
        }

        MouseArea {
            anchors.fill: parent
            onPressed: {
                agentState = 1
                // 👇 核心修复 1：呼叫大总管开始录音，不再直接调底层！
                SystemManager.startVoiceRecord()
            }
            onReleased: {
                agentState = 2
                // 👇 核心修复 2：呼叫大总管停止录音
                SystemManager.stopVoiceRecord()
            }
        }
    }

    // 👇 核心修复 3：监听大总管的信号，不再监听底层的 AudioRecorder
    Connections {
        target: SystemManager
        function onVoiceResultReady(text) {
            console.log("🤖 大总管转发的 AI 回复:", text)
            agentState = 0 // 恢复待机
        }
    }
}
