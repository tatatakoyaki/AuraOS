import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

Rectangle {
    color: "#1E1E1E"
    ColumnLayout {
        anchors.fill: parent
        spacing: 20
        Text { text: "网络与连接"; color: "white"; font.pixelSize: 28; font.bold: true }
        RowLayout {
            spacing: 15
            Text { text: "📶 Wi-Fi"; color: "#01E6DE"; font.pixelSize: 20 }
            Switch { checked: true }
        }
        Text { text: "已连接到: SmartIVI_5G"; color: "gray"; font.pixelSize: 16 }
    }
}
