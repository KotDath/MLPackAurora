#include "qlearningagent.h"
#include <QDebug>
#include <QDir>
#include <QStandardPaths>

QLearningAgent::QLearningAgent(int numStates, int numActions, 
                               double learningRate, double discountFactor, 
                               double epsilon, double epsilonDecay, double minEpsilon)
    : numStates(numStates), numActions(numActions),
      learningRate(learningRate), discountFactor(discountFactor),
      epsilon(epsilon), epsilonDecay(epsilonDecay), minEpsilon(minEpsilon),
      rng(std::random_device{}()), uniformDist(0.0, 1.0) {
    
    // Инициализируем Q-таблицу нулями
    qTable = arma::zeros<arma::mat>(numStates, numActions);
    
    qDebug() << "QLearningAgent initialized with" << numStates << "states and" << numActions << "actions";
    qDebug() << "Learning rate:" << learningRate << "Discount factor:" << discountFactor;
    qDebug() << "Initial epsilon:" << epsilon;
}

int QLearningAgent::selectAction(int state) {
    // Epsilon-greedy стратегия выбора действий
    if (uniformDist(rng) < epsilon) {
        // Исследование: случайное действие
        std::uniform_int_distribution<int> actionDist(0, numActions - 1);
        return actionDist(rng);
    } else {
        // Эксплуатация: лучшее известное действие
        return getBestAction(state);
    }
}

void QLearningAgent::updateQValue(int state, int action, double reward, int nextState) {
    if (state < 0 || state >= numStates || action < 0 || action >= numActions) {
        qDebug() << "Invalid state or action in updateQValue:" << state << action;
        return;
    }
    
    // Q-learning обновление: Q(s,a) = Q(s,a) + α[r + γ*max(Q(s',a')) - Q(s,a)]
    double currentQ = qTable(state, action);
    double maxNextQ = arma::max(qTable.row(nextState));
    double newQ = currentQ + learningRate * (reward + discountFactor * maxNextQ - currentQ);
    
    qTable(state, action) = newQ;
}

void QLearningAgent::reset() {
    // Сброс не нужен для Q-learning агента, Q-таблица сохраняется между эпизодами
}

arma::mat QLearningAgent::getQTable() const {
    return qTable;
}

double QLearningAgent::getEpsilon() const {
    return epsilon;
}

double QLearningAgent::getQValue(int state, int action) const {
    if (state < 0 || state >= numStates || action < 0 || action >= numActions) {
        return 0.0;
    }
    return qTable(state, action);
}

int QLearningAgent::getBestAction(int state) const {
    if (state < 0 || state >= numStates) {
        return 0;
    }
    
    arma::uword bestAction;
    qTable.row(state).max(bestAction);
    return static_cast<int>(bestAction);
}

void QLearningAgent::decayEpsilon() {
    epsilon = std::max(minEpsilon, epsilon * epsilonDecay);
}

void QLearningAgent::setEpsilon(double newEpsilon) {
    epsilon = std::max(0.0, std::min(1.0, newEpsilon));
}

void QLearningAgent::saveModel(const QString& filename) const {
    try {
        QString documentsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
        QString fullPath = documentsPath + "/" + filename;
        
        // Создаем директорию если она не существует
        QDir().mkpath(QFileInfo(fullPath).absolutePath());
        
        // Сохраняем Q-таблицу в формате HDF5
        bool success = qTable.save(fullPath.toStdString(), arma::hdf5_binary);
        if (success) {
            qDebug() << "Q-table saved successfully to:" << fullPath;
        } else {
            qDebug() << "Failed to save Q-table to:" << fullPath;
        }
    } catch (const std::exception& e) {
        qDebug() << "Exception while saving model:" << e.what();
    }
}

bool QLearningAgent::loadModel(const QString& filename) {
    try {
        QString documentsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
        QString fullPath = documentsPath + "/" + filename;
        
        arma::mat loadedTable;
        bool success = loadedTable.load(fullPath.toStdString(), arma::hdf5_binary);
        
        if (success && loadedTable.n_rows == numStates && loadedTable.n_cols == numActions) {
            qTable = loadedTable;
            qDebug() << "Q-table loaded successfully from:" << fullPath;
            return true;
        } else {
            qDebug() << "Failed to load Q-table from:" << fullPath;
            return false;
        }
    } catch (const std::exception& e) {
        qDebug() << "Exception while loading model:" << e.what();
        return false;
    }
} 
 