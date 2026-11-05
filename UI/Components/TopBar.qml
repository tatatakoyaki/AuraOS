import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

Rectangle {
    height: 45
    color: "#1A1A1A"
    radius: 10

    // 做一个类似车机的顶栏
    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 20
        anchors.rightMargin: 20

        // 左侧：车辆状态
        Row {
            spacing: 15
            Text { text: "🚗"; font.pixelSize: 18; anchors.verticalCenter: parent.verticalCenter }
            Text { text: "P 档"; color: "#01E6DE"; font.pixelSize: 14; font.bold: true; anchors.verticalCenter: parent.verticalCenter }
            Text { text: "🔋 85%"; color: "white"; font.pixelSize: 14; anchors.verticalCenter: parent.verticalCenter }
        }

        Item { Layout.fillWidth: true } // 弹簧，把左右两边撑开

        // 右侧：时间与网络
        Row {
            spacing: 15
            Text { text: "📶"; font.pixelSize: 14; anchors.verticalCenter: parent.verticalCenter }
            Text { id: topTime; text: "12:00"; color: "white"; font.pixelSize: 14; anchors.verticalCenter: parent.verticalCenter }
        }
    }

    // 一个简单的定时器让时间跳动
    Timer {
        interval: 1000; running: true; repeat: true
        onTriggered: {
            let date = new Date()
            topTime.text = Qt.formatDateTime(date, "hh:mm")
        }
    }
}
