// SPDX-FileCopyrightText: Copyright (c) 2025, Daniil Markevich
// SPDX-License-Identifier: BSD-3-Clause

#include <auroraapp.h>
#include <QtQuick>
#include <mlpack.hpp>

using namespace mlpack;
using namespace ens;
using namespace mlpack::rl;
int main(int argc, char *argv[])
{
    // Set up the network.
    FFN<MeanSquaredError, GaussianInitialization> network(MeanSquaredError(),
    GaussianInitialization(0, 0.001));
    network.Add<Linear>(128);
    network.Add<ReLU>();
    network.Add<Linear>(128);
    network.Add<ReLU>();
    network.Add<Linear>(2);

    SimpleDQN<> model(network);

    // Set up the policy and replay method.
    GreedyPolicy<CartPole> policy(1.0, 1000, 0.1, 0.99);
    RandomReplay<CartPole> replayMethod(10, 10000);

    TrainingConfig config;
    config.StepSize() = 0.01;
    config.Discount() = 0.9;
    config.TargetNetworkSyncInterval() = 100;
    config.ExplorationSteps() = 100;
    config.DoubleQLearning() = false;
    config.StepLimit() = 200;

    QLearning<CartPole, decltype(model), AdamUpdate, decltype(policy)>
          agent(config, model, policy, replayMethod);

    arma::running_stat<double> averageReturn;
    size_t episodes = 0;
    bool converged = true;
    while (true)
    {
      double episodeReturn = agent.Episode();
      averageReturn(episodeReturn);
      episodes += 1;

      if (episodes > 1000)
      {
        std::cout << "Cart Pole with DQN failed." << std::endl;
        converged = false;
        break;
      }

      /**
       * Reaching running average return 35 is enough to show it works.
       */
      std::cout << "Average return: " << averageReturn.mean()
          << " Episode return: " << episodeReturn << std::endl;
      if (averageReturn.mean() > 35)
        break;
    }
    if (converged)
      std::cout << "Hooray! Q-Learning agent successfully trained" << std::endl;

    QScopedPointer<QGuiApplication> application(Aurora::Application::application(argc, argv));
    application->setOrganizationName(QStringLiteral("ru.kotdath"));
    application->setApplicationName(QStringLiteral("MLPackAurora"));

    QScopedPointer<QQuickView> view(Aurora::Application::createView());
    view->setSource(Aurora::Application::pathTo(QStringLiteral("qml/MLPackAurora.qml")));
    view->show();

    return application->exec();
}
