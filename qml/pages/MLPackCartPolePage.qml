// SPDX-FileCopyrightText: Copyright (c) 2025, Daniil Markevich
// SPDX-License-Identifier: BSD-3-Clause

import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page
    objectName: "cartPolePage"
    allowedOrientations: Orientation.All

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height

        Column {
            id: column
            width: parent.width
            spacing: Theme.paddingMedium

            PageHeader {
                title: qsTr("CartPole MLPack")
            }

            // Status section
            Item {
                width: parent.width
                height: statusColumn.height + Theme.paddingLarge * 2

                Rectangle {
                    anchors.fill: parent
                    color: Theme.rgba(Theme.highlightBackgroundColor, 0.1)
                    radius: Theme.paddingSmall
                }

                Column {
                    id: statusColumn
                    anchors.centerIn: parent
                    width: parent.width - Theme.paddingLarge * 2
                    spacing: Theme.paddingSmall

                    Label {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: CartPoleController.status
                        font.pixelSize: Theme.fontSizeSmall
                        color: Theme.highlightColor
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                    }

                    // Training progress bar
                    Item {
                        width: parent.width
                        height: Theme.itemSizeExtraSmall
                        visible: CartPoleController.isTraining

                        Rectangle {
                            anchors.fill: parent
                            color: Theme.rgba(Theme.primaryColor, 0.2)
                            radius: height / 2
                        }

                        Rectangle {
                            width: parent.width * (CartPoleController.trainingProgress / 100.0)
                            height: parent.height
                            color: Theme.highlightColor
                            radius: height / 2
                        }

                        Label {
                            anchors.centerIn: parent
                            text: Math.round(CartPoleController.trainingProgress) + "%"
                            font.pixelSize: Theme.fontSizeExtraSmall
                            color: Theme.primaryColor
                        }
                    }
                }
            }

            // CartPole visualization
            Item {
                id: visualizationArea
                width: parent.width
                height: Theme.itemSizeLarge * 4

                Rectangle {
                    anchors.fill: parent
                    color: Theme.rgba(Theme.secondaryColor, 0.1)
                    radius: Theme.paddingSmall
                }

                // CartPole visualization component
                Item {
                    id: cartPoleViz
                    anchors.fill: parent
                    anchors.margins: Theme.paddingMedium

                    property real cartPosition: CartPoleController.cartPosition
                    property real poleAngle: CartPoleController.poleAngle
                    property bool isRunning: CartPoleController.isRunning
                    property bool isModelLoaded: CartPoleController.isModelLoaded

                    readonly property real worldWidth: 4.8
                    readonly property real cartWidth: 40
                    readonly property real cartHeight: 20
                    readonly property real poleLength: 80
                    readonly property real groundY: height - 40

                    Rectangle {
                        id: background
                        anchors.fill: parent
                        gradient: Gradient {
                            GradientStop { position: 0.0; color: Theme.rgba(Theme.highlightBackgroundColor, 0.3) }
                            GradientStop { position: 1.0; color: Theme.rgba(Theme.highlightBackgroundColor, 0.1) }
                        }
                        radius: Theme.paddingSmall
                    }

                    // Ground
                    Rectangle {
                        anchors.bottom: parent.bottom
                        anchors.left: parent.left
                        anchors.right: parent.right
                        height: 40
                        color: Theme.rgba(Theme.secondaryColor, 0.3)

                        Rectangle {
                            anchors.top: parent.top
                            anchors.left: parent.left
                            anchors.right: parent.right
                            height: 4
                            color: Theme.secondaryColor
                        }
                    }

                    // Track
                    Rectangle {
                        id: track
                        anchors.bottom: parent.bottom
                        anchors.bottomMargin: 35
                        anchors.left: parent.left
                        anchors.leftMargin: 20
                        anchors.right: parent.right
                        anchors.rightMargin: 20
                        height: 4
                        color: Theme.primaryColor
                        radius: 2
                    }

                    // Cart
                    Item {
                        id: cart
                        x: (cartPoleViz.cartPosition + cartPoleViz.worldWidth/2) / cartPoleViz.worldWidth * (track.width - cartPoleViz.cartWidth) + track.x
                        y: cartPoleViz.groundY - cartPoleViz.cartHeight
                        width: cartPoleViz.cartWidth
                        height: cartPoleViz.cartHeight

                        Rectangle {
                            anchors.fill: parent
                            color: cartPoleViz.isModelLoaded ? Theme.highlightColor : Theme.errorColor
                            radius: 4
                            border.color: Theme.primaryColor
                            border.width: 2
                        }

                        // Wheels
                        Rectangle {
                            x: 8
                            y: parent.height - 4
                            width: 12
                            height: 12
                            radius: 6
                            color: Theme.primaryColor
                        }

                        Rectangle {
                            x: parent.width - 20
                            y: parent.height - 4
                            width: 12
                            height: 12
                            radius: 6
                            color: Theme.primaryColor
                        }

                        // Pole pivot point
                        Rectangle {
                            id: pivot
                            anchors.horizontalCenter: parent.horizontalCenter
                            anchors.top: parent.top
                            width: 8
                            height: 8
                            radius: 4
                            color: Theme.secondaryColor
                        }
                    }

                    // Pole
                    Rectangle {
                        id: pole
                        width: 6
                        height: cartPoleViz.poleLength
                        x: cart.x + cart.width/2 - width/2
                        y: cart.y - height
                        color: Theme.rgba(Theme.highlightColor, 0.8)
                        radius: 3
                        transformOrigin: Item.Bottom
                        rotation: cartPoleViz.poleAngle * 180 / Math.PI

                        // Pole tip
                        Rectangle {
                            anchors.horizontalCenter: parent.horizontalCenter
                            anchors.top: parent.top
                            width: 16
                            height: 16
                            radius: 8
                            color: cartPoleViz.isRunning ? Theme.highlightColor : Theme.secondaryColor
                        }
                    }

                    // Status indicator
                    Rectangle {
                        anchors.top: parent.top
                        anchors.right: parent.right
                        anchors.margins: Theme.paddingSmall
                        width: 20
                        height: 20
                        radius: 10
                        color: cartPoleViz.isRunning ? Theme.highlightColor : Theme.secondaryColor

                        SequentialAnimation on opacity {
                            running: cartPoleViz.isRunning
                            loops: Animation.Infinite
                            NumberAnimation { to: 0.3; duration: 500 }
                            NumberAnimation { to: 1.0; duration: 500 }
                        }
                    }
                }
            }

            // Information panel
            Item {
                width: parent.width
                height: infoGrid.height + Theme.paddingLarge

                Rectangle {
                    anchors.fill: parent
                    color: Theme.rgba(Theme.highlightBackgroundColor, 0.05)
                    radius: Theme.paddingSmall
                }

                Grid {
                    id: infoGrid
                    anchors.centerIn: parent
                    columns: 2
                    spacing: Theme.paddingMedium
                    width: parent.width - Theme.paddingLarge * 2

                    Label {
                        text: qsTr("Episode:")
                        font.pixelSize: Theme.fontSizeSmall
                        color: Theme.secondaryColor
                    }
                    Label {
                        text: CartPoleController.currentEpisode
                        font.pixelSize: Theme.fontSizeSmall
                        color: Theme.primaryColor
                    }

                    Label {
                        text: qsTr("Step:")
                        font.pixelSize: Theme.fontSizeSmall
                        color: Theme.secondaryColor
                    }
                    Label {
                        text: CartPoleController.currentStep
                        font.pixelSize: Theme.fontSizeSmall
                        color: Theme.primaryColor
                    }

                    Label {
                        text: qsTr("Total Reward:")
                        font.pixelSize: Theme.fontSizeSmall
                        color: Theme.secondaryColor
                    }
                    Label {
                        text: CartPoleController.totalReward.toFixed(1)
                        font.pixelSize: Theme.fontSizeSmall
                        color: Theme.primaryColor
                    }

                    Label {
                        text: qsTr("Cart Position:")
                        font.pixelSize: Theme.fontSizeSmall
                        color: Theme.secondaryColor
                    }
                    Label {
                        text: CartPoleController.cartPosition.toFixed(3)
                        font.pixelSize: Theme.fontSizeSmall
                        color: Theme.primaryColor
                    }

                    Label {
                        text: qsTr("Pole Angle:")
                        font.pixelSize: Theme.fontSizeSmall
                        color: Theme.secondaryColor
                    }
                    Label {
                        text: (CartPoleController.poleAngle * 180 / Math.PI).toFixed(1) + "°"
                        font.pixelSize: Theme.fontSizeSmall
                        color: Theme.primaryColor
                    }
                }
            }

            // Control buttons
            Column {
                width: parent.width
                spacing: Theme.paddingMedium

                // Training controls
                SectionHeader {
                    text: qsTr("Training")
                }

                Row {
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: Theme.paddingMedium

                    Button {
                        text: qsTr("Start Training")
                        enabled: !CartPoleController.isTraining
                        onClicked: CartPoleController.startTraining()
                        preferredWidth: Theme.buttonWidthMedium
                    }

                    Button {
                        text: qsTr("Stop Training")
                        enabled: CartPoleController.isTraining
                        onClicked: CartPoleController.stopTraining()
                        preferredWidth: Theme.buttonWidthMedium
                    }
                }

                // Simulation controls
                SectionHeader {
                    text: qsTr("Simulation")
                }

                Row {
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: Theme.paddingMedium

                    Button {
                        text: CartPoleController.isRunning ? qsTr("Stop") : qsTr("Start")
                        enabled: !CartPoleController.isTraining
                        onClicked: {
                            if (CartPoleController.isRunning) {
                                CartPoleController.stopSimulation()
                            } else {
                                CartPoleController.startSimulation()
                            }
                        }
                        preferredWidth: Theme.buttonWidthMedium
                        color: CartPoleController.isRunning ? Theme.errorColor : Theme.highlightColor
                    }

                    Button {
                        text: qsTr("Next Episode")
                        enabled: !CartPoleController.isRunning && !CartPoleController.isTraining
                        onClicked: CartPoleController.nextEpisode()
                        preferredWidth: Theme.buttonWidthMedium
                    }
                }

                Row {
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: Theme.paddingMedium

                    Button {
                        text: qsTr("Reset")
                        enabled: !CartPoleController.isTraining
                        onClicked: CartPoleController.resetSimulation()
                        preferredWidth: Theme.buttonWidthMedium
                    }
                }
            }

            // Add some bottom spacing
            Item {
                width: parent.width
                height: Theme.paddingLarge
            }
        }
    }
} 
