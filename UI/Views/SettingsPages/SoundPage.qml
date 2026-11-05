import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

Rectangle {
    color: "#1E1E1E" // 和背景同色
    ColumnLayout {
        anchors.fill: parent
        spacing: 30
        Text { text: "声音设置"; color: "white"; font.pixelSize: 28; font.bold: true }

        RowLayout {
            Text { text: "媒体音量"; color: "white"; font.pixelSize: 20 }
            Slider { Layout.preferredWidth: 300; value: 50 }
        }
        Text { text: "测试页面，说明切换成功！"; color: "#01E6DE"; font.pixelSize: 20 }
    }
}
