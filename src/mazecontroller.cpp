#include "mazecontroller.h"
#include <QDebug>
#include <QCoreApplication>
#include <chrono>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QStandardPaths>

MazeController::MazeController(QObject *parent)
    : QObject(parent), isTraining(false), currentEpisode(0), totalEpisodes(0),
      episodeReward(0.0), stepCount(0), trainingDelay(100), fastMode(false), episodesPerUpdate(10), movingAverageReward(0.0) {
    
    // Создаем среду и агента
    environment = new MazeEnvironment();
    agent = new QLearningAgent(MazeEnvironment::NUM_STATES, MazeEnvironment::NUM_ACTIONS);
    
    // Настраиваем таймер для обычной тренировки
    trainingTimer = new QTimer(this);
    trainingTimer->setSingleShot(false);
    connect(trainingTimer, &QTimer::timeout, this, &MazeController::trainStep);
    
    // Настраиваем таймер для быстрой тренировки
    fastTrainingTimer = new QTimer(this);
    fastTrainingTimer->setSingleShot(false);
    connect(fastTrainingTimer, &QTimer::timeout, this, &MazeController::fastTrainStep);
    
    // Создаем директорию Documents если её нет
    QString documentsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    QDir documentsDir(documentsPath);
    if (!documentsDir.exists()) {
        documentsDir.mkpath(".");
    }
    
    // Инициализируем файл логирования
    QFile logFile(documentsPath + "/maze.txt");
    if (logFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&logFile);
        out << "Episode,Reward,Steps,Epsilon,MovingAverageReward,Timestamp\n";
        logFile.close();
    }
    
    qDebug() << "MazeController initialized";
}

QString MazeController::getMazeString() const {
    return environment->getMazeAsString();
}

bool MazeController::getIsTraining() const {
    return isTraining;
}

int MazeController::getCurrentEpisode() const {
    return currentEpisode;
}

double MazeController::getCurrentReward() const {
    return episodeReward;
}

double MazeController::getEpsilon() const {
    return agent->getEpsilon();
}

int MazeController::getStepCount() const {
    return stepCount;
}

QVariantList MazeController::getRewardHistory() const {
    QVariantList result;
    for (double reward : rewardHistory) {
        result.append(reward);
    }
    return result;
}

bool MazeController::getFastMode() const {
    return fastMode;
}

double MazeController::getMovingAverageReward() const {
    return movingAverageReward;
}

void MazeController::setFastMode(bool enabled) {
    if (fastMode != enabled) {
        fastMode = enabled;
        emit fastModeChanged();
        qDebug() << "Fast mode:" << (enabled ? "enabled" : "disabled");
    }
}

void MazeController::startTraining(int episodes) {
    if (isTraining) {
        qDebug() << "Training already in progress";
        return;
    }
    
    totalEpisodes = episodes;
    currentEpisode = 0;
    episodeReward = 0.0;
    stepCount = 0;
    isTraining = true;
    
    rewardHistory.clear();
    
    // Очищаем данные для скользящего среднего
    recentRewards.clear();
    movingAverageReward = 0.0;
    
    // Очищаем файл логирования и записываем заголовок
    QFile logFile(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/maze.txt");
    if (logFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&logFile);
        out << "Episode,Reward,Steps,Epsilon,MovingAverageReward,Timestamp\n";
        logFile.close();
    }
    
    // Сбрасываем среду для нового эпизода
    environment->reset();
    
    if (fastMode) {
        // В быстром режиме запускаем с минимальной задержкой
        fastTrainingTimer->start(1);
    } else {
        // Запускаем таймер обычной тренировки
        trainingTimer->start(trainingDelay);
    }
    
    emit trainingStateChanged();
    emit episodeChanged();
    emit rewardChanged();
    emit stepCountChanged();
    emit rewardHistoryChanged();
    
    qDebug() << "Training started for" << episodes << "episodes" << (fastMode ? "(fast mode)" : "");
}

void MazeController::stopTraining() {
    if (!isTraining) {
        return;
    }
    
    trainingTimer->stop();
    fastTrainingTimer->stop();
    isTraining = false;
    
    emit trainingStateChanged();
    qDebug() << "Training stopped at episode" << currentEpisode;
}

void MazeController::resetMaze() {
    environment->reset();
    episodeReward = 0.0;
    stepCount = 0;
    
    updateMaze();
    emit rewardChanged();
    emit stepCountChanged();
}

void MazeController::stepAgent() {
    if (environment->isTerminal()) {
        return;
    }
    
    int currentState = environment->getCurrentState();
    int action = agent->selectAction(currentState);
    
    auto [nextState, reward] = environment->step(action);
    
    // Обновляем Q-значение
    agent->updateQValue(currentState, action, reward, nextState);
    
    episodeReward += reward;
    stepCount++;
    
    updateMaze();
    emit rewardChanged();
    emit stepCountChanged();
    
    if (environment->isTerminal()) {
        emit goalReached();
        qDebug() << "Goal reached! Total reward:" << episodeReward << "Steps:" << stepCount;
    }
}

void MazeController::runEpisode() {
    resetMaze();
    
    const int maxSteps = 200; // Предотвращаем бесконечные циклы
    int steps = 0;
    
    while (!environment->isTerminal() && steps < maxSteps) {
        stepAgent();
        steps++;
    }
    
    if (steps >= maxSteps) {
        qDebug() << "Episode terminated: maximum steps reached";
    }
}

void MazeController::generateNewMaze() {
    environment->generateRandomMaze();
    resetMaze();
    qDebug() << "New maze generated";
}

void MazeController::saveModel(const QString& filename) {
    agent->saveModel(filename);
}

bool MazeController::loadModel(const QString& filename) {
    return agent->loadModel(filename);
}

void MazeController::setTrainingSpeed(int msDelay) {
    trainingDelay = std::max(1, msDelay);
    if (trainingTimer->isActive()) {
        trainingTimer->setInterval(trainingDelay);
    }
}

void MazeController::fastTrain(int episodes) {
    if (isTraining) {
        qDebug() << "Training already in progress";
        return;
    }
    
    qDebug() << "Starting fast training for" << episodes << "episodes...";
    
    // Очищаем файл логирования и записываем заголовок
    QFile logFile(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/maze.txt");
    if (logFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&logFile);
        out << "Episode,Reward,Steps,Epsilon,MovingAverageReward,Timestamp\n";
        logFile.close();
    }
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    for (int episode = 0; episode < episodes; episode++) {
        runSingleEpisode();
        
        // Обновляем UI каждые episodesPerUpdate эпизодов
        if (episode % episodesPerUpdate == 0) {
            currentEpisode = episode;
            emit episodeChanged();
            emit rewardHistoryChanged();
            emit epsilonChanged();
            
            // Позволяем UI обновиться
            QCoreApplication::processEvents();
        }
    }
    
    // Финальное обновление
    currentEpisode = episodes;
    emit episodeChanged();
    emit rewardHistoryChanged();
    emit epsilonChanged();
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    qDebug() << "Fast training completed in" << duration.count() << "ms";
    qDebug() << "Average time per episode:" << (double)duration.count() / episodes << "ms";
    
    emit trainingCompleted();
}

void MazeController::setEpisodesPerUpdate(int episodes) {
    episodesPerUpdate = std::max(1, episodes);
    qDebug() << "Episodes per UI update set to:" << episodesPerUpdate;
}

void MazeController::trainStep() {
    if (!isTraining) {
        return;
    }
    
    // Если эпизод завершен, начинаем новый
    if (environment->isTerminal() || stepCount >= 200) {
        finishEpisode();
        
        currentEpisode++;
        emit episodeChanged();
        
        // Проверяем, завершено ли обучение
        if (currentEpisode >= totalEpisodes) {
            stopTraining();
            emit trainingCompleted();
            qDebug() << "Training completed!";
            return;
        }
        
        // Начинаем новый эпизод
        environment->reset();
        episodeReward = 0.0;
        stepCount = 0;
        
        updateMaze();
        emit rewardChanged();
        emit stepCountChanged();
        
        return;
    }
    
    // Выполняем шаг агента
    stepAgent();
}

void MazeController::fastTrainStep() {
    if (!isTraining) {
        return;
    }
    
    // В быстром режиме выполняем несколько эпизодов за раз
    for (int i = 0; i < episodesPerUpdate && currentEpisode < totalEpisodes; i++) {
        runSingleEpisode();
        currentEpisode++;
    }
    
    // Обновляем UI
    emit episodeChanged();
    emit rewardHistoryChanged();
    emit epsilonChanged();
    
    if (currentEpisode >= totalEpisodes) {
        stopTraining();
        emit trainingCompleted();
        qDebug() << "Fast training completed!";
    }
}

void MazeController::runSingleEpisode() {
    environment->reset();
    double currentEpisodeReward = 0.0;
    int currentStepCount = 0;
    const int maxSteps = 200;
    
    while (!environment->isTerminal() && currentStepCount < maxSteps) {
        int currentState = environment->getCurrentState();
        int action = agent->selectAction(currentState);
        
        auto [nextState, reward] = environment->step(action);
        
        // Обновляем Q-значение
        agent->updateQValue(currentState, action, reward, nextState);
        
        currentEpisodeReward += reward;
        currentStepCount++;
    }
    
    // Добавляем награду эпизода в историю
    rewardHistory.append(currentEpisodeReward);
    
    // Обновляем скользящее среднее последних 100 эпизодов
    recentRewards.append(currentEpisodeReward);
    if (recentRewards.size() > movingAverageWindow) {
        recentRewards.removeFirst();
    }
    
    // Вычисляем скользящее среднее
    double sum = 0.0;
    for (double reward : recentRewards) {
        sum += reward;
    }
    double newMovingAverage = sum / recentRewards.size();
    
    if (qAbs(movingAverageReward - newMovingAverage) > 0.001) {
        movingAverageReward = newMovingAverage;
        emit movingAverageRewardChanged();
    }
    
    // Логируем в файл
    QFile logFile(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/maze.txt");
    if (logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&logFile);
        out << currentEpisode << "," 
            << QString::number(currentEpisodeReward, 'f', 6) << "," 
            << currentStepCount << "," 
            << QString::number(agent->getEpsilon(), 'f', 6) << ","
            << QString::number(movingAverageReward, 'f', 6) << ","
            << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << "\n";
        logFile.close();
    }

    // Уменьшаем epsilon
    agent->decayEpsilon();
    
    // Обновляем текущие значения для отображения
    episodeReward = currentEpisodeReward;
    stepCount = currentStepCount;
}

void MazeController::finishEpisode() {
    // Добавляем награду эпизода в историю
    rewardHistory.append(episodeReward);
    
    // Обновляем скользящее среднее последних 100 эпизодов
    recentRewards.append(episodeReward);
    if (recentRewards.size() > movingAverageWindow) {
        recentRewards.removeFirst();
    }
    
    // Вычисляем скользящее среднее
    double sum = 0.0;
    for (double reward : recentRewards) {
        sum += reward;
    }
    double newMovingAverage = sum / recentRewards.size();
    
    if (qAbs(movingAverageReward - newMovingAverage) > 0.001) {
        movingAverageReward = newMovingAverage;
        emit movingAverageRewardChanged();
    }
    
    // Логируем в файл
    QFile logFile(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/maze.txt");
    if (logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&logFile);
        out << currentEpisode << "," 
            << QString::number(episodeReward, 'f', 6) << "," 
            << stepCount << "," 
            << QString::number(agent->getEpsilon(), 'f', 6) << ","
            << QString::number(movingAverageReward, 'f', 6) << ","
            << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << "\n";
        logFile.close();
    }

    // Уменьшаем epsilon (исследование -> эксплуатация)
    agent->decayEpsilon();
    
    emit rewardHistoryChanged();
    emit epsilonChanged();
    
    // Выводим статистику каждые 100 эпизодов
    if (currentEpisode % 100 == 0) {
        qDebug() << "Episode" << currentEpisode << "- Moving average reward:" << movingAverageReward
                 << "Episode reward:" << episodeReward << "Epsilon:" << agent->getEpsilon() << "Steps:" << stepCount;
    }
}

void MazeController::updateMaze() {
    emit mazeChanged();
} 