#ifndef MCTS_H
#define MCTS_H

#include <iostream>
#include <vector>
#include <cstdlib>
#include <cmath>
#include <omnetpp.h>

class Mcts
{
private:
    std::vector<Mcts> childs;
    Mcts *parent;
    float uct_score = 0.0;
    float value = 0.0;
    int action = 0;
    int times_node_executed = 0;
    float exporation_param = 2.0;
    float oldValuestrength = 0.7;
    float uct(int total_num_runs);
    omnetpp::cRNG* randomNumGen;
    enum Init_Mode {RL, Normal};
    int initMode = Init_Mode::RL;
    float percent_take_lower_option = 0.1;
public:
    Mcts(float value, int action, Mcts* parent = nullptr);
    ~Mcts();
    void createChild(float value, int action);
    Mcts* currentBestNode(int total_num_runs);
    Mcts* getChild(int index);
    int getNumChilds();
    Mcts* getMonteCarloChild();
    Mcts* getBestChild();
    void printTree(int level=0);
    void backpropagateScore(float value);
    void backpropagateScore_RL(float value);
    int getBestAction();
    std::vector<int> getActionsPerformed();
    int getAction();
    Mcts* getPointer();
    int getLayer();
    Mcts* getRootNode();
    Mcts* getNodeforState(int layer);
};



#endif /* MCTS_H */
