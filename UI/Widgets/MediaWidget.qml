import QtQuick 2.15
import Car.Backend 1.0

Rectangle {
    color: "transparent"
    Column {
        anchors.centerIn: parent; spacing: 15
        Text { text: "🎵"; font.pixelSize: 40; anchors.horizontalCenter: parent.horizontalCenter }

        // 👇 读取大总管的歌名
        Text {
            text: SystemManager.currentSong;
            color: "white"; font.pixelSize: 16;
            elide: Text.ElideRight; width: 150; horizontalAlignment: Text.AlignHCenter
        }

        Row {
            spacing: 20; anchors.horizontalCenter: parent.horizontalCenter

            // 注意：去掉了上一首和下一首，因为底层 MediaHandler 暂时没实现列表功能

            // 播放/暂停按钮
            Text {
                // 👇 读取大总管的播放状态
                text: SystemManager.isPlaying ? "⏸" : "▶";
                color: "white"; font.pixelSize: 24;
                MouseArea {
                    anchors.fill: parent;
                    // 👇 交给大总管处理
                    onClicked: SystemManager.togglePlay()
                }
            }
        }
    }
}
