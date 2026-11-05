import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: taskViewer
    color: "#E6121212"
    visible: false
    z: 1000

    signal closeRequested
    signal resumeApp(string type, string name, string icon)
    signal appKilled(string appName)

    property alias model: taskList.model

    Text {
        anchors.top: parent.top
        anchors.topMargin: 40
        anchors.horizontalCenter: parent.horizontalCenter
        text: "正在运行的应用"
        color: "white"
        font.pixelSize: 24
        font.bold: true
    }

    ListView {
        id: taskList
        anchors.fill: parent
        anchors.topMargin: 100
        anchors.bottomMargin: 100
        orientation: ListView.Horizontal
        spacing: 40

        // 👇 核心1：滑动结束后按卡片吸附
        snapMode: ListView.SnapToItem

        // 👇 核心2：隐形的中心吸附框
        // 卡片宽度是 350，我们定义屏幕中间 350 像素宽的区域作为吸附区
        preferredHighlightBegin: (width - 350) / 2
        preferredHighlightEnd: (width + 350) / 2
        // 强制滑动停止时，currentItem 必须停在这个区域内
        highlightRangeMode: ListView.StrictlyEnforceRange

        // 惯性阻尼感（越滑越慢的效果）
        flickDeceleration: 1500

        // 初始化时，让第一个卡片居中显示
        Component.onCompleted: {
            currentIndex = 0
            positionViewAtIndex(0, ListView.Center)
        }

        // 动画：当其他卡片补位时平滑移动
        displaced: Transition {
            NumberAnimation { properties: "x,y"; duration: 250; easing.type: Easing.OutQuad }
        }

        delegate: Rectangle {
            id: cardRoot
            width: 350
            height: 500
            radius: 20
            color: "#2C2C2C"

            // 注意：这里没有任何 ListView.isCurrentItem 的样式变化
            // 所以它在视觉上是完全平等的，但在底层它知道自己有没有对齐中心

            Column {
                anchors.centerIn: parent
                spacing: 20
                Text { text: model.icon; font.pixelSize: 80; anchors.horizontalCenter: parent.horizontalCenter }
                Text { text: model.name; color: "white"; font.pixelSize: 24; font.bold: true }
            }

            // 回弹动画
            NumberAnimation {
                id: returnAnim
                target: cardRoot
                property: "y"
                to: 0
                duration: 200
                easing.type: Easing.OutQuad
            }

            // 飞走动画
            ParallelAnimation {
                id: flyOutAnim
                NumberAnimation { target: cardRoot; property: "y"; to: -800; duration: 250; easing.type: Easing.InBack }
                NumberAnimation { target: cardRoot; property: "opacity"; to: 0; duration: 250 }
                onStopped: {
                    taskViewer.appKilled(model.name)
                    taskList.model.remove(index)
                }
            }

            // 手势处理器：上滑接管，左右滑动还给 ListView
            DragHandler {
                id: verticalDrag
                target: cardRoot
                xAxis.enabled: false // 禁用横向拖拽
                yAxis.enabled: true  // 开启纵向拖拽

                onActiveChanged: {
                    if (active) {
                        returnAnim.stop()
                    } else {
                        if (cardRoot.y < -80) {
                            flyOutAnim.start()
                        } else {
                            returnAnim.start()
                        }
                    }
                }
            }

            TapHandler {
                onTapped: {
                    taskViewer.resumeApp(model.type, model.name, model.icon)
                }
            }
        }
    }

    Button {
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 30
        anchors.horizontalCenter: parent.horizontalCenter
        text: "返回桌面"
        onClicked: taskViewer.closeRequested()
    }
}
