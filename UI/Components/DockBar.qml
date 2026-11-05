import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    height: 70
    color: "#1A1A1A"
    radius: 15

    // 信号，告诉主界面该切页面了
    signal pageChanged(string pageName)

    RowLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 30

        Button {
            text: "🏠 主页"
            Layout.fillHeight: true
            onClicked: pageChanged("home")
        }
        Button {
            text: "❄️ 空调"
            Layout.fillHeight: true
            onClicked: pageChanged("ac")
        }
        Button {
            text: "🎵 音乐"
            Layout.fillHeight: true
            onClicked: pageChanged("media")
        }

        Button {
            text: "🗂️" // 多任务图标
            Layout.fillHeight: true
            onClicked: pageChanged("recent") // 发出一个特殊的信号
        }
    }
}
