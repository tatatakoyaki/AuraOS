import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: selector
    anchors.fill: parent
    color: "#CC000000"
    visible: false
    z: 3000

    property var currentAddedTypes: []

    signal widgetSelected(string type, int spanW, int spanH)
    signal closeRequested()

    MouseArea {
        anchors.fill: parent
        onClicked: selector.closeRequested()
    }

    Rectangle {
        width: 800
        height: 500
        anchors.centerIn: parent
        color: "#1A1A1A"
        radius: 24
        border.color: "#01E6DE"
        border.width: 2

        MouseArea { anchors.fill: parent; onClicked: {} }

        Text {
            anchors.top: parent.top; anchors.topMargin: 30
            anchors.horizontalCenter: parent.horizontalCenter
            text: "添加小组件"; color: "white"; font.pixelSize: 28; font.bold: true
        }

        Button {
            text: "✕"; anchors.top: parent.top; anchors.right: parent.right; anchors.margins: 20
            background: Rectangle { color: "transparent" }
            contentItem: Text { text: parent.text; color: "gray"; font.pixelSize: 24 }
            onClicked: selector.closeRequested()
        }

        GridView {
            anchors.fill: parent
            anchors.topMargin: 100; anchors.leftMargin: 40; anchors.rightMargin: 40; anchors.bottomMargin: 40
            cellWidth: 180; cellHeight: 180
            clip: true

            model: [
                { "type": "wifi", "name": "Wi-Fi", "icon": "📶", "w": 1, "h": 1 },
                { "type": "weather", "name": "天气", "icon": "☀️", "w": 2, "h": 2 },
                { "type": "media", "name": "音乐", "icon": "🎵", "w": 2, "h": 2 },
                { "type": "ac", "name": "空调", "icon": "❄️", "w": 2, "h": 1 }
            ]

            delegate: Item {
                width: 160; height: 160
                property bool isAdded: selector.currentAddedTypes.indexOf(modelData.type) !== -1

                Rectangle {
                    anchors.fill: parent
                    radius: 20
                    color: isAdded ? "#111111" : "#2C2C2C"
                    border.color: isAdded ? "#222222" : "#444"
                    opacity: isAdded ? 0.5 : 1.0

                    MouseArea {
                        anchors.fill: parent
                        enabled: !isAdded
                        onClicked: {
                            selector.widgetSelected(modelData.type, modelData.w, modelData.h)
                            selector.closeRequested()
                        }
                    }

                    Column {
                        anchors.centerIn: parent; spacing: 10
                        Text { text: modelData.icon; font.pixelSize: 60; anchors.horizontalCenter: parent.horizontalCenter }
                        Text { text: isAdded ? "已添加" : modelData.name; color: isAdded ? "gray" : "white"; font.pixelSize: 20; font.bold: true }
                    }
                }
            }
        }
    }
}
