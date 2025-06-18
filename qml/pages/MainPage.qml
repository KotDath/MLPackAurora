// SPDX-FileCopyrightText: 2025 Open Mobile Platform LLC <community@omp.ru>
// SPDX-License-Identifier: BSD-3-Clause

import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    objectName: "mainPage"
    allowedOrientations: Orientation.All

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height

        Column {
            id: column
            width: parent.width
            spacing: Theme.paddingLarge

            PageHeader {
                objectName: "pageHeader"
                title: qsTr("MLPack Learning")
                extraContent.children: [
                    IconButton {
                        objectName: "aboutButton"
                        icon.source: "image://theme/icon-m-about"
                        anchors.verticalCenter: parent.verticalCenter

                        onClicked: pageStack.push(Qt.resolvedUrl("AboutPage.qml"))
                    }
                ]
            }

            // Main content area with centered buttons
            Item {
                width: parent.width
                height: Theme.itemSizeLarge * 8

                Column {
                    anchors.centerIn: parent
                    spacing: Theme.paddingLarge

                    Button {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: qsTr("Q-Learning Maze")
                        preferredWidth: Theme.buttonWidthLarge
                        onClicked: pageStack.push(Qt.resolvedUrl("MazePage.qml"))
                    }

                    Button {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: qsTr("CartPole MLPack")
                        preferredWidth: Theme.buttonWidthLarge
                        onClicked: pageStack.push(Qt.resolvedUrl("MLPackCartPolePage.qml"))
                    }
                }
            }
        }
    }
}
