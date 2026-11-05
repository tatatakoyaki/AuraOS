import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Car.Backend 1.0

Rectangle {
    color: "#1E1E1E"
    radius: 15

    ColumnLayout {
        anchors.centerIn: parent
        spacing: 15
        width: parent.width * 0.9

        // 1. 搜索栏
        RowLayout {
            Layout.fillWidth: true
            spacing: 10
            TextField {
                id: searchInput
                Layout.fillWidth: true
                text: "两只老虎"
                color: "white"
                background: Rectangle { color: "#2C2C2C"; radius: 8 }
            }
            Button {
                text: "播放"
                // 👇 修改 1：请求大总管搜索并播放
                onClicked: SystemManager.requestPlayMusic(searchInput.text)
            }
        }

        // 2. 真正的专辑封面
        Rectangle {
            Layout.alignment: Qt.AlignHCenter
            width: 120; height: 120
            radius: 10
            color: "#2C2C2C"
            clip: true

            Image {
                anchors.fill: parent
                // 👇 修改 2：向大总管要封面
                source: SystemManager.coverUrl
                fillMode: Image.PreserveAspectCrop
                asynchronous: true
            }
            Text {
                anchors.centerIn: parent
                text: "🎵"
                font.pixelSize: 40
                // 👇 修改 3：向大总管要封面数据
                visible: SystemManager.coverUrl === ""
            }
        }

        // 3. 歌名
        Text {
            // 👇 修改 4：向大总管要歌名
            text: SystemManager.currentSong
            color: "white"
            font.pixelSize: 16
            font.bold: true
            Layout.alignment: Qt.AlignHCenter
        }

        // 4. 歌词显示区
        ListView {
            id: lyricView
            Layout.fillWidth: true
            Layout.preferredHeight: 180

            // 👇 修改 5：向大总管要歌词列表
            model: SystemManager.lyricLines

            // 👇 修改 6：向大总管要当前歌词行号
            currentIndex: SystemManager.currentLyricIndex

            ScrollBar.vertical: ScrollBar { policy: ScrollBar.AlwaysOff }
            flickDeceleration: 1500
            boundsBehavior: Flickable.StopAtBounds
            highlightMoveDuration: 300
            highlightRangeMode: ListView.StrictlyEnforceRange
            preferredHighlightBegin: height / 2 - 30
            preferredHighlightEnd: height / 2 + 30

            delegate: Item {
                width: lyricView.width
                height: 40

                Text {
                    text: modelData
                    anchors.centerIn: parent

                    property int distance: Math.abs(index - lyricView.currentIndex)

                    color: distance === 0 ? "#01E6DE" : "#8e8e8e"
                    font.pixelSize: distance === 0 ? 22 : 14 - (distance * 2)
                    opacity: distance === 0 ? 1.0 : Math.max(0.1, 0.6 - (distance * 0.15))
                    font.bold: distance === 0

                    Behavior on font.pixelSize { NumberAnimation { duration: 300 } }
                    Behavior on opacity { NumberAnimation { duration: 300 } }
                    Behavior on color { ColorAnimation { duration: 300 } }
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        // 可选功能
                    }
                }
            }

            property bool userFlicking: false
            onFlickStarted: userFlicking = true
            onFlickEnded: {
                restoreTimer.restart()
            }

            Timer {
                id: restoreTimer
                interval: 2000
                onTriggered: {
                    lyricView.userFlicking = false
                    lyricView.positionViewAtIndex(lyricView.currentIndex, ListView.Center)
                }
            }

            // 👇 修改 7：监听大总管的歌词变化信号
            Connections {
                target: SystemManager
                function onCurrentLyricChanged() {
                    if (!lyricView.userFlicking) {
                        lyricView.positionViewAtIndex(SystemManager.currentLyricIndex, ListView.Center)
                    }
                }
            }
        }

        // 5. 播放控制与音量
        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: 20
            Button {
                // 👇 修改 8：通过大总管控制
                text: SystemManager.isPlaying ? "⏸" : "▶"
                onClicked: SystemManager.togglePlay()
            }
            Button { text: "🔉"; onClicked: SystemManager.changeVolume(-10) }
            // 👇 修改 9：通过大总管读音量
            Text { text: SystemManager.volume; color: "white" }
            Button { text: "🔊"; onClicked: SystemManager.changeVolume(10) }
        }
    }
}
