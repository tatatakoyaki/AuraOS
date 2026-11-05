import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import Car.Backend 1.0

Item {
    id: root
    signal openAppRequested(string qmlPath)

    property bool isEditMode: false
    property bool isDragging: false
    property int currentDragIndex: -1
    property string dragIcon: ""
    property string dragName: ""

    ListModel { id: localAppModel }

    Component.onCompleted: {
        // 👇 修改 1：向大总管要数据
        let apps = SystemManager.installedApps;
        for (let i = 0; i < apps.length; i++) {
            localAppModel.append(apps[i]);
        }
    }

    Connections {
        // 👇 修改 2：监听大总管的信号
        target: SystemManager
        function onAppsChanged() {
            localAppModel.clear();
            let apps = SystemManager.installedApps;
            for (let i = 0; i < apps.length; i++) {
                localAppModel.append(apps[i]);
            }
        }
    }

    // 统一处理退出编辑模式与后端落库
    function exitEditMode() {
        if (root.isDragging) return;

        root.isEditMode = false;

        // 收集最终顺序并同步给 C++
        let finalOrder = [];
        for (let i = 0; i < localAppModel.count; i++) {
            finalOrder.push(localAppModel.get(i).name);
        }
        // 👇 修改 3：通过大总管同步
        SystemManager.syncAppLayout(finalOrder);
        console.log("【系统】已退出编辑模式，布局已落库");
    }

    // 全屏背景交互区
    MouseArea {
        anchors.fill: parent
        onPressAndHold: {
            if (root.isEditMode && !root.isDragging) {
                root.exitEditMode()
            }
        }
    }

    GridView {
        id: appGrid
        anchors.fill: parent
        anchors.margins: 50
        cellWidth: 120
        cellHeight: 140
        clip: false
        model: localAppModel

        displaced: Transition {
            NumberAnimation { properties: "x,y"; easing.type: Easing.OutQuad; duration: 250 }
        }

        delegate: Item {
            id: delegateItem
            width: appGrid.cellWidth
            height: appGrid.cellHeight

            property int myIndex: index
            property bool isGhost: (root.isDragging && root.currentDragIndex === index)

            Item {
                id: contentContainer
                width: 80
                height: 100
                anchors.centerIn: parent
                opacity: isGhost ? 0 : 1

                // iOS 风格的抖动动画
                SequentialAnimation on rotation {
                    id: wobbleAnim
                    running: root.isEditMode && !isGhost
                    loops: Animation.Infinite
                    NumberAnimation { to: 2; duration: 100; easing.type: Easing.InOutQuad }
                    NumberAnimation { to: -2; duration: 100; easing.type: Easing.InOutQuad }
                }

                Connections {
                    target: root
                    function onIsEditModeChanged() {
                        if (!root.isEditMode) {
                            wobbleAnim.stop()
                            contentContainer.rotation = 0
                        }
                    }
                }

                Rectangle {
                    width: 80; height: 80; radius: 20; color: "#2C2C2C"
                    anchors.top: parent.top
                    Text { anchors.centerIn: parent; text: model.icon; font.pixelSize: 36 }
                }
                Text {
                    anchors.top: parent.bottom; anchors.topMargin: 8
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: model.name; color: "white"; font.pixelSize: 14
                }

                MouseArea {
                    id: mouseArea
                    anchors.fill: parent
                    preventStealing: true

                    onPressAndHold: {
                        if (root.isEditMode && !root.isDragging) {
                            root.exitEditMode()
                        } else if (!root.isEditMode) {
                            root.isEditMode = true
                        }
                    }

                    onClicked: {
                        if (!root.isEditMode) {
                            root.openAppRequested(model.qmlPath)
                        }
                    }

                    onPressed: {
                        root.currentDragIndex = index
                        root.dragIcon = model.icon
                        root.dragName = model.name
                    }

                    onPositionChanged: (mouse) => {
                        if (root.isEditMode && !root.isDragging && (Math.abs(mouse.x - width/2) > 5 || Math.abs(mouse.y - height/2) > 5)) {
                            root.isDragging = true
                            ghostProxy.visible = true
                        }

                        if (root.isDragging) {
                            let globalPos = mapToItem(root, mouse.x, mouse.y)
                            ghostProxy.x = globalPos.x - 40
                            ghostProxy.y = globalPos.y - 40

                            let gridPos = appGrid.mapFromItem(root, ghostProxy.x + 40, ghostProxy.y + 40)
                            let targetIndex = appGrid.indexAt(gridPos.x, gridPos.y)

                            if (targetIndex !== -1 && targetIndex !== root.currentDragIndex) {
                                localAppModel.move(root.currentDragIndex, targetIndex, 1)
                                root.currentDragIndex = targetIndex
                            }
                        }
                    }

                    onReleased: {
                        if (root.isDragging) {
                            root.isDragging = false
                            ghostProxy.visible = false
                            root.currentDragIndex = -1
                        }
                    }

                    onCanceled: {
                        root.isDragging = false
                        ghostProxy.visible = false
                        root.currentDragIndex = -1
                    }
                }
            }
        }
    }

    // 全屏悬浮幽灵图标
    Rectangle {
        id: ghostProxy
        visible: false
        z: 999
        width: 80; height: 100
        color: "transparent"

        Rectangle {
            width: 80; height: 80; radius: 20; color: "#2C2C2C"
            scale: 1.15
            anchors.top: parent.top
            Text { anchors.centerIn: parent; text: root.dragIcon; font.pixelSize: 36 }
        }
        Text {
            anchors.top: parent.bottom; anchors.topMargin: 8
            anchors.horizontalCenter: parent.horizontalCenter
            text: root.dragName; color: "white"; font.pixelSize: 14
        }
    }
}
