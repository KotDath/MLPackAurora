#ifndef QLEARNINGAGENT_H
#define QLEARNINGAGENT_H

#include <mlpack/methods/reinforcement_learning/q_learning.hpp>
#include <mlpack/methods/reinforcement_learning/policy/greedy_policy.hpp>
#include <mlpack/methods/reinforcement_learning/replay/random_replay.hpp>
#include <mlpack/methods/reinforcement_learning/training_config.hpp>
#include <mlpack/core/data/save.hpp>
#include <mlpack/core/data/load.hpp>
#include <ensmallen.hpp>
#include <armadillo>
#include <random>
#include <QtCore>

class QLearningAgent {
public:
    QLearningAgent(int numStates, int numActions, 
                   double learningRate = 0.1, 
                   double discountFactor = 0.95, 
                   double epsilon = 1.0,
                   double epsilonDecay = 0.995,
                   double minEpsilon = 0.01);
    
    // Основные методы Q-learning
    int selectAction(int state);
    void updateQValue(int state, int action, double reward, int nextState);
    void reset();
    
    // Получение информации
    arma::mat getQTable() const;
    double getEpsilon() const;
    double getQValue(int state, int action) const;
    int getBestAction(int state) const;
    
    // Обучение
    void decayEpsilon();
    void setEpsilon(double newEpsilon);
    
    // Сохранение и загрузка модели
    void saveModel(const QString& filename) const;
    bool loadModel(const QString& filename);

private:
    int numStates;
    int numActions;
    double learningRate;
    double discountFactor;
    double epsilon;
    double epsilonDecay;
    double minEpsilon;
    
    arma::mat qTable;
    std::mt19937 rng;
    std::uniform_real_distribution<double> uniformDist;
};

#endif // QLEARNINGAGENT_H 