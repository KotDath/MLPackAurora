#include "mazeenvironment.h"
#include <QDebug>

MazeEnvironment::MazeEnvironment() : rng(std::random_device{}()) {
    maze.resize(MAZE_SIZE, std::vector<int>(MAZE_SIZE, EMPTY));
    startRow = 0;
    startCol = 0;
    goalRow = MAZE_SIZE - 1;
    goalCol = MAZE_SIZE - 1;
    
    generateRandomMaze();
    reset();
}

void MazeEnvironment::reset() {
    // Очищаем предыдущую позицию агента
    if (agentRow >= 0 && agentRow < MAZE_SIZE && agentCol >= 0 && agentCol < MAZE_SIZE) {
        if (maze[agentRow][agentCol] == AGENT) {
            maze[agentRow][agentCol] = EMPTY;
        }
    }
    
    // Устанавливаем агента в стартовую позицию
    agentRow = startRow;
    agentCol = startCol;
    
    // Убеждаемся, что стартовая и целевая позиции правильно обозначены
    maze[startRow][startCol] = START;
    maze[goalRow][goalCol] = GOAL;
}

std::pair<int, double> MazeEnvironment::step(int action) {
    int newRow = agentRow;
    int newCol = agentCol;
    
    // Определяем новую позицию на основе действия
    switch (action) {
        case UP:
            newRow--;
            break;
        case DOWN:
            newRow++;
            break;
        case LEFT:
            newCol--;
            break;
        case RIGHT:
            newCol++;
            break;
    }
    
    double reward;
    
    // Проверяем границы
    if (!isValidPosition(newRow, newCol)) {
        reward = -5.0; // Штраф за выход за границы
        return {getCurrentState(), reward};
    }
    
    // Проверяем препятствие
    if (maze[newRow][newCol] == WALL) {
        reward = -10.0; // Штраф за столкновение со стеной
        return {getCurrentState(), reward};
    }
    
    // Очищаем старую позицию (кроме стартовой и целевой)
    if (maze[agentRow][agentCol] == AGENT) {
        maze[agentRow][agentCol] = EMPTY;
    }
    
    // Перемещаем агента
    agentRow = newRow;
    agentCol = newCol;
    
    // Получаем награду за новую позицию
    reward = getReward(newRow, newCol);
    
    // Обновляем визуализацию (кроме стартовой и целевой позиций)
    if (maze[newRow][newCol] != START && maze[newRow][newCol] != GOAL) {
        maze[newRow][newCol] = AGENT;
    }
    
    return {getCurrentState(), reward};
}

int MazeEnvironment::getCurrentState() const {
    return positionToState(agentRow, agentCol);
}

bool MazeEnvironment::isTerminal() const {
    return (agentRow == goalRow && agentCol == goalCol);
}

std::vector<std::vector<int>> MazeEnvironment::getMaze() const {
    auto result = maze;
    // Убеждаемся, что агент отображается правильно
    if (result[agentRow][agentCol] != START && result[agentRow][agentCol] != GOAL) {
        result[agentRow][agentCol] = AGENT;
    }
    return result;
}

std::pair<int, int> MazeEnvironment::getAgentPosition() const {
    return {agentRow, agentCol};
}

QString MazeEnvironment::getMazeAsString() const {
    QString result;
    auto currentMaze = getMaze();
    
    for (int i = 0; i < MAZE_SIZE; i++) {
        for (int j = 0; j < MAZE_SIZE; j++) {
            switch (currentMaze[i][j]) {
                case EMPTY:
                    result += ". ";
                    break;
                case WALL:
                    result += "# ";
                    break;
                case START:
                    result += (i == agentRow && j == agentCol) ? "S " : "s ";
                    break;
                case GOAL:
                    result += (i == agentRow && j == agentCol) ? "G " : "g ";
                    break;
                case AGENT:
                    result += "A ";
                    break;
            }
        }
        result += "\n";
    }
    return result;
}

void MazeEnvironment::generateRandomMaze(double wallProbability) {
    std::uniform_real_distribution<double> dis(0.0, 1.0);
    
    // Очищаем лабиринт
    for (int i = 0; i < MAZE_SIZE; i++) {
        for (int j = 0; j < MAZE_SIZE; j++) {
            maze[i][j] = EMPTY;
        }
    }
    
    // Генерируем стены случайным образом
    for (int i = 0; i < MAZE_SIZE; i++) {
        for (int j = 0; j < MAZE_SIZE; j++) {
            // Не размещаем стены на стартовой и целевой позициях
            if ((i == startRow && j == startCol) || (i == goalRow && j == goalCol)) {
                continue;
            }
            
            if (dis(rng) < wallProbability) {
                maze[i][j] = WALL;
            }
        }
    }
    
    // Устанавливаем стартовую и целевую позиции
    maze[startRow][startCol] = START;
    maze[goalRow][goalCol] = GOAL;
    
    // Проверяем, что есть путь от старта до цели (простая проверка)
    // В более сложной реализации можно использовать BFS для гарантии наличия пути
}

int MazeEnvironment::positionToState(int row, int col) const {
    return row * MAZE_SIZE + col;
}

std::pair<int, int> MazeEnvironment::stateToPosition(int state) const {
    return {state / MAZE_SIZE, state % MAZE_SIZE};
}

bool MazeEnvironment::isValidPosition(int row, int col) const {
    return row >= 0 && row < MAZE_SIZE && col >= 0 && col < MAZE_SIZE;
}

double MazeEnvironment::getReward(int row, int col) const {
    if (row == goalRow && col == goalCol) {
        return 100.0; // Большая награда за достижение цели
    }
    return -1.0; // Небольшой штраф за каждый шаг
} 