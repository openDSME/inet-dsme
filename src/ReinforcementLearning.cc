//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "ReinforcementLearning.h"
#include <cmath>
#include <iostream>
#include <omnetpp.h>
#include "QLearning.h"
#include "Mcts.h"

ReinforcementLearning::ReinforcementLearning() {
    // logging of important characteristics
    this->logging_transmissionpower = new cOutVector("TransmissionPower");
    this->logging_states = new cOutVector("States");
    this->logging_dataStatus = new cOutVector("DataStatus");
}

ReinforcementLearning::~ReinforcementLearning() {
}

float ReinforcementLearning::reward(int action, float prr_last) {
    // Calculate the reward for the Q-Learning algorithm
    // Use as little transmission power as possible to achieve as much reward as possible
    return ((this->actions - action) * 1.0) * prr_last;
}

float ReinforcementLearning::neg_reward(int action, float prr_last) {
    // Reward function for MCTS
    // can be negative
    return (((this->actions - action) * 1.0) * (prr_last - 0.3));
}

void ReinforcementLearning::logPower(double power = 0.0) {
    // save the transmission power that is used
    if (this->ReinforementLearningOption
            == ReinforementLearningOptions::Normal) {
        this->logging_transmissionpower->record(power);
    } else {
        this->logging_transmissionpower->record(this->getPower());
    }
}

int ReinforcementLearning::prr_to_state(float prr_last) {
    // use the prr to calculate which state should be used
    // depends on the number of states and the window size of the moving average
    int state = std::floor(prr_last * this->states);
    if (state >= this->states) {
        state = this->states - 1;
    }
    return state;
}

void ReinforcementLearning::setSuperFrameState(int superframe, int slot) {
    // set the state depending on the slot of the Multi Superframe structure
    if (this->state_transition == this->StateTransitions::SuperFrame) {
        this->state_next = superframe * this->slots + slot;
    }
}

int ReinforcementLearning::getStateTransition() {
    // returns which state transition is used
    return state_transition;
}

void ReinforcementLearning::setStateTransition(std::string transition) {
    // set the state transition, that should be used
    if (transition == "SuperFrame") {
        // depend on the Multi Superframe structure
        // needs an extra call in the DSMEPlatform
        this->state_transition = this->StateTransitions::SuperFrame;
    }
    if (transition == "Packets") {
        // advance 1 state after each transmission
        this->state_transition = this->StateTransitions::Packets;
    }
    if (transition == "Prr") {
        // switch depending on the PRR
        this->state_transition = this->StateTransitions::Prr;
    }
}

double ReinforcementLearning::getPower() {
    // return the power to the current action
    return this->transmissionPowers[this->action_next];
}

int ReinforcementLearning::getBestAction() {
    // how to select the next state
    if (this->state_transition == StateTransitions::Packets) {
        // when the state is selected depending on the state
        this->state_next = (this->state_next + 1) % this->states;
    }

    if (this->algo == Algos::Qlearning) {
        // get the next action from the Q-Learning algorithm
        this->action_next = this->QLearningClass->getAction(this->state_next);
    }

    if (this->algo == Algos::MCts) {
        // best Action in current Layer

        this->MctsState = this->MctsClass->getNodeforState(this->state_next,
                this->total_num_runs);

        Mcts *mctschildnode = this->MctsState->getMonteCarloChild(
                this->total_num_runs);

        this->action_next = mctschildnode->getAction();
        this->MctsState = mctschildnode;

    }

    this->logging_states->record(this->state_next);
    return this->action_next;
}

int ReinforcementLearning::rewardAction(float prr_last, int datastatus) {
    // how to select state
    if (this->state_transition == StateTransitions::Prr) {
        this->state_next = this->prr_to_state(prr_last);
    }

    // if Q-Learning is used
    if (this->algo == Algos::Qlearning) {
        // if prr is zero then try to reset the learning rate
        if (prr_last == 0.0) {
            this->QLearningClass->setExplorationRate(this->epsilon_set_back,
                    true);
            // this->QLearningClass->setEpsilonDynamicIncrease(); -> is worse
        }
        // get reward for next round
        float reward = this->reward(this->action_next, prr_last);

        // recalculate Q-Table
        this->QLearningClass->updateQTable(this->action_current,
                this->state_current, this->state_next, this->reward_last);
        // delayed reward, so that the next state is available
        this->reward_last = reward;
    }
    // if MCTS is used
    if (this->algo == Algos::MCts) {

        int maxTreeDepth = this->states;

        this->total_num_runs++;

        // get current best node n layers deep
        Mcts *currentNode = this->MctsClass->getNodeforState(
                this->state_current, this->total_num_runs);

        // update values
        // negative reward
        currentNode->backpropagateScore(
                this->neg_reward(this->action_current, prr_last));

        // clear not used nodes with pruning
        MctsClass->pruneTree();

    }
    // old state = new state
    this->action_last = this->action_current;
    this->action_current = this->action_next;

    this->state_last = this->state_current;
    this->state_current = this->state_next;
    // log all decisions made
    this->logging_states->record(this->state_next);
    this->logging_dataStatus->record(datastatus);
    return this->action_next;
}

void ReinforcementLearning::print() {
    // run the print option, depending on the RL algorithm used
    std::cout << " CS: " << this->state_next << " CA: " << this->action_next
            << std::endl;
    if (this->algo == Algos::Qlearning) {
        this->QLearningClass->print_q_table();
    }
    if (this->algo == Algos::MCts) {
        this->MctsClass->printTree();
    }
}

int ReinforcementLearning::getCurrentAction() {
    // return the current action
    return this->action_next;
}

int ReinforcementLearning::getCurrentState() {
    // return the current state
    return this->state_next;
}

// use QLearning
void ReinforcementLearning::initQLearning() {
    // initialize the Q-Learning algorithm
    this->ReinforementLearningOption = ReinforementLearningOptions::Learning;
    this->algo = Algos::Qlearning;
    this->QLearningClass = new QLearning(this->states, this->actions);
}
void ReinforcementLearning::initQLearning(int states) {
    // initialize the Q-Learning algorithm, and set the amount of states
    this->states = states;
    this->initQLearning();
}
void ReinforcementLearning::setupQLearning(double alpha, double gamma,
        double epsilon, double epsilon_falloff, double epsilon_set_back,
        double min_epsilon, bool is_greedy, bool is_hotbooting) {
    // set the initial values for the Q-Learning algorithm from the DSME.ned file
    this->QLearningClass->updateAllParameters(alpha, gamma, epsilon,
            epsilon_falloff, min_epsilon, is_greedy, is_hotbooting);
    this->epsilon_set_back = epsilon_set_back;
}

void ReinforcementLearning::initMcts() {
    // initialize the MCTS algorithm
    this->ReinforementLearningOption = ReinforementLearningOptions::Learning;
    this->algo = Algos::MCts;
    this->MctsClass = new Mcts(0.0, this->actions);
    this->MctsState = this->MctsClass;
    // insert the first layer in the tree
    for (int i = 0; i < this->actions; i++) {
        this->MctsClass->createChild(0.0, i);
    }
}

void ReinforcementLearning::initMcts(int states) {
    // set the amount of states that should be used
    this->states = states;
    this->initMcts();
}

int ReinforcementLearning::getReinforcementLearningOption() {
    // returns the selected algorithm
    return this->ReinforementLearningOption;
}

void ReinforcementLearning::setReinforcementLearningOption(int option) {
    // set the algorithm that should be used, is also done in the init methods for the algorithms
    this->ReinforementLearningOption = option;
}

