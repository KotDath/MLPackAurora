#ifndef CARTPOLECONTROLLER_H
#define CARTPOLECONTROLLER_H

#include <QObject>
#include <QTimer>
#include <QThread>
#include <QMutex>
#include <QScopedPointer>
#include <QPointF>
#include <QList>
#include <mlpack.hpp>
#include "CartPoleV1.h"

using namespace mlpack;
using namespace ens;

class CartPoleV1Trainer : public QObject
{
    Q_OBJECT

public:
    explicit CartPoleV1Trainer(QObject *parent = nullptr, int targetAverageReturn = 70);
    void setTargetAverageReturn(int value) { m_targetAverageReturn = value; }
    int targetAverageReturn() const { return m_targetAverageReturn; }
    const FFN<MeanSquaredError, GaussianInitialization>& network() const { return m_network; }

public slots:
    void startTraining();
    void stopTraining();

signals:
    void trainingProgress(int episode, double averageReturn, double episodeReturn, double epsilon);
    void trainingCompleted(bool converged, int totalEpisodes, double finalAverageReturn);
    void trainingError(const QString& error);

private:
    FFN<MeanSquaredError, GaussianInitialization> m_network;
    bool m_shouldStop;
    int m_targetAverageReturn = 70;
};

class CartPoleV1Controller : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isTraining READ isTraining NOTIFY isTrainingChanged)
    Q_PROPERTY(bool isRunning READ isRunning NOTIFY isRunningChanged)
    Q_PROPERTY(int currentEpisode READ currentEpisode NOTIFY currentEpisodeChanged)
    Q_PROPERTY(int currentStep READ currentStep NOTIFY currentStepChanged)
    Q_PROPERTY(double totalReward READ totalReward NOTIFY totalRewardChanged)
    Q_PROPERTY(double cartPosition READ cartPosition NOTIFY cartPositionChanged)
    Q_PROPERTY(double poleAngle READ poleAngle NOTIFY poleAngleChanged)
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)
    Q_PROPERTY(double trainingProgress READ trainingProgress NOTIFY trainingProgressChanged)
    Q_PROPERTY(int targetAverageReturn READ targetAverageReturn WRITE setTargetAverageReturn NOTIFY targetAverageReturnChanged)

public:
    explicit CartPoleV1Controller(QObject *parent = nullptr);
    ~CartPoleV1Controller();

    // Property getters
    bool isTraining() const { return m_isTraining; }
    bool isRunning() const { return m_isRunning; }
    int currentEpisode() const { return m_currentEpisode; }
    int currentStep() const { return m_currentStep; }
    double totalReward() const { return m_totalReward; }
    double cartPosition() const { return m_cartPosition; }
    double poleAngle() const { return m_poleAngle; }
    QString status() const { return m_status; }
    double trainingProgress() const { return m_trainingProgress; }
    int targetAverageReturn() const { return m_targetAverageReturn; }

public slots:
    void startTraining();
    void stopTraining();
    void startSimulation();
    void stopSimulation();
    void nextEpisode();
    void resetSimulation();
    void setTargetAverageReturn(int value);

signals:
    void isTrainingChanged();
    void isRunningChanged();
    void currentEpisodeChanged();
    void currentStepChanged();
    void totalRewardChanged();
    void cartPositionChanged();
    void poleAngleChanged();
    void statusChanged();
    void trainingProgressChanged();
    void targetAverageReturnChanged();

private slots:
    void onTrainingProgress(int episode, double averageReturn, double episodeReturn, double epsilon);
    void onTrainingCompleted(bool converged, int totalEpisodes, double finalAverageReturn);
    void onTrainingError(const QString& error);
    void updateSimulation();

private:
    void setIsTraining(bool value);
    void setIsRunning(bool value);
    void setCurrentEpisode(int value);
    void setCurrentStep(int value);
    void setTotalReward(double value);
    void setCartPosition(double value);
    void setPoleAngle(double value);
    void setStatus(const QString& value);
    void setTrainingProgress(double value);

    void resetSimulationState();
    void takeSimulationStep();

    // Training
    bool m_isTraining;
    QThread* m_trainingThread;
    CartPoleV1Trainer* m_trainer;
    double m_trainingProgress;

    // Simulation
    bool m_isRunning;
    QTimer* m_simulationTimer;
    int m_currentEpisode;
    int m_currentStep;
    double m_totalReward;
    double m_cartPosition;
    double m_poleAngle;
    QString m_status;

    // MLPack objects
    FFN<MeanSquaredError, GaussianInitialization> m_network;
    QScopedPointer<CartPoleV1> m_environment;
    CartPoleV1::State m_currentState;
    bool m_episodeActive;

    // Thread safety
    QMutex m_mutex;

    int m_targetAverageReturn = 70;
};

#endif // CARTPOLECONTROLLER_H 
