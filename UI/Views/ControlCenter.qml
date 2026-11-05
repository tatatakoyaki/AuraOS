import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Car.Backend 1.0

Rectangle {
    id: controlCenter
    anchors.top: parent.top
    anchors.left: parent.left
    anchors.right: parent.right
    height: parent.height - 70

    color: "#EE121212"
    visible: false
    z: 2000

    property int columns: 10
    property int rows: 4
    property real spacing: 20

    property real safeWidth: width > 0 ? width : 1280
    property real safeHeight: height > 0 ? height : 650
    property real bottomPadding: 120

    property real maxWidthCell: (safeWidth - (columns + 1) * spacing) / columns
    property real maxHeightCell: (safeHeight - bottomPadding - (rows + 1) * spacing) / rows
    property real cellWidth: Math.min(maxWidthCell, maxHeightCell)
    property real cellHeight: cellWidth

    property real offsetX: spacing
    property real offsetY: spacing

    property bool isEditMode: false

    Behavior on y { NumberAnimation { duration: 300; easing.type: Easing.OutCubic } }
    y: visible ? 0 : -height

    ListModel { id: currentWidgetsModel }

    Component.onCompleted: {
        // 👇 修改 1：向大总管要数据
        let data = SystemManager.widgets
        for (let i = 0; i < data.length; i++) {
            let sw = Number(data[i].spanW)
            let sh = Number(data[i].spanH)
            if (sw > 0 && sh > 0) {
                currentWidgetsModel.append({
                    "type": String(data[i].type),
                    "x": Number(data[i].x) || 0,
                    "y": Number(data[i].y) || 0,
                    "spanW": sw,
                    "spanH": sh
                })
            }
        }
    }

    function saveLayoutToBackend() {
        let finalData = []
        for (let i = 0; i < currentWidgetsModel.count; i++) {
            finalData.push(currentWidgetsModel.get(i))
        }
        // 👇 修改 2：让大总管去保存数据
        SystemManager.saveWidgetLayout(finalData)
    }

    function getAddedTypes() {
        let types = []
        for (let i = 0; i < currentWidgetsModel.count; i++) {
            types.push(currentWidgetsModel.get(i).type)
        }
        return types
    }

    function isSlotAvailable(startX, startY, w, h) {
        if (startX + w > controlCenter.columns) return false
        if (startY + h > controlCenter.rows) return false
        for (let i = 0; i < currentWidgetsModel.count; i++) {
            let item = currentWidgetsModel.get(i)
            let xOverlap = (startX < item.x + item.spanW) && (startX + w > item.x)
            let yOverlap = (startY < item.y + item.spanH) && (startY + h > item.y)
            if (xOverlap && yOverlap) return false
        }
        return true
    }

    function findFirstAvailableSlot(w, h) {
        for (let y = 0; y < controlCenter.rows; y++) {
            for (let x = 0; x < controlCenter.columns; x++) {
                if (isSlotAvailable(x, y, w, h)) return { "x": x, "y": y }
            }
        }
        return null
    }

    // 最底层的背景点击区（用于点击真正的空地退出）
    MouseArea {
        anchors.fill: parent
        onPressAndHold: {
            if (!controlCenter.isEditMode) controlCenter.isEditMode = true
        }
        onClicked: {
            if (controlCenter.isEditMode) {
                controlCenter.isEditMode = false
                saveLayoutToBackend()
            } else {
                controlCenter.visible = false
                saveLayoutToBackend()
            }
        }
    }

    Grid {
        x: controlCenter.offsetX
        y: controlCenter.offsetY
        columns: controlCenter.columns
        spacing: controlCenter.spacing
        visible: controlCenter.isEditMode

        Repeater {
            model: controlCenter.columns * controlCenter.rows
            Rectangle {
                width: controlCenter.cellWidth; height: controlCenter.cellHeight
                color: "transparent"; border.color: "#444444"; border.width: 2; radius: 10
            }
        }
    }

    // 渲染卡片
    Repeater {
        model: currentWidgetsModel
        delegate: Item {
            x: controlCenter.offsetX + model.x * (controlCenter.cellWidth + controlCenter.spacing)
            y: controlCenter.offsetY + model.y * (controlCenter.cellHeight + controlCenter.spacing)
            width: model.spanW * controlCenter.cellWidth + (model.spanW - 1) * controlCenter.spacing
            height: model.spanH * controlCenter.cellHeight + (model.spanH - 1) * controlCenter.spacing

            Rectangle {
                anchors.fill: parent
                radius: (model.spanW === 1 && model.spanH === 1) ? width / 2 : 20
                color: "#2C2C2C"
                border.color: controlCenter.isEditMode ? "white" : "transparent"
                border.width: 2
                clip: true

                // 👇 核心修复：卡片专属拦截盾，吃掉落在卡片上的点击事件，防止误触关闭
                MouseArea {
                    anchors.fill: parent
                    // 什么都不用写，只要存在就能阻止事件穿透到底层
                }

                Loader {
                    anchors.fill: parent
                    source: {
                        if (model.type === "wifi") return "qrc:/UI/Widgets/WifiWidget.qml"
                        if (model.type === "media") return "qrc:/UI/Widgets/MediaWidget.qml"
                        if (model.type === "ac") return "qrc:/UI/Widgets/AcWidget.qml"
                        if (model.type === "weather") return "qrc:/UI/Widgets/WeatherWidget.qml"
                        return ""
                    }
                }

                // 删除按钮
                Rectangle {
                    width: 30; height: 30; radius: 15
                    color: "#FF3B30"
                    anchors.top: parent.top; anchors.right: parent.right; anchors.margins: -10
                    visible: controlCenter.isEditMode
                    Text { anchors.centerIn: parent; text: "-"; color: "white"; font.pixelSize: 24; font.bold: true }
                    MouseArea { anchors.fill: parent; onClicked: currentWidgetsModel.remove(index) }
                }
            }
        }
    }

    // 底部添加按钮
    Rectangle {
        width: 200; height: 50
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 30
        anchors.horizontalCenter: parent.horizontalCenter
        radius: 25; color: "#01E6DE"; visible: controlCenter.isEditMode; z: 10
        Text { anchors.centerIn: parent; text: "+ 添加小组件"; color: "black"; font.pixelSize: 18; font.bold: true }
        MouseArea {
            anchors.fill: parent
            onClicked: { widgetSelector.visible = true; }
        }
    }

    // 选择器
    WidgetSelector {
        id: widgetSelector
        visible: false
        z: 3000
        currentAddedTypes: controlCenter.getAddedTypes()

        onWidgetSelected: (type, w, h) => {
            let targetPos = controlCenter.findFirstAvailableSlot(w, h)
            if (targetPos === null) return
            currentWidgetsModel.append({
                "type": type, "x": targetPos.x, "y": targetPos.y, "spanW": w, "spanH": h
            })
        }
        onCloseRequested: widgetSelector.visible = false
    }
}
