#ifndef MAZECONTROLLER_H
#define MAZECONTROLLER_H

#include <QObject>
#include <QTimer>
#include <QVariantList>
#include "mazeenvironment.h"
#include "qlearningagent.h"

class MazeController : public QObject {
    Q_OBJECT
    
    Q_PROPERTY(QString mazeString READ getMazeString NOTIFY mazeChanged)
    Q_PROPERTY(bool isTraining READ getIsTraining NOTIFY trainingStateChanged)
    Q_PROPERTY(int currentEpisode READ getCurrentEpisode NOTIFY episodeChanged)
    Q_PROPERTY(double currentReward READ getCurrentReward NOTIFY rewardChanged)
    Q_PROPERTY(double epsilon READ getEpsilon NOTIFY epsilonChanged)
    Q_PROPERTY(int stepCount READ getStepCount NOTIFY stepCountChanged)
    Q_PROPERTY(QVariantList rewardHistory READ getRewardHistory NOTIFY rewardHistoryChanged)
    Q_PROPERTY(bool fastMode READ getFastMode WRITE setFastMode NOTIFY fastModeChanged)

public:
    explicit MazeController(QObject *parent = nullptr);
    
    // Методы для QML
    Q_INVOKABLE void startTraining(int episodes = 1000);
    Q_INVOKABLE void stopTraining();
    Q_INVOKABLE void resetMaze();
    Q_INVOKABLE void stepAgent();
    Q_INVOKABLE void runEpisode();
    Q_INVOKABLE void generateNewMaze();
    Q_INVOKABLE void saveModel(const QString& filename = "maze_qlearning_model.h5");
    Q_INVOKABLE bool loadModel(const QString& filename = "maze_qlearning_model.h5");
    Q_INVOKABLE void setTrainingSpeed(int msDelay);
    Q_INVOKABLE void fastTrain(int episodes = 1000); // Быстрое обучение без визуализации
    Q_INVOKABLE void setEpisodesPerUpdate(int episodes); // Количество эпизодов между обновлениями UI
    
    // Геттеры для свойств
    QString getMazeString() const;
    bool getIsTraining() const;
    int getCurrentEpisode() const;
    double getCurrentReward() const;
    double getEpsilon() const;
    int getStepCount() const;
    QVariantList getRewardHistory() const;
    bool getFastMode() const;
    void setFastMode(bool enabled);

signals:
    void mazeChanged();
    void trainingStateChanged();
    void episodeChanged();
    void rewardChanged();
    void epsilonChanged();
    void stepCountChanged();
    void rewardHistoryChanged();
    void trainingCompleted();
    void goalReached();
    void fastModeChanged();

private slots:
    void trainStep();
    void fastTrainStep();

private:
    MazeEnvironment* environment;
    QLearningAgent* agent;
    QTimer* trainingTimer;
    QTimer* fastTrainingTimer;
    
    // Состояние тренировки
    bool isTraining;
    int currentEpisode;
    int totalEpisodes;
    double episodeReward;
    int stepCount;
    bool fastMode;
    int episodesPerUpdate;
    
    // История для анализа
    QList<double> rewardHistory;
    
    // Настройки
    int trainingDelay; // Задержка между шагами в мс
    
    void finishEpisode();
    void updateMaze();
    void runSingleEpisode(); // Запуск одного эпизода без UI обновлений
};

#endif // MAZECONTROLLER_H 