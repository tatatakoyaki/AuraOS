import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Car.Backend 1.0

Rectangle {
    color: "#1E1E1E"
    radius: 15

    ColumnLayout {
        anchors.centerIn: parent
        spacing: 30

        Text {
            text: "空调控制"
            color: "white"
            font.pixelSize: 28
            Layout.alignment: Qt.AlignHCenter
        }

        // 温度显示
        Text {
            id: tempDisplay
            // 👇 修改 1：向大总管读取当前温度
            text: SystemManager.currentTemp + " °C"
            color: "#01E6DE"
            font.pixelSize: 64
            font.bold: true
            Layout.alignment: Qt.AlignHCenter
        }

        // 按钮组
        RowLayout {
            spacing: 40
            Layout.alignment: Qt.AlignHCenter

            Button {
                text: " - "
                font.pixelSize: 30
                onClicked: {
                    // 👇 修改 2：向大总管申请降温
                    SystemManager.requestHvacChange(-1)
                }
            }

            Button {
                text: " + "
                font.pixelSize: 30
                onClicked: {
                    // 👇 修改 3：向大总管申请升温
                    SystemManager.requestHvacChange(1)
                }
            }
        }
    }
}
