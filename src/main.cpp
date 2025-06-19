// SPDX-FileCopyrightText: Copyright (c) 2025, Daniil Markevich
// SPDX-License-Identifier: BSD-3-Clause

#include <auroraapp.h>
#include <QtQuick>
#include <omp.h>
#include "mazecontroller.h"
#include "cartpolecontroller.h"

int main(int argc, char *argv[])
{
    omp_set_num_threads(1);

    QScopedPointer<QGuiApplication> application(Aurora::Application::application(argc, argv));
    application->setOrganizationName(QStringLiteral("ru.kotdath"));
    application->setApplicationName(QStringLiteral("MLPackAurora"));

    // Create controllers
    MazeController mazeController;
    CartPoleV1Controller cartPoleV1Controller;

    QScopedPointer<QQuickView> view(Aurora::Application::createView());

    // Register controllers with QML
    view->rootContext()->setContextProperty("MazeController", &mazeController);
    view->rootContext()->setContextProperty("CartPoleController", &cartPoleV1Controller);
    
    view->setSource(Aurora::Application::pathTo(QStringLiteral("qml/MLPackAurora.qml")));
    view->show();

    return application->exec();
}
