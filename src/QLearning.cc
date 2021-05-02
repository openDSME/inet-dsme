#include <iostream>
#include <cstdlib>
#include <vector>
#include <random>
#include "QLearning.h"

QLearning::QLearning(int states, int actions) {
    this->states = states;
    this->actions = actions;
//this->q_table = new std::vector<std::vector<float>>
    this->setQTable(0.0);
    if (is_greedy) {
        this->epsilon = 1.0;
    }
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

int QLearning::random(int min, int max) //range : [min, max]
        {
    static bool first = true;
    if (first) {
        srand(time(NULL)); //seeding for the first time only!
        first = false;
    }
    return min + rand() % ((max + 1) - min);
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

void QLearning::updateQTable(int action, int state, int next_state,
        float reward) {
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
