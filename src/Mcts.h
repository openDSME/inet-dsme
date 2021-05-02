#ifndef MCTS_H
#define MCTS_H

#include <iostream>
#include <vector>
#include <cstdlib>
#include <cmath>

class Mcts
{
private:
    std::vector<Mcts> childs;
    Mcts *parent;
    float ucb1_score = 0.0;
    float value = 0;
    int action = -1;
    int times_node_executed = 0;
    float ucb1(int total_num_runs);
public:
    Mcts(float value, int action, Mcts* parent = nullptr);
    ~Mcts();
    void createChild(float value, int action);
    Mcts* currentBestNode(int total_num_runs);
    Mcts* getChild(int index);
    void printTree(int level=0);
    void backpropagateScore(float value);
    int getBestAction();
    std::vector<int> getActionsPerformed();
    int getAction();
    Mcts* getPointer();
};



#endif
