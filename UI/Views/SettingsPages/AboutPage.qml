import QtQuick 2.15
import QtQuick.Layouts 1.15

ColumnLayout {
    spacing: 20

    Text { text: "关于本机"; color: "white"; font.pixelSize: 28; font.bold: true }

    Rectangle {
        Layout.fillWidth: true
        Layout.preferredHeight: 100
        color: "#2C2C2C"
        radius: 15

        Column {
            anchors.centerIn: parent
            spacing: 10
            Text { text: "智能座舱 IVI 系统"; color: "#01E6DE"; font.pixelSize: 24; font.bold: true }
            Text { text: "版本号: V1.5.0 (Build 2026.11)"; color: "gray"; font.pixelSize: 16 }
        }
    }

    Text { text: "本系统由 C++ 与 QML 强力驱动"; color: "gray"; font.pixelSize: 16 }
}
