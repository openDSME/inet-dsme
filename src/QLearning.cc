#include <iostream>
#include <cstdlib>
#include <vector>
#include <omnetpp.h>
#include "QLearning.h"

QLearning::QLearning(int states, int actions) {
    this->states = states;
    this->actions = actions;
    this->setQTable(0.0);
    if (is_greedy) {
        this->epsilon = 1.0;
    }
    // init random number generator
    this->randomNumGen = omnetpp::cModule().getRNG(1);
    this->logging_epsilon = new omnetpp::cOutVector("epsilon:Q-Learning");
    this->logging_rewards = new omnetpp::cOutVector("reward:Q-Learning");
}

QLearning::~QLearning() {
    q_table.clear();
}

void QLearning::setQTable(float value) {
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
//range : [min, max]
// static bool first = true;
// if (first) {
//     srand(time(NULL)); //seeding for the first time only!
//     first = false;
// }
// return min + rand() % ((max + 1) - min);
int number = omnetpp::intuniform(this->randomNumGen, min, max);
return number;
}

int QLearning::getRandomAction() {
return this->random(0, this->actions);
}

int QLearning::getAction(int state) {
if (this->is_greedy) {
    if (this->epsilon < this->min_epsilon) {
        this->is_greedy = false;
    } else {
        this->epsilon *= this->epsilon_percentage;
    }
}
this->logging_epsilon->record(this->epsilon);
if (this->random(0, 100) / 100.0 < this->epsilon) {
    // std::cout << "random";
    return this->getRandomAction();
}
return this->getMaxQValue(state);
}

int QLearning::getMaxQValue(int state) {
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

void QLearning::setLearningRate(float value, bool greedy) {
this->epsilon = value;
this->is_greedy = greedy;
}

void QLearning::updateQTable(int action, int state, int next_state,
    float reward) {
    this->logging_rewards->record(reward);
float q_value = q_table[state][action];
float max_value = q_table[next_state][this->getMaxQValue(next_state)];
float new_q_value = (1 - alpha) * q_value
        + alpha * (reward + gamma * (max_value));

q_table[state][action] = new_q_value;
state = next_state;
}

void QLearning::print_q_table() {
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

void QLearning::updateAllParameters(double alpha, double gamma, double epsilon, double epsilon_percentage, bool is_greedy, bool is_hotbooting){
    this->alpha = alpha;
    this->gamma = gamma;
    this->epsilon = epsilon;
    this->epsilon_percentage = epsilon_percentage;
    this->is_greedy = is_greedy;
    this->is_hotbooting = this->is_hotbooting;
    this->setQTableHotbooting();
}


