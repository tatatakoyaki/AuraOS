import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

ColumnLayout {
    spacing: 30

    Text { text: "显示与亮度"; color: "white"; font.pixelSize: 28; font.bold: true }

    // 模拟深色模式切换
    RowLayout {
        spacing: 20
        Text { text: "深色模式"; color: "white"; font.pixelSize: 20 }
        Switch {
            checked: true
            // 以后可以在这里加真实的主题切换逻辑
        }
    }

    // 模拟亮度调节
    ColumnLayout {
        Text { text: "屏幕亮度"; color: "white"; font.pixelSize: 20 }
        Slider {
            Layout.preferredWidth: 400
            from: 0; to: 100; value: 80
        }
    }
}
