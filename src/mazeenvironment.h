#ifndef MAZEENVIRONMENT_H
#define MAZEENVIRONMENT_H

#include <mlpack.hpp>
#include <armadillo>
#include <vector>
#include <random>
#include <QtCore>

class MazeEnvironment {
public:
    static constexpr int MAZE_SIZE = 10;
    static constexpr int NUM_ACTIONS = 4; // Вверх, Вниз, Влево, Вправо
    static constexpr int NUM_STATES = MAZE_SIZE * MAZE_SIZE;
    
    enum Action {
        UP = 0,
        DOWN = 1,
        LEFT = 2,
        RIGHT = 3
    };
    
    enum CellType {
        EMPTY = 0,
        WALL = 1,
        START = 2,
        GOAL = 3,
        AGENT = 4
    };

    MazeEnvironment();
    
    // Основные методы среды
    void reset();
    std::pair<int, double> step(int action);
    int getCurrentState() const;
    bool isTerminal() const;
    std::vector<std::vector<int>> getMaze() const;
    std::pair<int, int> getAgentPosition() const;
    
    // Для визуализации
    QString getMazeAsString() const;
    
    // Генерация лабиринта
    void generateRandomMaze(double wallProbability = 0.2);

private:
    std::vector<std::vector<int>> maze;
    int agentRow, agentCol;
    int goalRow, goalCol;
    int startRow, startCol;
    std::mt19937 rng;
    
    // Вспомогательные методы
    int positionToState(int row, int col) const;
    std::pair<int, int> stateToPosition(int state) const;
    bool isValidPosition(int row, int col) const;
    double getReward(int row, int col) const;
};

#endif // MAZEENVIRONMENT_H 