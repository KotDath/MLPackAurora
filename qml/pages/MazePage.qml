// SPDX-FileCopyrightText: 2025 Open Mobile Platform LLC <community@omp.ru>
// SPDX-License-Identifier: BSD-3-Clause

import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: mazePage
    objectName: "mazePage"
    allowedOrientations: Orientation.All

    property bool showStatistics: false
    property bool showSpeedControls: false

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height + Theme.paddingLarge

        Column {
            id: column
            width: parent.width
            spacing: Theme.paddingMedium

            PageHeader {
                title: qsTr("Q-Learning Maze")
                description: qsTr("Episode: %1 | Reward: %2%3").arg(MazeController.currentEpisode).arg(Math.round(MazeController.currentReward * 10) / 10).arg(MazeController.fastMode ? " (Fast)" : "")
            }

            // Лабиринт
            Item {
                width: parent.width
                height: mazeDisplay.height + Theme.paddingLarge

                Rectangle {
                    id: mazeBackground
                    anchors.centerIn: parent
                    width: mazeDisplay.width + Theme.paddingMedium
                    height: mazeDisplay.height + Theme.paddingMedium
                    color: Theme.rgba(Theme.highlightBackgroundColor, 0.1)
                    radius: Theme.paddingSmall

                    Text {
                        id: mazeDisplay
                        anchors.centerIn: parent
                        text: MazeController.mazeString
                        font.family: "Courier"
                        font.pixelSize: Theme.fontSizeExtraSmall
                        color: Theme.primaryColor
                        lineHeight: 1.2

                        // Цветовая схема для разных символов
                        Component.onCompleted: {
                            updateMazeColors()
                        }

                        function updateMazeColors() {
                            // Это простое отображение, для более сложной цветовой схемы
                            // можно использовать Rich Text или Canvas
                        }
                    }
                }
            }

            // Статистика
            Row {
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: Theme.paddingLarge

                Column {
                    Label {
                        text: qsTr("Steps: %1").arg(MazeController.stepCount)
                        font.pixelSize: Theme.fontSizeSmall
                        color: Theme.secondaryColor
                    }
                    Label {
                        text: qsTr("Epsilon: %1").arg(Math.round(MazeController.epsilon * 1000) / 1000)
                        font.pixelSize: Theme.fontSizeSmall
                        color: Theme.secondaryColor
                    }
                }
            }

            // Переключатель быстрого режима
            TextSwitch {
                anchors.horizontalCenter: parent.horizontalCenter
                text: qsTr("Fast Mode")
                description: qsTr("Train without step-by-step visualization")
                checked: MazeController.fastMode
                onClicked: MazeController.fastMode = checked
                enabled: !MazeController.isTraining
            }

            // Кнопки управления
            Flow {
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width - Theme.horizontalPageMargin * 2
                spacing: Theme.paddingMedium

                Button {
                    text: MazeController.isTraining ? qsTr("Stop Training") : qsTr("Start Training")
                    onClicked: {
                        if (MazeController.isTraining) {
                            MazeController.stopTraining()
                        } else {
                            MazeController.startTraining(500)  // 500 эпизодов
                        }
                    }
                    enabled: true
                }

                Button {
                    text: qsTr("Fast Train 1000")
                    onClicked: MazeController.fastTrain(1000)
                    enabled: !MazeController.isTraining
                }

                Button {
                    text: qsTr("Fast Train 5000")
                    onClicked: MazeController.fastTrain(5000)
                    enabled: !MazeController.isTraining
                }

                Button {
                    text: qsTr("Step")
                    onClicked: MazeController.stepAgent()
                    enabled: !MazeController.isTraining
                }

                Button {
                    text: qsTr("Run Episode")
                    onClicked: MazeController.runEpisode()
                    enabled: !MazeController.isTraining
                }

                Button {
                    text: qsTr("Reset")
                    onClicked: MazeController.resetMaze()
                    enabled: !MazeController.isTraining
                }

                Button {
                    text: qsTr("New Maze")
                    onClicked: MazeController.generateNewMaze()
                    enabled: !MazeController.isTraining
                }

                Button {
                    text: qsTr("Speed Settings")
                    onClicked: showSpeedControls = !showSpeedControls
                }

                Button {
                    text: qsTr("Statistics")
                    onClicked: showStatistics = !showStatistics
                }
            }

            // Настройки скорости (разворачиваемая секция)
            Column {
                width: parent.width
                visible: showSpeedControls
                spacing: Theme.paddingMedium

                SectionHeader {
                    text: qsTr("Speed Settings")
                }

                Column {
                    width: parent.width - Theme.horizontalPageMargin * 2
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: Theme.paddingMedium

                    Label {
                        text: qsTr("Training Speed (ms delay)")
                        font.pixelSize: Theme.fontSizeSmall
                        color: Theme.secondaryColor
                    }

                    Row {
                        anchors.horizontalCenter: parent.horizontalCenter
                        spacing: Theme.paddingMedium

                        Button {
                            text: qsTr("Very Fast (1ms)")
                            onClicked: MazeController.setTrainingSpeed(1)
                            preferredWidth: Theme.buttonWidthSmall
                        }

                        Button {
                            text: qsTr("Fast (10ms)")
                            onClicked: MazeController.setTrainingSpeed(10)
                            preferredWidth: Theme.buttonWidthSmall
                        }

                        Button {
                            text: qsTr("Normal (100ms)")
                            onClicked: MazeController.setTrainingSpeed(100)
                            preferredWidth: Theme.buttonWidthSmall
                        }

                        Button {
                            text: qsTr("Slow (500ms)")
                            onClicked: MazeController.setTrainingSpeed(500)
                            preferredWidth: Theme.buttonWidthSmall
                        }
                    }

                    Label {
                        text: qsTr("Episodes per UI update")
                        font.pixelSize: Theme.fontSizeSmall
                        color: Theme.secondaryColor
                    }

                    Row {
                        anchors.horizontalCenter: parent.horizontalCenter
                        spacing: Theme.paddingMedium

                        Button {
                            text: "1"
                            onClicked: MazeController.setEpisodesPerUpdate(1)
                            preferredWidth: Theme.buttonWidthSmall
                        }

                        Button {
                            text: "10"
                            onClicked: MazeController.setEpisodesPerUpdate(10)
                            preferredWidth: Theme.buttonWidthSmall
                        }

                        Button {
                            text: "50"
                            onClicked: MazeController.setEpisodesPerUpdate(50)
                            preferredWidth: Theme.buttonWidthSmall
                        }

                        Button {
                            text: "100"
                            onClicked: MazeController.setEpisodesPerUpdate(100)
                            preferredWidth: Theme.buttonWidthSmall
                        }
                    }
                }
            }

            // Секция статистики (разворачиваемая)
            Column {
                width: parent.width
                visible: showStatistics
                spacing: Theme.paddingMedium

                SectionHeader {
                    text: qsTr("Training Statistics")
                }

                // График наград (простая текстовая версия)
                Item {
                    width: parent.width
                    height: rewardChart.height + Theme.paddingMedium

                    Rectangle {
                        anchors.fill: parent
                        color: Theme.rgba(Theme.highlightBackgroundColor, 0.05)
                        radius: Theme.paddingSmall
                    }

                    Column {
                        id: rewardChart
                        anchors.centerIn: parent
                        width: parent.width - Theme.paddingLarge

                        Label {
                            text: qsTr("Recent Rewards (last 10 episodes):")
                            font.pixelSize: Theme.fontSizeSmall
                            color: Theme.secondaryColor
                        }

                        Text {
                            width: parent.width
                            text: {
                                var history = MazeController.rewardHistory
                                var recent = history.slice(Math.max(0, history.length - 10))
                                var result = ""
                                for (var i = 0; i < recent.length; i++) {
                                    result += Math.round(recent[i] * 10) / 10
                                    if (i < recent.length - 1) result += ", "
                                }
                                return result || qsTr("No data yet")
                            }
                            font.pixelSize: Theme.fontSizeExtraSmall
                            color: Theme.primaryColor
                            wrapMode: Text.WordWrap
                        }

                        Label {
                            text: qsTr("Average reward: %1").arg(
                                MazeController.rewardHistory.length > 0 ? 
                                Math.round((MazeController.rewardHistory.reduce(function(a, b) { return a + b }, 0) / MazeController.rewardHistory.length) * 10) / 10 : 
                                0
                            )
                            font.pixelSize: Theme.fontSizeSmall
                            color: Theme.secondaryColor
                        }

                        Label {
                            text: qsTr("Total episodes: %1").arg(MazeController.rewardHistory.length)
                            font.pixelSize: Theme.fontSizeSmall
                            color: Theme.secondaryColor
                        }
                    }
                }

                // Кнопки сохранения/загрузки модели
                Row {
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: Theme.paddingMedium

                    Button {
                        text: qsTr("Save Model")
                        onClicked: {
                            MazeController.saveModel()
                            // Можно добавить уведомление об успешном сохранении
                        }
                        enabled: !MazeController.isTraining
                    }

                    Button {
                        text: qsTr("Load Model")
                        onClicked: {
                            var success = MazeController.loadModel()
                            // Можно добавить уведомление о результате загрузки
                        }
                        enabled: !MazeController.isTraining
                    }
                }
            }
        }
    }

    // Соединения для обновления интерфейса
    Connections {
        target: MazeController
        onGoalReached: {
            // Можно добавить анимацию или звуковой сигнал
            console.log("Goal reached!")
        }
        onTrainingCompleted: {
            console.log("Training completed!")
        }
    }

    // Легенда символов (можно добавить в заголовок или сделать отдельную секцию)
    Component.onCompleted: {
        console.log("Maze Legend:")
        console.log("S/s - Start position")
        console.log("G/g - Goal position") 
        console.log("A - Agent")
        console.log("# - Wall")
        console.log(". - Empty space")
    }
} 