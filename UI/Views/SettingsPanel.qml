import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Car.Backend 1.0
import "SettingsPages"

Item {
    id: settingsRoot

    property int currentMenuIndex: 0

    RowLayout {
        anchors.fill: parent
        spacing: 20

        // ==========================================
        // 1. 左侧导航菜单
        // ==========================================
        Rectangle {
            Layout.fillHeight: true
            Layout.preferredWidth: 250
            color: "#1E1E1E"
            radius: 15

            ListModel {
                id: menuModel
                ListElement { name: "显示与亮度"; icon: "☀️"; }
                ListElement { name: "声音"; icon: "🔊"; }
                ListElement { name: "网络与连接"; icon: "📶"; }
                ListElement { name: "关于本机"; icon: "ℹ️"; }
            }

            ListView {
                id: menuList
                anchors.fill: parent
                anchors.margins: 10
                model: menuModel
                spacing: 10

                delegate: Rectangle {
                    width: parent.width
                    height: 60
                    radius: 10
                    color: (index === settingsRoot.currentMenuIndex) ? "#01E6DE" : "transparent"

                    Row {
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left: parent.left
                        anchors.leftMargin: 20
                        spacing: 15
                        Text { text: model.icon; font.pixelSize: 20 }
                        Text {
                            text: model.name;
                            color: (index === settingsRoot.currentMenuIndex) ? "black" : "white";
                            font.pixelSize: 18;
                            font.bold: (index === settingsRoot.currentMenuIndex)
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            // 👇 核心：只改变索引，不销毁、不重建
                            settingsRoot.currentMenuIndex = index
                        }
                    }
                }
            }
        }

        // ==========================================
        // 2. 右侧详情内容区 (使用 StackLayout 性能起飞)
        // ==========================================
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#1E1E1E"
            radius: 15
            clip: true // 防止内部页面溢出边缘

            StackLayout {
                id: rightStack
                anchors.fill: parent
                anchors.margins: 20

                // 👇 直接把当前索引绑定到左侧菜单的索引，瞬间切换
                currentIndex: settingsRoot.currentMenuIndex

                // 注意：这里直接把四个组件实例化并放进来，它们只会加载一次
                DisplayPage {}
                SoundPage {}
                NetworkPage {}
                AboutPage {}
            }
        }
    }
}
