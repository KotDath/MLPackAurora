// SPDX-FileCopyrightText: Copyright (c) 2025, Daniil Markevich
// SPDX-License-Identifier: BSD-3-Clause

import QtQuick 2.0
import Sailfish.Silica 1.0

CoverBackground {
    objectName: "defaultCover"

    CoverTemplate {
        objectName: "applicationCover"
        primaryText: "App"
        secondaryText: qsTr("MLPackAurora")
        icon {
            source: Qt.resolvedUrl("../icons/MLPackAurora.svg")
            sourceSize { width: icon.width; height: icon.height }
        }
    }
}
