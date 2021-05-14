#ifndef QLearning_H
#define QLearning_H

#include <iostream>
#include <cstdlib>
#include <vector>
#include <omnetpp.h>


class QLearning{

public:
    QLearning(int states, int actions);
    ~QLearning();
    void print_q_table();
    void updateQTable(int action, int state, int next_state, float reward);
    void test(){
        std::cout << "test";
    }
    int getAction(int state);
    void setLearningRate(float value, bool greedy = false);
    void updateAllParameters(double alpha, double gamma, double epsilon, double epsilon_percentage, bool is_greedy, bool is_hotbooting);

private:
    int random(int min, int max);
    int getRandomAction();
    int getMaxQValue(int state);
    void setQTable(float value);
    void setQTableHotbooting();
    int states;
    int actions;
    bool is_greedy = true; // use normal or a greedy random
    bool is_hotbooting = true;
    float min_epsilon = 0.01;
    float epsilon_percentage = 0.9999;
    float epsilon = 0.3; // Exploration Param
    float alpha = 0.1; // Weight old q-value
    float gamma = 0.6; // Weight new q-value
    std::vector<std::vector<float>> q_table;
    omnetpp::cRNG* randomNumGen;
    omnetpp::cOutVector *logging_rewards;
    omnetpp::cOutVector *logging_epsilon;

};


#endif
