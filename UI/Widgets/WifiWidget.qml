import QtQuick 2.15
Rectangle {
    color: "transparent"
    Column {
        anchors.centerIn: parent; spacing: 5
        Text { text: "📶"; font.pixelSize: 28; anchors.horizontalCenter: parent.horizontalCenter }
        Text { text: "Wi-Fi"; color: "white"; font.pixelSize: 16 }
    }
}
