#include "cartpolecontroller.h"
#include <QDebug>
#include <QStandardPaths>
#include <QDir>
#include <QCoreApplication>
#include <cmath>

// CartPoleTrainer implementation
CartPoleTrainer::CartPoleTrainer(QObject *parent)
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

void CartPoleTrainer::startTraining()
{
    m_shouldStop = false;
    
    try {
        SimpleDQN<> model(m_network);
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
            QString modelPath = dataDir + "/simple_dqn_cartpole.bin";
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

void CartPoleTrainer::stopTraining()
{
    m_shouldStop = true;
}

// CartPoleController implementation
CartPoleController::CartPoleController(QObject *parent)
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
    , m_environment(new CartPole())
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
    connect(m_simulationTimer, &QTimer::timeout, this, &CartPoleController::updateSimulation);

    // Initialize simulation state
    resetSimulationState();
}

CartPoleController::~CartPoleController()
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

void CartPoleController::startTraining()
{
    if (m_isTraining) return;

    setIsTraining(true);
    setStatus("Starting training...");
    setTrainingProgress(0.0);

    // Create training thread
    m_trainingThread = new QThread(this);
    m_trainer = new CartPoleTrainer();
    m_trainer->moveToThread(m_trainingThread);

    // Connect signals
    connect(m_trainingThread, &QThread::started, m_trainer, &CartPoleTrainer::startTraining);
    connect(m_trainer, &CartPoleTrainer::trainingProgress, this, &CartPoleController::onTrainingProgress);
    connect(m_trainer, &CartPoleTrainer::trainingCompleted, this, &CartPoleController::onTrainingCompleted);
    connect(m_trainer, &CartPoleTrainer::trainingError, this, &CartPoleController::onTrainingError);

    m_trainingThread->start();
}

void CartPoleController::stopTraining()
{
    if (!m_isTraining) return;

    if (m_trainer) {
        QMetaObject::invokeMethod(m_trainer, "stopTraining", Qt::QueuedConnection);
    }
    
    setStatus("Stopping training...");
}

void CartPoleController::startSimulation()
{
    if (m_isRunning) return;

    setIsRunning(true);
    setStatus(m_trainingProgress >= 100.0 ? "Running with trained agent" : "Running with random actions");
    m_episodeActive = true;
    m_simulationTimer->start();
}

void CartPoleController::stopSimulation()
{
    if (!m_isRunning) return;

    setIsRunning(false);
    setStatus("Simulation stopped");
    m_simulationTimer->stop();
}

void CartPoleController::nextEpisode()
{
    if (m_isRunning) return;

    setCurrentEpisode(m_currentEpisode + 1);
    resetSimulationState();
    setStatus("Ready for next episode");
}

void CartPoleController::resetSimulation()
{
    stopSimulation();
    setCurrentEpisode(1);
    resetSimulationState();
    setStatus("Simulation reset");
}

void CartPoleController::onTrainingProgress(int episode, double averageReturn, double episodeReturn, double epsilon)
{
    setTrainingProgress(static_cast<double>(episode) / 2000.0 * 100.0);
    setStatus(QString("Training: Episode %1, Avg Return: %2")
              .arg(episode)
              .arg(averageReturn, 0, 'f', 2));
}

void CartPoleController::onTrainingCompleted(bool converged, int totalEpisodes, double finalAverageReturn)
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

void CartPoleController::onTrainingError(const QString& error)
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

void CartPoleController::updateSimulation()
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

void CartPoleController::resetSimulationState()
{
    m_currentState = m_environment->InitialSample();
    setCurrentStep(0);
    setTotalReward(0.0);
    setCartPosition(m_currentState.Position());
    setPoleAngle(m_currentState.Angle());
    m_episodeActive = true;
}

void CartPoleController::takeSimulationStep()
{
    CartPole::Action action;
    
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
        action.action = static_cast<CartPole::Action::actions>(bestAction);
    } else {
        // Random actions
        action.action = (qrand() % 2 == 0) ? CartPole::Action::backward : CartPole::Action::forward;
    }

    // Take action
    CartPole::State nextState;
    double reward = m_environment->Sample(m_currentState, action, nextState);
    m_currentState = nextState;
    
    setCurrentStep(m_currentStep + 1);
    setTotalReward(m_totalReward + reward);
    setCartPosition(m_currentState.Position());
    setPoleAngle(m_currentState.Angle());
}

// Property setters
void CartPoleController::setIsTraining(bool value)
{
    if (m_isTraining != value) {
        m_isTraining = value;
        emit isTrainingChanged();
    }
}

void CartPoleController::setIsRunning(bool value)
{
    if (m_isRunning != value) {
        m_isRunning = value;
        emit isRunningChanged();
    }
}

void CartPoleController::setCurrentEpisode(int value)
{
    if (m_currentEpisode != value) {
        m_currentEpisode = value;
        emit currentEpisodeChanged();
    }
}

void CartPoleController::setCurrentStep(int value)
{
    if (m_currentStep != value) {
        m_currentStep = value;
        emit currentStepChanged();
    }
}

void CartPoleController::setTotalReward(double value)
{
    if (qAbs(m_totalReward - value) > 0.01) {
        m_totalReward = value;
        emit totalRewardChanged();
    }
}

void CartPoleController::setCartPosition(double value)
{
    if (qAbs(m_cartPosition - value) > 0.001) {
        m_cartPosition = value;
        emit cartPositionChanged();
    }
}

void CartPoleController::setPoleAngle(double value)
{
    if (qAbs(m_poleAngle - value) > 0.001) {
        m_poleAngle = value;
        emit poleAngleChanged();
    }
}

void CartPoleController::setStatus(const QString& value)
{
    if (m_status != value) {
        m_status = value;
        emit statusChanged();
    }
}

void CartPoleController::setTrainingProgress(double value)
{
    if (qAbs(m_trainingProgress - value) > 0.1) {
        m_trainingProgress = value;
        emit trainingProgressChanged();
    }
}
