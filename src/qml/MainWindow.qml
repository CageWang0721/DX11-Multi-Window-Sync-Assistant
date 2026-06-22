import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: root
    visible: true
    width: 860
    height: 620
    minimumWidth: 720
    minimumHeight: 460
    title: "DX11 多窗口同步器 v2.0"

    // ═══ Win11 Mica + Acrylic Palette ═══
    // Mica 由 C++ 端 DwmSetWindowAttribute 启用，透明区域自动渲染
    // 面板用半透明模拟 Acrylic 叠层效果
    readonly property color micaOverlay:   Qt.rgba(0.95, 0.95, 0.97, 0.78)
    readonly property color surfaceColor:  Qt.rgba(0.98, 0.98, 0.99, 0.72)
    readonly property color surfaceHover:  Qt.rgba(0.94, 0.94, 0.96, 0.85)
    readonly property color surfaceRaised: Qt.rgba(0.99, 0.99, 0.99, 0.88)
    readonly property color borderColor:   Qt.rgba(0.0, 0.0, 0.0, 0.07)
    readonly property color accentColor:   "#005fb8"
    readonly property color accentDim:     Qt.rgba(0.0, 0.37, 0.72, 0.12)
    readonly property color textPrimary:   "#1a1a1a"
    readonly property color textSecondary: "#555555"
    readonly property color textDim:       "#999999"
    readonly property color greenColor:    "#10893e"
    readonly property color greenBg:       Qt.rgba(0.06, 0.55, 0.24, 0.12)
    readonly property color redColor:      "#c42b1c"
    readonly property color redBg:         Qt.rgba(0.76, 0.17, 0.11, 0.08)

    // ═══ Selection State ═══
    property var selectedMap: ({})
    property int selectionVersion: 0

    color: "transparent"

    // ─── Mica 背景叠层 ─────────────────────────────────
    Rectangle {
        anchors.fill: parent
        color: micaOverlay
        z: -1
    }

    // ─── Header Bar (Acrylic 风格) ─────────────────────
    header: Rectangle {
        color: Qt.rgba(0.98, 0.98, 0.99, 0.68)
        height: 48
        border.color: Qt.rgba(0.0, 0.0, 0.0, 0.05)

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 20
            anchors.rightMargin: 16
            spacing: 12

            Text {
                text: "🎮"
                font.pixelSize: 22
            }
            Text {
                text: "DX11 多窗口同步器"
                color: textPrimary
                font.family: "Segoe UI Variable, Segoe UI, Microsoft YaHei UI"
                font.pixelSize: 16
                font.weight: Font.DemiBold
                Layout.fillWidth: true
            }
            Text {
                text: "v2.0"
                color: textDim
                font.pixelSize: 12
            }
        }
    }

    // ═══ Main Content ═══════════════════════════════════
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        // ── Filter Row ─────────────────────────────────
        RowLayout {
            spacing: 10

            TextField {
                id: titleFilter
                placeholderText: "输入关键字过滤窗口..."
                Layout.fillWidth: true
                Layout.preferredHeight: 36
                color: textPrimary
                font.pixelSize: 13

                background: Rectangle {
                    radius: 8
                    color: surfaceColor
                    border.color: titleFilter.activeFocus ? accentColor : borderColor
                    border.width: 1
                }
            }

            TextField {
                id: procFilter
                placeholderText: "进程名过滤 如: game.exe"
                Layout.fillWidth: true
                Layout.preferredHeight: 36
                color: textPrimary
                font.pixelSize: 13

                background: Rectangle {
                    radius: 8
                    color: surfaceColor
                    border.color: procFilter.activeFocus ? accentColor : borderColor
                    border.width: 1
                }
            }

            Button {
                id: btnRefresh
                text: "🔄 刷新列表"
                Layout.preferredWidth: 130
                Layout.preferredHeight: 36

                contentItem: Text {
                    text: btnRefresh.text
                    color: textPrimary
                    font.pixelSize: 13
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                background: Rectangle {
                    radius: 8
                    color: btnRefresh.hovered ? surfaceHover : surfaceColor
                    border.color: borderColor
                    border.width: 1
                }

                onClicked: {
                    selectedMap = {}
                    selectionVersion++
                    syncController.clearChildren()
                    windowModel.refresh(titleFilter.text, procFilter.text)
                }
            }
        }

        // ── Labels + Lists ─────────────────────────────
        RowLayout {
            spacing: 12
            Layout.fillWidth: true
            Layout.fillHeight: true

            // ─── 父窗口列 ──────────────────────────────
            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 6

                Text {
                    text: "👤 父窗口 (单选)"
                    color: textSecondary
                    font.family: "Segoe UI Variable, Segoe UI, Microsoft YaHei UI"
                    font.pixelSize: 13
                    font.weight: Font.DemiBold
                }

                // 列表容器 (Acrylic 卡片)
                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    radius: 10
                    color: surfaceColor
                    border.color: borderColor
                    border.width: 1

                    ListView {
                        id: parentList
                        anchors.fill: parent
                        anchors.margins: 4
                        clip: true
                        model: windowModel
                        currentIndex: -1

                        ScrollBar.vertical: ScrollBar {
                            policy: ScrollBar.AsNeeded
                            contentItem: Rectangle {
                                implicitWidth: 6
                                radius: 3
                                color: Qt.rgba(0,0,0,0.15)
                            }
                        }

                        delegate: Rectangle {
                            id: parentDelegate
                            width: parentList.width
                            height: 36
                            radius: 6
                            color: parentList.currentIndex === index ? accentDim : "transparent"
                            border.color: parentList.currentIndex === index ? accentColor : "transparent"
                            border.width: parentList.currentIndex === index ? 1 : 0

                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 10
                                anchors.rightMargin: 10
                                spacing: 8

                                Text {
                                    text: model.title
                                    color: textPrimary
                                    font.pixelSize: 13
                                    elide: Text.ElideRight
                                    Layout.fillWidth: true
                                }

                                Text {
                                    text: "PID:" + model.processId
                                    color: parentList.currentIndex === index ? "#004c8c" : textDim
                                    font.pixelSize: 11
                                }
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    parentList.currentIndex = index
                                    syncController.setParentHwnd(model.hwnd)
                                }
                            }
                        }
                    }
                }

                // 父窗口信息
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 52
                    radius: 8
                    color: surfaceColor
                    border.color: borderColor

                    Text {
                        anchors.fill: parent
                        anchors.margins: 10
                        text: syncController.parentInfo
                        color: textSecondary
                        font.pixelSize: 12
                        lineHeight: 1.4
                    }
                }
            }

            // ─── 子窗口列 ──────────────────────────────
            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 6

                Text {
                    text: "🎮 子窗口 (点击切换多选)"
                    color: textSecondary
                    font.family: "Segoe UI Variable, Segoe UI, Microsoft YaHei UI"
                    font.pixelSize: 13
                    font.weight: Font.DemiBold
                }

                // 列表容器 (Acrylic 卡片)
                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    radius: 10
                    color: surfaceColor
                    border.color: borderColor
                    border.width: 1

                    ListView {
                        id: childList
                        anchors.fill: parent
                        anchors.margins: 4
                        clip: true
                        model: windowModel

                        ScrollBar.vertical: ScrollBar {
                            policy: ScrollBar.AsNeeded
                            contentItem: Rectangle {
                                implicitWidth: 6
                                radius: 3
                                color: Qt.rgba(0,0,0,0.15)
                            }
                        }

                        delegate: Rectangle {
                            id: childDelegate
                            width: childList.width
                            height: 36
                            radius: 6
                            property bool isSelected: {
                                selectionVersion
                                return selectedMap[model.hwnd] === true
                            }
                            color: isSelected ? accentDim : "transparent"
                            border.color: isSelected ? accentColor : "transparent"
                            border.width: isSelected ? 1 : 0

                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 10
                                anchors.rightMargin: 10
                                spacing: 8

                                Rectangle {
                                    width: 18
                                    height: 18
                                    radius: 4
                                    color: isSelected ? accentColor : "transparent"
                                    border.color: isSelected ? accentColor : Qt.rgba(0,0,0,0.15)
                                    border.width: 1.5

                                    Text {
                                        anchors.centerIn: parent
                                        text: "✓"
                                        color: isSelected ? "#ffffff" : "transparent"
                                        font.pixelSize: 12
                                        font.weight: Font.Bold
                                    }
                                }

                                Text {
                                    text: model.title
                                    color: textPrimary
                                    font.pixelSize: 13
                                    elide: Text.ElideRight
                                    Layout.fillWidth: true
                                }

                                Text {
                                    text: "PID:" + model.processId
                                    color: isSelected ? "#005a9e" : textDim
                                    font.pixelSize: 11
                                }
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    if (selectedMap[model.hwnd]) {
                                        delete selectedMap[model.hwnd]
                                        syncController.removeChildHwnd(model.hwnd)
                                    } else {
                                        selectedMap[model.hwnd] = true
                                        syncController.addChildHwnd(model.hwnd)
                                    }
                                    selectedMap = Object.assign({}, selectedMap)
                                    selectionVersion++
                                }
                            }
                        }
                    }
                }

                // 子窗口信息
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 52
                    radius: 8
                    color: surfaceColor
                    border.color: borderColor

                    Text {
                        anchors.fill: parent
                        anchors.margins: 10
                        text: syncController.childInfo
                        color: textSecondary
                        font.pixelSize: 12
                    }
                }
            }
        }

        // ── Action Buttons ─────────────────────────────
        RowLayout {
            spacing: 10

            Button {
                id: btnStart
                text: "▶ 开始同步"
                Layout.preferredWidth: 140
                Layout.preferredHeight: 40
                enabled: syncController.canStart

                contentItem: Text {
                    text: btnStart.text
                    color: btnStart.enabled ? "#ffffff" : textDim
                    font.pixelSize: 14
                    font.weight: Font.DemiBold
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                background: Rectangle {
                    radius: 8
                    color: btnStart.enabled ? (btnStart.hovered ? "#2e8b57" : "#1a7a3a") : Qt.rgba(0,0,0,0.04)
                    border.color: btnStart.enabled ? "#2e8b57" : Qt.rgba(0,0,0,0.06)
                    border.width: 1
                }

                onClicked: syncController.start()
            }

            Button {
                id: btnStop
                text: "⏹ 终止同步"
                Layout.preferredWidth: 140
                Layout.preferredHeight: 40
                enabled: syncController.canStop

                contentItem: Text {
                    text: btnStop.text
                    color: btnStop.enabled ? "#ffffff" : textDim
                    font.pixelSize: 14
                    font.weight: Font.DemiBold
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                background: Rectangle {
                    radius: 8
                    color: btnStop.enabled ? (btnStop.hovered ? "#c42b1c" : "#a02020") : Qt.rgba(0,0,0,0.04)
                    border.color: btnStop.enabled ? "#c42b1c" : Qt.rgba(0,0,0,0.06)
                    border.width: 1
                }

                onClicked: syncController.stop()
            }

            Text {
                text: "热键: Ctrl+Shift+F12"
                color: textDim
                font.pixelSize: 12
                Layout.leftMargin: 16
            }
        }

        // ── Status Bar ─────────────────────────────────
        Rectangle {
            id: statusBar
            Layout.fillWidth: true
            Layout.preferredHeight: 32
            radius: 8
            color: syncController.running ? greenBg : surfaceColor
            border.color: syncController.running ? Qt.rgba(0.06,0.55,0.24,0.25) : borderColor
            border.width: 1

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 14
                anchors.rightMargin: 14
                spacing: 8

                Rectangle {
                    width: 10
                    height: 10
                    radius: 5
                    color: syncController.running ? "#10893e" : Qt.rgba(0,0,0,0.15)
                }

                Text {
                    text: syncController.statusText
                    color: syncController.running ? "#0b5e0b" : textSecondary
                    font.pixelSize: 12
                    Layout.fillWidth: true
                    elide: Text.ElideRight
                }
            }
        }
    }

    // ═══ Filter live reload ═══════════════════════════
    Timer {
        id: filterDebounce
        interval: 300
        running: false
        repeat: false
        onTriggered: {
            selectedMap = {}
            selectionVersion++
            syncController.clearChildren()
            windowModel.refresh(titleFilter.text, procFilter.text)
        }
    }

    Connections {
        target: titleFilter
        function onTextChanged() { filterDebounce.restart() }
    }
    Connections {
        target: procFilter
        function onTextChanged() { filterDebounce.restart() }
    }
}
