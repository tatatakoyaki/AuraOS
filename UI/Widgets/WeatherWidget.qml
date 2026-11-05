import QtQuick 2.15
Rectangle {
    color: "transparent"
    Column {
        anchors.centerIn: parent; spacing: 5
        Text { text: "☀️"; font.pixelSize: 60; anchors.horizontalCenter: parent.horizontalCenter }
        Text { text: "26°C 晴"; color: "white"; font.pixelSize: 20; font.bold: true }
    }
}
