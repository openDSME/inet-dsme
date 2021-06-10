#include <iostream>
#include <cstdlib>
#include <vector>
#include <omnetpp.h>
#include "QLearning.h"

QLearning::QLearning(int states, int actions) {
    // setup values
    this->states = states;
    this->actions = actions;
    this->setQTable(0.0);
    if (is_greedy) {
        this->epsilon = 1.0;
    }
    // init random number generator
    this->randomNumGen = omnetpp::cModule().getRNG(1);
    // logging data in omnetpp
    this->logging_epsilon = new omnetpp::cOutVector("epsilon:Q-Learning");
    this->logging_rewards = new omnetpp::cOutVector("reward:Q-Learning");
}

QLearning::~QLearning() {
    q_table.clear();
}

void QLearning::setQTable(float value) {
    // initialize the Q-Table with all values set the same
    q_table.clear();
    std::vector<float> action_vec;
    for (int i = 0; i < this->actions; i++) {
        action_vec.push_back(value);
    }
    for (int j = 0; j < this->states; j++) {
        q_table.push_back(action_vec);
    }
}

void QLearning::setQTableHotbooting(){
    if(this->is_hotbooting == true){
        // fill last column with 1.0
        for(int i = 0; i < this->q_table.size(); i++){
            int row_length = this->q_table[i].size();
            this->q_table[i][row_length - 1] = 1.0;
        }
    }
}

int QLearning::random(int min, int max){
    // use the omnetpp deterministic random number generator
    // range: [min, max]
    int number = omnetpp::intuniform(this->randomNumGen, min, max);
    return number;
}

int QLearning::getRandomAction() {
    return this->random(0, this->actions);
}

int QLearning::getAction(int state) {
    // get the next action
    if (this->is_greedy) {
        // use epsilon greedy strategy
        if (this->epsilon < this->min_epsilon) {
            this->is_greedy = false;
        } else {
            this->epsilon *= this->epsilon_falloff;
        }
    }
    this->logging_epsilon->record(this->epsilon);
    if (this->random(0, 100) / 100.0 < this->epsilon) {
        // exploration
        return this->getRandomAction();
    }
    // exploitation
    return this->getMaxQValue(state);
}

int QLearning::getMaxQValue(int state) {
    // returns the maximum Q-Value of a given state
    int currentBestIndex = 0;
    int currentBestQValue = -9999;
    for (int i = 0; i < this->actions; i++) {
        if (currentBestQValue < q_table[state][i]) {
            currentBestIndex = i;
            currentBestQValue = q_table[state][i];
        }
    }
    return currentBestIndex;
}

void QLearning::setExplorationRate(float value, bool greedy) {
    // set the exploration rate
    this->epsilon = value;
    this->is_greedy = greedy;
}
void QLearning::setEpsilonDynamicIncrease(){
    // Can be used instead of the setExplorationRate for a more dynamic handling of the epsilon value
    this->epsilon *= this->epsilon_dynamic_increase;
    if(this->epsilon >= 1.0){
        this->epsilon = 1.0;
    }
    this->is_greedy = true;
}

void QLearning::updateQTable(int action, int state, int next_state,
    float reward) {
        // update the Q-Table with new values
        this->logging_rewards->record(reward);
        float q_value = q_table[state][action];
        float max_value = q_table[next_state][this->getMaxQValue(next_state)];
        float new_q_value = (1 - alpha) * q_value
                + alpha * (reward + gamma * (max_value));

        q_table[state][action] = new_q_value;
        state = next_state;
    }

void QLearning::print_q_table() {
    // print out the Q-Table
    std::cout << "--- Q-Table ---" << std::endl;
    int statecounter = 0;
    for (std::vector<std::vector<float>>::iterator row = q_table.begin();
            row != q_table.end(); ++row) {
        std::cout << "> ";
        for (std::vector<float>::iterator cell = (*row).begin();
                cell != (*row).end(); ++cell) {
            std::cout << std::to_string(*cell) << " ";
        }
        std::cout << "< " << std::to_string(this->getMaxQValue(statecounter++))
                << std::endl;
    }
    std::cout << "---   END   ---" << std::endl;
}

void QLearning::updateAllParameters(double alpha, double gamma, double epsilon, double epsilon_falloff, double min_epsilon, bool is_greedy, bool is_hotbooting){
    // set all parameter according to the parameter study
    this->alpha = alpha;
    this->gamma = gamma;
    this->min_epsilon = min_epsilon;
    this->epsilon = epsilon;
    this->epsilon_falloff = epsilon_falloff;
    this->is_greedy = is_greedy;
    this->is_hotbooting = is_hotbooting;
    this->setQTableHotbooting();
}


