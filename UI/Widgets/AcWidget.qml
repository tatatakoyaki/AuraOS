import QtQuick 2.15
import Car.Backend 1.0

Rectangle {
    color: "transparent"
    Row {
        anchors.centerIn: parent; spacing: 20

        // 减温
        Text {
            text: "-"; color: "white"; font.pixelSize: 30;
            MouseArea {
                anchors.fill: parent;
                // 👇 交给大总管处理
                onClicked: SystemManager.requestHvacChange(-1)
            }
        }

        // 显示温度
        Text {
            // 👇 向大总管读取温度
            text: SystemManager.currentTemp + "°C";
            color: "#01E6DE"; font.pixelSize: 32; font.bold: true
        }

        // 升温
        Text {
            text: "+"; color: "white"; font.pixelSize: 30;
            MouseArea {
                anchors.fill: parent;
                // 👇 交给大总管处理
                onClicked: SystemManager.requestHvacChange(1)
            }
        }
    }
}
