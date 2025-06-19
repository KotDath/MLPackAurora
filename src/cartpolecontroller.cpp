#include "cartpolecontroller.h"
#include <QDebug>
#include <QStandardPaths>
#include <QDir>
#include <QCoreApplication>
#include <cmath>

// CartPoleV1Trainer implementation
CartPoleV1Trainer::CartPoleV1Trainer(QObject *parent)
    : QObject(parent)
    , m_shouldStop(false)
{
    // Initialize network architecture (same as in demo)
    m_network = FFN<MeanSquaredError, GaussianInitialization>(MeanSquaredError(),
                                                              GaussianInitialization(0, 0.01));
    m_network.Add<Linear>(128);
    m_network.Add<ReLU>();
    m_network.Add<Linear>(128);
    m_network.Add<ReLU>();
    m_network.Add<Linear>(2);
}

void CartPoleV1Trainer::startTraining()
{
    m_shouldStop = false;
    
    try {
        SimpleDQN<> model(m_network);
        GreedyPolicy<CartPoleV1> policy(1.0, 1000, 0.1, 0.99);
        RandomReplay<CartPoleV1> replayMethod(10, 10000);

        TrainingConfig config;
        config.StepSize() = 1e-3;
        config.Discount() = 0.99;
        config.TargetNetworkSyncInterval() = 100;
        config.ExplorationSteps() = 300;
        config.DoubleQLearning() = true;
        config.StepLimit() = 200;

        QLearning<CartPoleV1, decltype(model), AdamUpdate, decltype(policy)>
            agent(config, model, policy, replayMethod);

        arma::running_stat<double> averageReturn;
        size_t episodes = 0;
        bool converged = false;
        
        while (!m_shouldStop && episodes < 2000) {
            double episodeReturn = agent.Episode();
            averageReturn(episodeReturn);
            episodes++;

            // Emit progress signal
            emit trainingProgress(episodes, averageReturn.mean(), episodeReturn, policy.Epsilon());

            // Check convergence
            if (averageReturn.mean() > 70 && episodes >= 50) {
                converged = true;
                break;
            }

            // Allow UI updates
            QCoreApplication::processEvents();
        }

        if (converged || episodes >= 2000) {
            // Copy trained parameters to our network
            m_network.Parameters() = agent.Network().Parameters();
            
            // Save the trained network
            QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
            QDir().mkpath(dataDir);
            QString modelPath = dataDir + "/simple_dqn_CartPoleV1.bin";
            qDebug() << "model path: " << modelPath;
            
            try {
                //mlpack::data::Save(modelPath.toStdString(), "agent", m_network, false);
                emit trainingCompleted(converged, episodes, averageReturn.mean());
            } catch (const std::exception& e) {
                emit trainingError(QString("Failed to save model: %1").arg(e.what()));
            }
        } else {
            emit trainingError("Training was stopped");
        }
        
    } catch (const std::exception& e) {
        emit trainingError(QString("Training error: %1").arg(e.what()));
    }
}

void CartPoleV1Trainer::stopTraining()
{
    m_shouldStop = true;
}

// CartPoleV1Controller implementation
CartPoleV1Controller::CartPoleV1Controller(QObject *parent)
    : QObject(parent)
    , m_isTraining(false)
    , m_trainingThread(nullptr)
    , m_trainer(nullptr)
    , m_trainingProgress(0.0)
    , m_isRunning(false)
    , m_currentEpisode(1)
    , m_currentStep(0)
    , m_totalReward(0.0)
    , m_cartPosition(0.0)
    , m_poleAngle(0.0)
    , m_status("Ready")
    , m_environment(new CartPoleV1())
    , m_episodeActive(false)
{
    // Initialize network
    m_network = FFN<MeanSquaredError, GaussianInitialization>(MeanSquaredError(),
                                                              GaussianInitialization(0, 0.01));
    m_network.Add<Linear>(128);
    m_network.Add<ReLU>();
    m_network.Add<Linear>(128);
    m_network.Add<ReLU>();
    m_network.Add<Linear>(2);
    // Setup simulation timer
    m_simulationTimer = new QTimer(this);
    m_simulationTimer->setInterval(50); // 20 FPS
    connect(m_simulationTimer, &QTimer::timeout, this, &CartPoleV1Controller::updateSimulation);

    // Initialize simulation state
    resetSimulationState();
}

CartPoleV1Controller::~CartPoleV1Controller()
{
    if (m_trainingThread) {
        if (m_trainer) {
            QMetaObject::invokeMethod(m_trainer, "stopTraining", Qt::QueuedConnection);
        }
        m_trainingThread->quit();
        m_trainingThread->wait(5000);
        delete m_trainingThread;
    }
}

void CartPoleV1Controller::startTraining()
{
    if (m_isTraining) return;

    setIsTraining(true);
    setStatus("Starting training...");
    setTrainingProgress(0.0);

    // Create training thread
    m_trainingThread = new QThread(this);
    m_trainer = new CartPoleV1Trainer();
    m_trainer->moveToThread(m_trainingThread);

    // Connect signals
    connect(m_trainingThread, &QThread::started, m_trainer, &CartPoleV1Trainer::startTraining);
    connect(m_trainer, &CartPoleV1Trainer::trainingProgress, this, &CartPoleV1Controller::onTrainingProgress);
    connect(m_trainer, &CartPoleV1Trainer::trainingCompleted, this, &CartPoleV1Controller::onTrainingCompleted);
    connect(m_trainer, &CartPoleV1Trainer::trainingError, this, &CartPoleV1Controller::onTrainingError);

    m_trainingThread->start();
}

void CartPoleV1Controller::stopTraining()
{
    if (!m_isTraining) return;

    if (m_trainer) {
        QMetaObject::invokeMethod(m_trainer, "stopTraining", Qt::QueuedConnection);
    }
    
    setStatus("Stopping training...");
}

void CartPoleV1Controller::startSimulation()
{
    if (m_isRunning) return;

    setIsRunning(true);
    setStatus(m_trainingProgress >= 100.0 ? "Running with trained agent" : "Running with random actions");
    m_episodeActive = true;
    m_simulationTimer->start();
}

void CartPoleV1Controller::stopSimulation()
{
    if (!m_isRunning) return;

    setIsRunning(false);
    setStatus("Simulation stopped");
    m_simulationTimer->stop();
}

void CartPoleV1Controller::nextEpisode()
{
    if (m_isRunning) return;

    setCurrentEpisode(m_currentEpisode + 1);
    resetSimulationState();
    setStatus("Ready for next episode");
}

void CartPoleV1Controller::resetSimulation()
{
    stopSimulation();
    setCurrentEpisode(1);
    resetSimulationState();
    setStatus("Simulation reset");
}

void CartPoleV1Controller::onTrainingProgress(int episode, double averageReturn, double episodeReturn, double epsilon)
{
    setTrainingProgress(static_cast<double>(episode) / 2000.0 * 100.0);
    setStatus(QString("Training: Episode %1, Avg Return: %2")
              .arg(episode)
              .arg(averageReturn, 0, 'f', 2));
}

void CartPoleV1Controller::onTrainingCompleted(bool converged, int totalEpisodes, double finalAverageReturn)
{
    setIsTraining(false);
    setTrainingProgress(100.0);

    // Копируем параметры обученной сети из тренера в контроллер
    if (m_trainer) {
        m_network.Parameters() = m_trainer->network().Parameters();
    }

    if (converged) {
        setStatus(QString("Training completed! Episodes: %1, Final return: %2")
                  .arg(totalEpisodes)
                  .arg(finalAverageReturn, 0, 'f', 2));
    } else {
        setStatus(QString("Training stopped. Episodes: %1, Best return: %2")
                  .arg(totalEpisodes)
                  .arg(finalAverageReturn, 0, 'f', 2));
    }

    // Clean up training thread
    if (m_trainingThread) {
        m_trainingThread->quit();
        m_trainingThread->wait();
        m_trainer->deleteLater();
        m_trainingThread->deleteLater();
        m_trainingThread = nullptr;
        m_trainer = nullptr;
    }
}

void CartPoleV1Controller::onTrainingError(const QString& error)
{
    setIsTraining(false);
    setStatus(QString("Training error: %1").arg(error));
    
    // Clean up training thread
    if (m_trainingThread) {
        m_trainingThread->quit();
        m_trainingThread->wait();
        m_trainer->deleteLater();
        m_trainingThread->deleteLater();
        m_trainingThread = nullptr;
        m_trainer = nullptr;
    }
}

void CartPoleV1Controller::updateSimulation()
{
    if (!m_isRunning || !m_episodeActive) return;

    // Check if episode should end
    if (m_environment->IsTerminal(m_currentState) || m_currentStep >= 500) {
        m_episodeActive = false;
        setStatus(QString("Episode %1 ended. Steps: %2, Reward: %3")
                  .arg(m_currentEpisode)
                  .arg(m_currentStep)
                  .arg(m_totalReward, 0, 'f', 1));
        return;
    }

    takeSimulationStep();
}

void CartPoleV1Controller::resetSimulationState()
{
    m_currentState = m_environment->InitialSample();
    setCurrentStep(0);
    setTotalReward(0.0);
    setCartPosition(m_currentState.Position());
    setPoleAngle(m_currentState.Angle());
    m_episodeActive = true;
}

void CartPoleV1Controller::takeSimulationStep()
{
    CartPoleV1::Action action;
    
    if (m_trainingProgress >= 100.0) {
        // Use trained network
        arma::mat stateVec(4, 1);
        stateVec(0, 0) = m_currentState.Position();
        stateVec(1, 0) = m_currentState.Velocity();
        stateVec(2, 0) = m_currentState.Angle();
        stateVec(3, 0) = m_currentState.AngularVelocity();
        
        arma::mat qValues;
        m_network.Predict(stateVec, qValues);
        
        // Choose action with highest Q-value
        arma::uword bestAction;
        qValues.max(bestAction);
        action.action = static_cast<CartPoleV1::Action::actions>(bestAction);
    } else {
        // Random actions
        action.action = (qrand() % 2 == 0) ? CartPoleV1::Action::backward : CartPoleV1::Action::forward;
    }

    // Take action
    CartPoleV1::State nextState;
    double reward = m_environment->Sample(m_currentState, action, nextState);
    m_currentState = nextState;
    
    setCurrentStep(m_currentStep + 1);
    setTotalReward(m_totalReward + reward);
    setCartPosition(m_currentState.Position());
    setPoleAngle(m_currentState.Angle());
}

// Property setters
void CartPoleV1Controller::setIsTraining(bool value)
{
    if (m_isTraining != value) {
        m_isTraining = value;
        emit isTrainingChanged();
    }
}

void CartPoleV1Controller::setIsRunning(bool value)
{
    if (m_isRunning != value) {
        m_isRunning = value;
        emit isRunningChanged();
    }
}

void CartPoleV1Controller::setCurrentEpisode(int value)
{
    if (m_currentEpisode != value) {
        m_currentEpisode = value;
        emit currentEpisodeChanged();
    }
}

void CartPoleV1Controller::setCurrentStep(int value)
{
    if (m_currentStep != value) {
        m_currentStep = value;
        emit currentStepChanged();
    }
}

void CartPoleV1Controller::setTotalReward(double value)
{
    if (qAbs(m_totalReward - value) > 0.01) {
        m_totalReward = value;
        emit totalRewardChanged();
    }
}

void CartPoleV1Controller::setCartPosition(double value)
{
    if (qAbs(m_cartPosition - value) > 0.001) {
        m_cartPosition = value;
        emit cartPositionChanged();
    }
}

void CartPoleV1Controller::setPoleAngle(double value)
{
    if (qAbs(m_poleAngle - value) > 0.001) {
        m_poleAngle = value;
        emit poleAngleChanged();
    }
}

void CartPoleV1Controller::setStatus(const QString& value)
{
    if (m_status != value) {
        m_status = value;
        emit statusChanged();
    }
}

void CartPoleV1Controller::setTrainingProgress(double value)
{
    if (qAbs(m_trainingProgress - value) > 0.1) {
        m_trainingProgress = value;
        emit trainingProgressChanged();
    }
}
